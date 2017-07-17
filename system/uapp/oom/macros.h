// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <magenta/status.h>
#include <stdio.h>

#define RETURN_IF_ERROR(x)                                             \
    do {                                                               \
        mx_status_t TRY_status__ = (x);                                \
        if (TRY_status__ != MX_OK) {                                   \
            printf("%s:%d: %s failed: %s (%d)\n",                      \
                    __func__, __LINE__, #x,                            \
                    mx_status_get_string(TRY_status__), TRY_status__); \
            return TRY_status__;                                       \
        }                                                              \
    } while (0)
