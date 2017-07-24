// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <ddk/device.h>
#include <ddk/io-buffer.h>
#include <ddk/protocol/platform-device.h>
#include <magenta/types.h>
#include <threads.h>

typedef struct {
    mx_device_t* mxdev;
    usb_dci_interface_t dci_intf;
    pdev_mmio_buffer_t usb3otg;
    io_buffer_t event_buffer;
    mx_handle_t irq_handle;
    thrd_t irq_thread;
} usb_dwc3_t;
