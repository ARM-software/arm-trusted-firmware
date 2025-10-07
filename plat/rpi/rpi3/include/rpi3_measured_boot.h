/*
 * Copyright (c) 2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef RPI3_MEASURED_BOOT_H
#define RPI3_MEASURED_BOOT_H

#include <stdint.h>

#include <arch_helpers.h>
#include <common/measured_boot.h>

void rpi3_mboot_fetch_eventlog_info(uint8_t **eventlog_addr, size_t *eventlog_size);

int rpi3_set_nt_fw_info(size_t log_size, uintptr_t *ns_log_addr);

#endif /* RPI3_MEASURED_BOOT_H */
