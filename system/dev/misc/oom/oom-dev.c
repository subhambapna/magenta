// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ddk/binding.h>
#include <ddk/device.h>
#include <ddk/driver.h>

#include <magenta/syscalls.h>
#include <magenta/threads.h>
#include <magenta/types.h>

// ioctls: #include <magenta/device/oom.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>

static mx_status_t oom_ioctl(void* ctx, uint32_t op,
                             const void* cmd, size_t cmdlen,
                             void* reply, size_t max, size_t* out_actual) {
    switch (op) {
    // case IOCTL_OOM_START: {
    //     if (cmdlen != sizeof(uint32_t)) {
    //         return MX_ERR_INVALID_ARGS;
    //     }
    //     uint32_t group_mask = *(uint32_t *)cmd;
    //     return mx_oom_control(get_root_resource(), OOM_ACTION_START, group_mask, NULL);
    // }
    // case IOCTL_OOM_STOP: {
    //     mx_oom_control(get_root_resource(), OOM_ACTION_STOP, 0, NULL);
    //     mx_oom_control(get_root_resource(), OOM_ACTION_REWIND, 0, NULL);
    //     return MX_OK;
    // }
    // TODO: get a port that's notified on lowmem events, at certain thresholds,
    // requests for cache clearing, ...
    default:
        return MX_ERR_INVALID_ARGS;
    }
}

static /*const*/ mx_protocol_device_t oom_device_proto = {
    .version = DEVICE_OPS_VERSION,
    .ioctl = oom_ioctl,
};

static int ranker_thread(void *arg) {
    while (true) {
        mx_nanosleep(mx_deadline_after(MX_SEC(1)));
    }
    return 0;
}

// Type of the |cookie| args to mx_driver_ops_t functions.
typedef struct {
    mx_device_t* dev;
    thrd_t ranker_thread;
} oom_cookie_t;

static mx_status_t oom_bind(void* unused_ctx, mx_device_t* parent,
                            void** out_cookie) {
    // Add the device.
    /*const*/ device_add_args_t args = {
        .version = DEVICE_ADD_ARGS_VERSION,
        .name = "oom",
        // TODO(dbort): Use .ctx to hold a channel to the ranker thread, and
        // maybe dispatchers that will notify observers.
        .ops = &oom_device_proto,
    };
    mx_device_t* dev;
    mx_status_t s = device_add(parent, &args, &dev);
    if (s != MX_OK) {
        return s;
    }

    // Allocate the cookie.
    oom_cookie_t *cookie = (oom_cookie_t*)calloc(1, sizeof(oom_cookie_t));
    if (cookie == NULL) {
        device_remove(dev);
        return MX_ERR_NO_MEMORY;
    }
    cookie->dev = dev;

    // Start the thread.
    int ts = thrd_create_with_name(
        &cookie->ranker_thread, ranker_thread, NULL, "ranker");
    if (ts != thrd_success) {
        device_remove(dev);
        free(cookie);
        return thrd_status_to_mx_status(ts);
    }

    *out_cookie = cookie;
    return MX_OK;
}

void oom_unbind(void* unused_ctx, mx_device_t* parent, void* cookie) {
    //xxx kill, join the thread. Ask it to die nicely
    // thrd_join(cookie->ranker_thread, NULL);
    memset(cookie, 0, sizeof(oom_cookie_t));
    free(cookie);
}

static /*const*/ mx_driver_ops_t oom_driver_ops = {
    .version = DRIVER_OPS_VERSION,
    .bind = oom_bind,
    .unbind = oom_unbind,
};

MAGENTA_DRIVER_BEGIN(oom, oom_driver_ops, "magenta", "0.1", 1)
BI_MATCH_IF(EQ, BIND_PROTOCOL, MX_PROTOCOL_MISC_PARENT)
,
    MAGENTA_DRIVER_END(oom)
