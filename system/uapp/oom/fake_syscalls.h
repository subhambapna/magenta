// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Fake syscalls for testing.

#pragma once

#include <magenta/compiler.h>
#include <magenta/types.h>

__BEGIN_CDECLS

// Fake version of mx_job_set_relative_importance.
mx_status_t _fake_job_set_relative_importance(
    mx_handle_t root_resource,
    mx_handle_t job, mx_handle_t less_important_job);

//xxx replace this with something that returns an array of koid+name
void dump_importance_list(void);

__END_CDECLS
