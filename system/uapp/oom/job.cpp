// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "job.h"

#include <magenta/status.h>
#include <magenta/syscalls.h>
#include <mx/handle.h>
#include <mxalloc/new.h>
#include <mxtl/algorithm.h>

#include <string.h>

#include "macros.h"

using mxtl::unique_ptr;

mx_status_t Job::Create(mx_koid_t koid, mx_handle_t handle,
                        const char* name, importance_t importance,
                        const Job* parent, unique_ptr<Job>* out) {
    mx::handle job{mx::handle(handle)};

    // Importance keys have the form:
    //   <capped-importance> ':' <importance-chain> '~'
    //
    // See job.h for details.
    //
    // Example values:
    //   32:c8c8c896c83264~
    //   96:c8c8c8c896~
    //   c8:c8~
    unique_ptr<char[]> ik; // Importance key.
    AllocChecker ac;
    importance_t capped_importance;
    if (parent == nullptr) {
        capped_importance = importance;

        const size_t ik_len = sizeof("xx:xx~");
        ik.reset(new (&ac) char[ik_len]);
        if (!ac.check()) {
            return MX_ERR_NO_MEMORY;
        }
        MX_DEBUG_ASSERT(importance <= 0xff); // Generate two characters
        snprintf(ik.get(), ik_len, "%02x:%02x~",
                 capped_importance, importance);
    } else {
        capped_importance = mxtl::min(importance, parent->capped_importance());

        // Make sure our parent's importance key (PIK) has the expected
        // structure so we can safely use substrings of it.
        const char* pik = parent->importance_key();
        const size_t pik_len = strlen(pik);
        MX_DEBUG_ASSERT(pik_len >= 4);
        MX_DEBUG_ASSERT(pik[2] == ':');
        MX_DEBUG_ASSERT(pik[pik_len - 1] == '~');

        // Our importance key (IK) will add two characters to our parent's
        // importance key.
        const size_t ik_len = pik_len + 2 + 1;
        ik.reset(new (&ac) char[ik_len]);
        if (!ac.check()) {
            return MX_ERR_NO_MEMORY;
        }

        // Generate our importance key.
        const char* pik_substr = pik + 3; // Skip leading 'xx:'
        const size_t pik_substr_len = pik_len - 4; // And skip trailing '~'
        MX_DEBUG_ASSERT(importance <= 0xff); // Generate two characters
        snprintf(ik.get(), ik_len, "%02x:%.*s%02x~",
                 capped_importance,
                 static_cast<int>(pik_substr_len), pik_substr,
                 importance);
    }

    unique_ptr<char[]> namep;
    namep.reset(new (&ac) char[strlen(name) + 1]);
    if (!ac.check()) {
        return MX_ERR_NO_MEMORY;
    }
    strcpy(namep.get(), name);

    // Create and return the actual Job.
    out->reset(new (&ac) Job(koid, job.release(), mxtl::move(namep),
                             importance, capped_importance, mxtl::move(ik)));
    if (!ac.check()) {
        return MX_ERR_NO_MEMORY;
    }
    return MX_OK;
}

Job::Job(mx_koid_t koid, mx_handle_t handle,
         unique_ptr<char[]> name, importance_t importance,
         importance_t capped_importance, unique_ptr<char[]> importance_key)
    : koid_(koid), handle_(handle), name_(mxtl::move(name)),
      importance_(importance), capped_importance_(capped_importance),
      importance_key_(mxtl::move(importance_key)) {
}

Job::~Job() {
    if (handle_ != MX_HANDLE_INVALID) {
        mx_handle_close(handle_);
    }
}

void sort_jobs_by_importance_key(Job::List *jobs) {
    Job::List sorted;

    // Insertion-sort |jobs| into |sorted|, then swap the lists before
    // returning.
    // TODO(dbort): This goes quadratic easily; Mergesort would be better.
    while (!jobs->is_empty()) {
        unique_ptr<Job> job(mxtl::move(jobs->pop_front()));
        if (sorted.is_empty()) {
            sorted.push_front(mxtl::move(job));
        } else {
            auto iter = sorted.begin();
            while (iter != sorted.end()) {
                int cmp = strcmp(iter->importance_key(), job->importance_key());
                if (cmp >= 0) {
                    // The current item is of greater or equal importance to us;
                    // insert before it.
                    sorted.insert(iter, mxtl::move(job));
                    break;
                }
                iter++;
            }
            if (job != nullptr) {
                // Most important job.
                sorted.push_back(mxtl::move(job));
            }
        }
    }
    jobs->swap(sorted);
}
