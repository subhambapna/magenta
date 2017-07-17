// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <mx/job.h>
#include <mxtl/array.h>
#include <mxtl/macros.h>
#include <mxtl/unique_ptr.h>

// Holds onto job handles and keeps them alive.
// TODO: Consider making this object<T>-templated and move into mx.
class JobStack : private mxtl::Array<mx::job> {
public:
    static JobStack *Create(size_t count);

    // Swaps the job onto the stack.
    void push(mx::job* job);

    // Returns the most-recently-pushed job.
    mx::job& top() const;

private:
    JobStack(mx::job* array, size_t count);
    DISALLOW_COPY_ASSIGN_AND_MOVE(JobStack);

    size_t next_;
};

// Creates a canned tree of jobs under the specified root job.
// Does not create any processes.
mx_status_t create_test_jobs_under(
    mx_handle_t root, mxtl::unique_ptr<JobStack>* jobs);
