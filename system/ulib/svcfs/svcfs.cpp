// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <svcfs/svcfs.h>

#include <fcntl.h>
#include <string.h>

#include <fs/dispatcher.h>
#include <mxalloc/new.h>

namespace svcfs {

static void CopyToArray(const char* string, size_t len, mxtl::Array<char>* result) {
    mxtl::Array<char> array(new char[len + 1], len);
    memcpy(array.get(), string, len);
    array[len] = '\0';
    result->swap(array);
}

static bool IsDot(const char* name, size_t len) {
    return (len == 1) && (name[0] == '.');
}

static bool IsDotDot(const char* name, size_t len) {
    return (len == 2) && (name[0] == '.') && (name[1] == '.');
}

static bool IsValidServiceName(const char* name, size_t len) {
    return name && len >= 1 && !IsDot(name, len) && !IsDotDot(name, len) &&
        !memchr(name, '/', len) && !memchr(name, 0, len);
}

struct dircookie_t {
    uint64_t last_id;
};

static_assert(sizeof(dircookie_t) <= sizeof(vdircookie_t),
              "svcfs dircookie too large to fit in IO state");

// ServiceProvider -------------------------------------------------------------

ServiceProvider::~ServiceProvider() = default;

// VnodeSvc --------------------------------------------------------------------

VnodeSvc::VnodeSvc(uint64_t node_id,
                   mxtl::Array<char> name,
                   ServiceProvider* provider)
    : node_id_(node_id), name_(mxtl::move(name)), provider_(provider) {
}

VnodeSvc::~VnodeSvc() = default;

mx_status_t VnodeSvc::Open(uint32_t flags) {
    if (flags & O_DIRECTORY) {
        return MX_ERR_NOT_DIR;
    }
    return MX_OK;
}

mx_status_t VnodeSvc::Serve(fs::Vfs* vfs, mx::channel channel, uint32_t flags) {
    if (!provider_) {
        return MX_ERR_UNAVAILABLE;
    }

    provider_->Connect(name_.get(), name_.size(), mxtl::move(channel));

    // If node_id_ is zero, this vnode was created during |Lookup| and doesn't
    // have a parent. Without a parent, there isn't anyone to clean up the raw
    // |provider_| pointer, which means we need to clean it up here.
    if (!node_id_)
        provider_ = nullptr;

    return MX_OK;
}

bool VnodeSvc::NameMatch(const char* name, size_t len) const {
    return (name_.size() == len) && (memcmp(name_.get(), name, len) == 0);
}

void VnodeSvc::ClearProvider() {
    provider_ = nullptr;
}

// VnodeDir --------------------------------------------------------------------

VnodeDir::VnodeDir()
    : next_node_id_(2) {}

VnodeDir::~VnodeDir() = default;

mx_status_t VnodeDir::Open(uint32_t flags) {
    return MX_OK;
}

mx_status_t VnodeDir::Lookup(mxtl::RefPtr<fs::Vnode>* out, const char* name, size_t len) {
    mxtl::RefPtr<VnodeSvc> vn = nullptr;
    for (auto& child : services_) {
        if (child.NameMatch(name, len)) {
            *out = mxtl::RefPtr<VnodeSvc>(&child);
            return MX_OK;
        }
    }

    return MX_ERR_NOT_FOUND;
}

mx_status_t VnodeDir::Getattr(vnattr_t* attr) {
    memset(attr, 0, sizeof(vnattr_t));
    attr->mode = V_TYPE_DIR | V_IRUSR;
    attr->nlink = 1;
    return MX_OK;
}

void VnodeDir::Notify(const char* name, size_t len, unsigned event) { watcher_.Notify(name, len, event); }
mx_status_t VnodeDir::WatchDir(mx::channel* out) { return watcher_.WatchDir(out); }
mx_status_t VnodeDir::WatchDirV2(fs::Vfs* vfs, const vfs_watch_dir_t* cmd) {
    return watcher_.WatchDirV2(vfs, this, cmd);
}

mx_status_t VnodeDir::Readdir(void* cookie, void* data, size_t len) {
    dircookie_t* c = static_cast<dircookie_t*>(cookie);
    fs::DirentFiller df(data, len);

    mx_status_t r = 0;
    if (c->last_id < 1) {
        if ((r = df.Next(".", 1, VTYPE_TO_DTYPE(V_TYPE_DIR))) != MX_OK) {
            return df.BytesFilled();
        }
        c->last_id = 1;
    }

    for (const VnodeSvc& vn : services_) {
        if (c->last_id >= vn.node_id()) {
            continue;
        }
        if ((r = df.Next(vn.name().get(), vn.name().size(),
                         VTYPE_TO_DTYPE(V_TYPE_FILE))) != MX_OK) {
            return df.BytesFilled();
        }
        c->last_id = vn.node_id();
    }

    return df.BytesFilled();
}

bool VnodeDir::AddService(const char* name, size_t len, ServiceProvider* provider) {
    if (!IsValidServiceName(name, len)) {
        return false;
    }

    mxtl::Array<char> array;
    CopyToArray(name, len, &array);

    mxtl::RefPtr<VnodeSvc> vn = mxtl::AdoptRef(new VnodeSvc(
        next_node_id_++, mxtl::move(array), provider));

    services_.push_back(mxtl::move(vn));
    Notify(name, len, VFS_WATCH_EVT_ADDED);
    return true;
}

bool VnodeDir::RemoveService(const char* name, size_t len) {
    for (auto& child : services_) {
        if (child.NameMatch(name, len)) {
            child.ClearProvider();
            services_.erase(child);
            return true;
        }
    }
    return false;
}

void VnodeDir::RemoveAllServices() {
    for (VnodeSvc& vn : services_) {
        vn.ClearProvider();
    }
    services_.clear();
}

// VnodeProviderDir --------------------------------------------------------------------

VnodeProviderDir::VnodeProviderDir()
    : provider_(nullptr) {}

VnodeProviderDir::~VnodeProviderDir() = default;

mx_status_t VnodeProviderDir::Open(uint32_t flags) {
    return MX_OK;
}

mx_status_t VnodeProviderDir::Lookup(mxtl::RefPtr<fs::Vnode>* out, const char* name, size_t len) {
    if (!IsValidServiceName(name, len)) {
        return MX_ERR_NOT_FOUND;
    }

    mxtl::Array<char> array;
    CopyToArray(name, len, &array);

    *out = mxtl::AdoptRef(new VnodeSvc(0, mxtl::move(array), provider_));
    return MX_OK;
}

mx_status_t VnodeProviderDir::Getattr(vnattr_t* attr) {
    memset(attr, 0, sizeof(vnattr_t));
    attr->mode = V_TYPE_DIR | V_IRUSR;
    attr->nlink = 1;
    return MX_OK;
}

void VnodeProviderDir::SetServiceProvider(ServiceProvider* provider) {
    provider_ = provider;
}

} // namespace svcfs
