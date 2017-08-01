// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "dwc3.h"
#include "dwc3-regs.h"

#include <stdio.h>

static uint32_t dwc3_ep_cmd(dwc3_t* dwc, unsigned ep_num, uint32_t command, uint32_t param0,
                               uint32_t param1, uint32_t param2) {
    volatile void* mmio = dwc3_mmio(dwc);

    DWC3_WRITE32(mmio + DEPCMDPAR0(ep_num), param0);
    DWC3_WRITE32(mmio + DEPCMDPAR1(ep_num), param1);
    DWC3_WRITE32(mmio + DEPCMDPAR2(ep_num), param2);

    volatile void* depcmd = mmio + DEPCMD(ep_num);
    DWC3_WRITE32(depcmd, command);
    
    dwc3_wait_bits(depcmd, DEPCMD_CMDACT, 0);
    return DWC3_GET_BITS32(depcmd, DEPCMD_CMDSTATUS_START, DEPCMD_CMDSTATUS_BITS);
}

mx_status_t dwc3_cmd_start_config(dwc3_t* dwc, unsigned ep_num, unsigned resource_index) {

    dwc3_ep_cmd(dwc, ep_num, DEPSTARTCFG | DEPCMD_RESOURCE_INDEX(resource_index), 0, 0, 0);
    return MX_OK;
}

mx_status_t dwc3_cmd_ep_config_init(dwc3_t* dwc, unsigned ep_num, unsigned fifo_num,
                                    unsigned ep_type, unsigned max_packet_size, unsigned interval) {

    uint32_t param0 = DEPCFG_ACTION_INITIALIZE | DEPCFG_FIFO_NUM(fifo_num) |
                      DEPCFG_MAX_PACKET_SIZE(max_packet_size) | DEPCFG_EP_TYPE(ep_type);
    uint32_t param1 = DEPCFG_EP_NUMBER(ep_num) | DEPCFG_INTERVAL(interval) |
                      DEPCFG_XFER_NOT_READY_EN | DEPCFG_XFER_COMPLETE_EN |
                      DEPCFG_INTR_NUM(0);
    
    dwc3_ep_cmd(dwc, ep_num, DEPCFG, param0, param1, 0);
    return MX_OK;
}

mx_status_t dwc3_cmd_ep_transfer_config(dwc3_t* dwc, unsigned ep_num) {

     
    dwc3_ep_cmd(dwc, ep_num, DEPXFERCFG, 1, 0, 0);
    return MX_OK;
}
