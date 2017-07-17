// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <magenta/types.h>
#include <mxtl/intrusive_double_list.h>
#include <mxtl/unique_ptr.h>

// A running Magenta job, with an open handle and cached properties.
//
// Defines a globally-sortable "importance key" that can be used to
// rank all jobs by relative importance.
//
// Importance keys have the form:
//
//   <capped-importance> ':' <importance-chain> '~'
//
// <capped-importance> is the Job's capped_importance() as two hex digits.
// <importance-chain> is the concatenation of the importance() values of a
// Job's ancestors and itself, with the root-most Job's importance first.
//
// This produces a key that, when sorted using strcmp(), will cluster Jobs
// with the same capped_importance(), then break ties using a Job's
// ancestors' importances. The trailing '~' causes shorter strings to sort
// higher than longer strings, ensuring that parents are more important
// than their children.
//
// Example values:
//
//     32:c8c8c896c83264~ << A deep BACKGROUND job, capped to DISPOSABLE
//     32:c8c8c896c832~
//     32:c8c8c896c89632~
//     64:c8c8c896c86464~
//     64:c8c8c896c864c8~
//     96:c8c8c896c8~     << A CRITICAL job, capped to FOREGROUND
//     96:c8c8c896~       << The previous job's FOREGROUND parent
//     96:c8c8c8c896~
//     c8:c8c8~
//     c8:c8~
//
// TODO(dbort): Consider using a similarly-structured uint64_t instead of a
// string.
class Job : public mxtl::DoublyLinkedListable<mxtl::unique_ptr<Job>> {
public:
    using importance_t = uint32_t;

    // Takes ownership of |handle|, even on failure. To aid testability, this
    // class must not perform any syscalls on |handle| except mx_handle_close.
    static mx_status_t Create(
        mx_koid_t koid, mx_handle_t handle,
        const char* name, importance_t importance,
        const Job* parent, mxtl::unique_ptr<Job>* out);

    ~Job();

    mx_koid_t koid() const { return koid_; }
    const char* name() const { return name_.get(); }

    // The importance returned by MX_PROP_JOB_IMPORTANCE.
    importance_t importance() const { return importance_; }

    // Like importance(), but capped to be no more important
    // than any ancestor.
    importance_t capped_importance() const {
        return capped_importance_;
    }

    // An opaque, strcmp()-comparable string for sorting Jobs by their global
    // relative importance. Higher importance will sort higher.
    // See the class comment for a description of the keys.
    const char* importance_key() const { return importance_key_.get(); }

    // Returns the handle but maintains ownership.
    mx_handle_t handle() const { return handle_; }

    // Type for linked lists of this class.
    using List = mxtl::DoublyLinkedList<mxtl::unique_ptr<Job>>;

private:
    Job(mx_koid_t koid, mx_handle_t handle, mxtl::unique_ptr<char[]> name,
        importance_t importance, importance_t capped_importance,
        mxtl::unique_ptr<char[]> importance_key);

    const mx_koid_t koid_;

    // An open handle to the underlying job object, or MX_HANDLE_INVALID.
    // Closed when this object is destroyed.
    const mx_handle_t handle_;

    const mxtl::unique_ptr<char[]> name_;
    const importance_t importance_;
    const importance_t capped_importance_;
    const mxtl::unique_ptr<char[]> importance_key_;
};

// Reorders |jobs| so entries are ordered by importance_key(), with
// the least-important job first.
void sort_jobs_by_importance_key(Job::List *jobs);
