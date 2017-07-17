# Copyright 2017 The Fuchsia Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR).ranker

MODULE_NAME := oom-ranker

MODULE_TYPE := userapp

MODULE_SRCS += \
    $(LOCAL_DIR)/canned_jobs.cpp \
    $(LOCAL_DIR)/job.cpp \
    $(LOCAL_DIR)/ranker.cpp \
    $(LOCAL_DIR)/resources.c

# Tests
MODULE_SRCS += \
    $(LOCAL_DIR)/fake_syscalls.cpp

MODULE_STATIC_LIBS := \
    system/ulib/task-utils \
    system/ulib/mxalloc \
    system/ulib/mxcpp \
    system/ulib/mx \
    system/ulib/mxtl

MODULE_LIBS := \
    system/ulib/mxio \
    system/ulib/magenta \
    system/ulib/c

include make/module.mk
