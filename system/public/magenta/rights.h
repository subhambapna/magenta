// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <magenta/types.h>

// Default rights for objects.
#define MX_DEFAULT_CHANNEL_RIGHTS \
  (MX_RIGHT_TRANSFER | MX_RIGHT_READ | MX_RIGHT_WRITE)

#define MX_DEFAULT_EVENT_RIGHTS \
  (MX_RIGHT_DUPLICATE | MX_RIGHT_TRANSFER | MX_RIGHT_READ | MX_RIGHT_WRITE)

#define MX_DEFAULT_EVENT_PAIR_RIGHTS \
  (MX_RIGHT_DUPLICATE | MX_RIGHT_TRANSFER | MX_RIGHT_READ | MX_RIGHT_WRITE)

#define MX_DEFAULT_FIFO_RIGHTS \
  (MX_RIGHT_TRANSFER | MX_RIGHT_DUPLICATE | MX_RIGHT_READ | MX_RIGHT_WRITE)

#define MX_DEFAULT_GUEST_RIGHTS \
  (MX_RIGHT_READ | MX_RIGHT_WRITE | MX_RIGHT_EXECUTE)

#define MX_DEFAULT_HYPERVISOR_RIGHTS MX_RIGHT_EXECUTE

#define MX_DEFAULT_INTERRUPT_RIGHTS \
  (MX_RIGHT_TRANSFER | MX_RIGHT_READ | MX_RIGHT_WRITE)

#define MX_DEFAULT_IO_MAPPING_RIGHTS MX_RIGHT_READ

#define MX_DEFAULT_JOB_RIGHTS                                                \
  (MX_RIGHT_TRANSFER | MX_RIGHT_DUPLICATE | MX_RIGHT_READ | MX_RIGHT_WRITE | \
   MX_RIGHT_ENUMERATE | MX_RIGHT_DESTROY | MX_RIGHT_GET_PROPERTY |           \
   MX_RIGHT_SET_PROPERTY | MX_RIGHT_SET_POLICY | MX_RIGHT_GET_POLICY)

#define MX_DEFAULT_LOG_RIGHTS \
  (MX_RIGHT_TRANSFER | MX_RIGHT_WRITE | MX_RIGHT_DUPLICATE)

#define MX_DEFAULT_PCI_DEVICE_RIGHTS \
  (MX_RIGHT_READ | MX_RIGHT_WRITE | MX_RIGHT_DUPLICATE | MX_RIGHT_TRANSFER)
#define MX_DEFAULT_PCI_INTERRUPT_RIGHTS \
  (MX_RIGHT_READ | MX_RIGHT_WRITE | MX_RIGHT_TRANSFER)

#define MX_DEFAULT_IO_PORT_RIGHTS \
  (MX_RIGHT_DUPLICATE | MX_RIGHT_TRANSFER | MX_RIGHT_READ | MX_RIGHT_WRITE)
#define MX_DEFAULT_IO_PORT_V2_RIGHTS \
  (MX_RIGHT_DUPLICATE | MX_RIGHT_TRANSFER | MX_RIGHT_READ | MX_RIGHT_WRITE)

#define MX_DEFAULT_PROCESS_RIGHTS                                            \
  (MX_RIGHT_READ | MX_RIGHT_WRITE | MX_RIGHT_DUPLICATE | MX_RIGHT_TRANSFER | \
   MX_RIGHT_ENUMERATE | MX_RIGHT_DESTROY | MX_RIGHT_GET_PROPERTY |           \
   MX_RIGHT_SET_PROPERTY)

#define MX_DEFAULT_RESOURCE_RIGHTS \
  (MX_RIGHT_READ | MX_RIGHT_WRITE | MX_RIGHT_DUPLICATE | MX_RIGHT_TRANSFER)

#define MX_DEFAULT_SOCKET_RIGHTS \
  (MX_RIGHT_TRANSFER | MX_RIGHT_DUPLICATE | MX_RIGHT_READ | MX_RIGHT_WRITE)

#define MX_DEFAULT_THREAD_RIGHTS                                             \
  (MX_RIGHT_READ | MX_RIGHT_WRITE | MX_RIGHT_DUPLICATE | MX_RIGHT_TRANSFER | \
   MX_RIGHT_DESTROY | MX_RIGHT_GET_PROPERTY | MX_RIGHT_SET_PROPERTY)

#define MX_DEFAULT_TIMERS_RIGHTS \
  (MX_RIGHT_DUPLICATE | MX_RIGHT_TRANSFER | MX_RIGHT_READ | MX_RIGHT_WRITE)

#define MX_DEFAULT_VMAR_RIGHTS (MX_RIGHT_DUPLICATE | MX_RIGHT_TRANSFER)

#define MX_DEFAULT_VMO_RIGHTS                                                \
  (MX_RIGHT_DUPLICATE | MX_RIGHT_TRANSFER | MX_RIGHT_READ | MX_RIGHT_WRITE | \
   MX_RIGHT_EXECUTE | MX_RIGHT_MAP | MX_RIGHT_GET_PROPERTY |                 \
   MX_RIGHT_SET_PROPERTY)
