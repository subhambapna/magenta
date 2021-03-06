# Copyright 2017 The Fuchsia Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_TYPE := driver

MODULE_SRCS += \
    $(LOCAL_DIR)/mmc.c \
    $(LOCAL_DIR)/sd.c \
    $(LOCAL_DIR)/sdmmc.c \

MODULE_STATIC_LIBS := system/ulib/ddk system/ulib/sync system/ulib/pretty

MODULE_LIBS := system/ulib/driver system/ulib/c system/ulib/magenta system/ulib/mxio

include make/module.mk
