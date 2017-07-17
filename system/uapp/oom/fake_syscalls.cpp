// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Fake syscalls for testing.

#include "fake_syscalls.h"

#include <magenta/status.h>
#include <magenta/syscalls.h>
#include <magenta/syscalls/object.h>
#include <mxalloc/new.h>
#include <mxtl/intrusive_double_list.h>
#include <mxtl/unique_ptr.h>

#include <string.h>
#include <inttypes.h>

#include "macros.h"

using mxtl::unique_ptr;

namespace {

mx_status_t get_handle_koid(mx_handle_t handle, mx_koid_t* koid) {
    mx_info_handle_basic_t info;
    RETURN_IF_ERROR(mx_object_get_info(
        handle, MX_INFO_HANDLE_BASIC, &info, sizeof(info), nullptr, nullptr));
    *koid = info.koid;
    return MX_OK;
}

class RankedJob
    : public mxtl::DoublyLinkedListable<mxtl::unique_ptr<RankedJob>> {
public:
    static mx_status_t Create(mx_handle_t handle, unique_ptr<RankedJob>* out) {
        mx_koid_t koid;
        RETURN_IF_ERROR(get_handle_koid(handle, &koid));

        char name[MX_MAX_NAME_LEN];
        RETURN_IF_ERROR(
            mx_object_get_property(handle, MX_PROP_NAME, name, sizeof(name)));

        AllocChecker ac;
        out->reset(new (&ac) RankedJob(koid, name));
        if (!ac.check()) {
            return MX_ERR_NO_MEMORY;
        }
        return MX_OK;
    }

    mx_koid_t koid() const { return koid_; }
    const char* name() const { return name_; }

    // Type of linked lists of this class.
    using List = mxtl::DoublyLinkedList<mxtl::unique_ptr<RankedJob>>;

private:
    RankedJob(mx_koid_t koid, const char* name)
        : koid_(koid) {
        strlcpy(const_cast<char*>(name_), name, sizeof(name_));
    }

    const char name_[MX_MAX_NAME_LEN] = {};
    const mx_koid_t koid_;
};

RankedJob::List ranked_jobs;

RankedJob* get_ranked_job_by_koid(mx_koid_t koid) {
    for (auto& job : ranked_jobs) {
        if (job.koid() == koid) {
            return &job;
        }
    }
    return nullptr;
}

} // namespace

mx_status_t _fake_job_set_relative_importance(
    mx_handle_t root_resource,
    mx_handle_t job, mx_handle_t less_important_job) {

    // Make sure the root resource looks legit.
    mx_info_handle_basic_t info;
    RETURN_IF_ERROR(mx_object_get_info(root_resource, MX_INFO_HANDLE_BASIC,
                                       &info, sizeof(info), nullptr, nullptr));
    if (info.type != MX_OBJ_TYPE_RESOURCE) {
        return MX_ERR_WRONG_TYPE;
    }

    unique_ptr<RankedJob> rjob;
    {
        mx_koid_t koid;
        RETURN_IF_ERROR(get_handle_koid(job, &koid));
        RankedJob* j = get_ranked_job_by_koid(koid);
        if (j == nullptr) {
            RETURN_IF_ERROR(RankedJob::Create(job, &rjob));
        } else {
            rjob.reset(ranked_jobs.erase(*j).release());
        }
    }
    fflush(stdout);
    MX_ASSERT(!rjob->InContainer());

    if (less_important_job == MX_HANDLE_INVALID) {
        // Make this the least important.
        ranked_jobs.push_front(mxtl::move(rjob));
    } else {
        // Insert rjob just after less_important_job.
        mx_koid_t koid;
        RETURN_IF_ERROR(get_handle_koid(less_important_job, &koid));
        RankedJob* li_job = get_ranked_job_by_koid(koid);
        // Simplification: less_important_job must exist already. The real
        // syscall wouldn't have this restriction.
        MX_DEBUG_ASSERT(li_job != nullptr);
        ranked_jobs.insert_after(
            ranked_jobs.make_iterator(*li_job), mxtl::move(rjob));
    }
    return MX_OK;
}

void dump_importance_list() {
    printf("Least important:\n");
    for (const auto& rj : ranked_jobs) {
        printf("- k:%" PRIu64 " [%-*s]\n",
               rj.koid(), MX_MAX_NAME_LEN, rj.name());
    }
}
