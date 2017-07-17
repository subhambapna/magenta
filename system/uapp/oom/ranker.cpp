// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//xxx this should be some kind of service, maybe a device
/*xxx
Wish list:
- Notification when job importances change (permissions can be tough)
- Notification on job creation/death
  - Process creation/death would be nice too
  - Could be a job-level channel that watches for immediate children
    or all descendants. Kinda looks like inotify, if there's a namespace
    for jobs

A bunch of this can happen down in the kernel if this userspace stuff goes away
*/

#include <magenta/status.h>
#include <magenta/syscalls.h>
#include <magenta/syscalls/object.h>
#include <mxalloc/new.h>
#include <task-utils/walker.h>

#include <inttypes.h>
#include <string.h>

#include "canned_jobs.h"
#include "job.h"
#include "macros.h"
#include "resources.h"

using mxtl::unique_ptr;

// Builds a list of all jobs in the system.
class JobWalker final : private TaskEnumerator {
public:
    static mx_status_t BuildList(Job::List* jobs) {
        AllocChecker ac;
        unique_ptr<const Job* []> stack(new (&ac) const Job*[kMaxDepth]);
        if (!ac.check()) {
            return MX_ERR_NO_MEMORY;
        }
        mx_status_t s =
            JobWalker(jobs, mxtl::move(stack)).WalkRootJobTree();
        if (s != MX_OK) {
            return s;
        }
        return MX_OK;
    }

private:
    JobWalker(Job::List* jobs, unique_ptr<const Job* []> stack)
        : jobs_(jobs), stack_(mxtl::move(stack)) {
        memset(stack_.get(), 0, kMaxDepth);
    }

    mx_status_t OnJob(int depth, mx_handle_t handle,
                      mx_koid_t koid, mx_koid_t parent_koid) override {
        MX_ASSERT(depth >= 0);
        MX_ASSERT(depth < kMaxDepth);
        // Make sure our entry on the stack won't point to a stale entry
        // if we fail before inserting ourselves.
        stack_[depth] = nullptr;
        // Clear a few more entries to highlight any bugs in this code.
        stack_[depth + 1] = nullptr;
        stack_[depth + 2] = nullptr;
        stack_[depth + 3] = nullptr;

        const Job* parent;
        if (depth == 0) {
            // Root job.
            parent = nullptr;
        } else {
            parent = stack_[depth - 1];
            MX_ASSERT(parent != nullptr);
        }

        mx_handle_t dup;
        mx_status_t s = mx_handle_duplicate(handle, MX_RIGHT_SAME_RIGHTS, &dup);
        if (s != MX_OK) {
            fprintf(stderr,
                    "ERROR: duplicating handle for job %" PRIu64 ": %s (%d)\n",
                    koid, mx_status_get_string(s), s);
            dup = MX_HANDLE_INVALID;
        }

        // Read some object properties.
        // TODO: Don't stop walking the tree if one job is bad; it might have
        // just died. Watch out for the walker visiting its children, though;
        // maybe put a tombstone in the stack.
        MX_DEBUG_ASSERT(handle != MX_HANDLE_INVALID);
        char name[MX_MAX_NAME_LEN];
        RETURN_IF_ERROR(mx_object_get_property(dup, MX_PROP_NAME,
                                               name, sizeof(name)));
        uint32_t importance;
        RETURN_IF_ERROR(mx_object_get_property(dup, MX_PROP_JOB_IMPORTANCE,
                                               &importance, sizeof(uint32_t)));

        unique_ptr<Job> job;
        RETURN_IF_ERROR(Job::Create(koid, dup, name, importance, parent, &job));

        // Push ourselves on the stack so our children can find us.
        stack_[depth] = job.get();

        jobs_->push_back(mxtl::move(job));
        return MX_OK;
    }

    bool has_on_job() const override { return true; }

    Job::List* const jobs_;

    static constexpr int kMaxDepth = 128;
    unique_ptr<const Job* []> stack_; // kMaxDepth entries
};

#define FAKE_RANKING 0
#if FAKE_RANKING
#include "fake_syscalls.h"
#undef mx_job_set_relative_importance
#define mx_job_set_relative_importance _fake_job_set_relative_importance
#else // !FAKE_RANKING
#define dump_importance_list(...) ((void)0)
#endif // !FAKE_RANKING

static mx_status_t do_job_stuff() {
    Job::List jobs;
    RETURN_IF_ERROR(JobWalker::BuildList(&jobs));
    sort_jobs_by_importance_key(&jobs);

    mx_handle_t root_resource;
    RETURN_IF_ERROR(get_root_resource(&root_resource));

    mx_handle_t less_important_job = MX_HANDLE_INVALID;
    for (const auto& job : jobs) {
        printf("+ k:%" PRIu64 " [%-*s] |i=%02x, c=%02x| %s\n",
               job.koid(), MX_MAX_NAME_LEN, job.name(),
               job.importance(),
               job.capped_importance(),
               job.importance_key());
        RETURN_IF_ERROR(mx_job_set_relative_importance(
            root_resource, job.handle(), less_important_job));
        less_important_job = job.handle();
    }

    dump_importance_list();
    return MX_OK;
}

int main(int argc, char** argv) {
    //xxx add an arg to create these jobs, and to dump the list.
    //xxx though in the long run this will be a service/device.
    unique_ptr<JobStack> jobs; // Keeps job handles alive.
    create_test_jobs_under(mx_job_default(), &jobs);
    mx_status_t s = do_job_stuff();
    if (s != MX_OK) {
        fprintf(stderr, "Ranking failed: %s (%d)\n",
                mx_status_get_string(s), s);
        return 1;
    }
    return 0;
}
