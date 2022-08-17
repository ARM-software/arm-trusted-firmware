/*
 * Copyright (c) 2022, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DFD_H
#define DFD_H

#include <arch_helpers.h>
#include <lib/mmio.h>

void dfd_resume(void);
void dfd_setup(uint64_t base_addr, uint64_t chain_length, uint64_t cache_dump);

#endif /* DFD_H */
