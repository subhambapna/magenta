// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "dwc3.h"
#include "dwc3-regs.h"

#include <stdio.h>

#define EVENT_BUFFER_SIZE   PAGE_SIZE

static int dwc_irq_thread(void* arg) {
    usb_dwc3_t* dwc = arg;
    volatile void* mmio = dwc->mmio.vaddr;

/*
    uint32_t* ring_start = io_buffer_virt(&dwc->event_buffer);
    uint32_t* ring_cur = ring_start;
    uint32_t* ring_end = (void *)ring_start + EVENT_BUFFER_SIZE;
 */   
    while (1) {
        mx_status_t status = mx_interrupt_wait(dwc->irq_handle);
        if (status != MX_OK) {
            printf("mx_interrupt_wait returned %d\n", status);
            mx_interrupt_complete(dwc->irq_handle);
            break;
        }
        mx_interrupt_complete(dwc->irq_handle);
        printf("dwc_irq_thread got interrupt\n");

        uint32_t event_count = DWC3_READ32(GEVNTCOUNT(0)) & GEVNTCOUNT_EVNTCOUNT_MASK;
        if (event_count > 0) {
            
        
            DWC3_WRITE32(mmio + GEVNTCOUNT(0), event_count);    
        }
    }

    printf("dwc_irq_thread done\n");
    return 0;
}

mx_status_t dwc3_events_init(usb_dwc3_t* dwc) {
    // allocate event buffer
    mx_status_t status = io_buffer_init(&dwc->event_buffer, EVENT_BUFFER_SIZE, IO_BUFFER_RW);
    if (status != MX_OK) {
        return status;
    }

    volatile void* mmio = dwc->mmio.vaddr;

    // set event buffer pointer and size
    // keep interrupts masked until we are ready
    DWC3_WRITE64(mmio + GEVNTADR(0), io_buffer_phys(&dwc->event_buffer));
    DWC3_WRITE32(mmio + GEVNTSIZ(0), EVENT_BUFFER_SIZE | GEVNTSIZ_EVNTINTRPTMASK);

    return MX_OK;
}

void dwc3_events_start(usb_dwc3_t* dwc) {

    // enable events
    volatile void* mmio = dwc->mmio.vaddr;
    DWC3_WRITE32(mmio + GEVNTCOUNT(0), 0);

    uint32_t event_mask = DEVTEN_ULSTCNGEN | \
                          DEVTEN_CONNECTDONEEVTEN | \
                          DEVTEN_USBRSTEVTEN | \
                          DEVTEN_DISSCONNEVTEN;
    DWC3_WRITE32(mmio + DEVTEN, event_mask);

    thrd_create_with_name(&dwc->irq_thread, dwc_irq_thread, dwc, "dwc_irq_thread");
}
