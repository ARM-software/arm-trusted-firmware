/*
 * Copyright (c) 2015-2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CMN_SEC_H
#define CMN_SEC_H

#include <stdint.h>

#define SECURE_MASTER 0
#define NS_MASTER 1

void tz_master_default_cfg(void);
void tz_usb_ns_master_set(uint32_t ns);
void tz_sdio_ns_master_set(uint32_t ns);

#endif
