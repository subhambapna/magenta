// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <ddk/device.h>
#include <ddk/io-buffer.h>
#include <ddk/protocol/platform-device.h>
#include <ddk/protocol/usb-dci.h>
#include <magenta/types.h>
#include <threads.h>

#include "dwc3-types.h"

typedef struct {
    io_buffer_t fifo_buffer;
    dwc3_trb_t* fifo_start;
    dwc3_trb_t* fifo_current;
    dwc3_trb_t* fifo_last;
} dwc3_endpoint_t;

#define DWC3_MAX_EPS    32

typedef struct {
    mx_device_t* mxdev;
    usb_dci_interface_t dci_intf;
    pdev_mmio_buffer_t mmio;

    // event stuff
    io_buffer_t event_buffer;
    mx_handle_t irq_handle;
    thrd_t irq_thread;

    dwc3_endpoint_t eps[DWC3_MAX_EPS];
} dwc3_t;

static inline volatile void* dwc3_mmio(dwc3_t* dwc) {
    return dwc->mmio.vaddr;
}

// Commands
mx_status_t dwc3_cmd_start_config(dwc3_t* dwc, unsigned ep_num, unsigned resource_index);
mx_status_t dwc3_cmd_ep_config_init(dwc3_t* dwc, unsigned ep_num, unsigned fifo_num,
                                    unsigned ep_type, unsigned max_packet_size, unsigned interval);
mx_status_t dwc3_cmd_ep_transfer_config(dwc3_t* dwc, unsigned ep_num);

// Endpoints
mx_status_t dwc3_ep_init(dwc3_t* dwc, unsigned ep_num);
void dwc3_ep_release(dwc3_t* dwc, unsigned ep_num);
mx_status_t dwc3_ep0_enable(dwc3_t* dwc);

// Events
mx_status_t dwc3_events_init(dwc3_t* dwc);
void dwc3_events_start(dwc3_t* dwc);

// Utils
void dwc3_wait_bits(volatile uint32_t* ptr, uint32_t bits, uint32_t expected);
