/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __PLAT_PRIVATE_H__
#define __PLAT_PRIVATE_H__

#include <bl_common.h>
#include "hi3798cv200.h"

void plat_configure_mmu_el3(unsigned long total_base,
			    unsigned long total_size,
			    unsigned long ro_start,
			    unsigned long ro_limit,
			    unsigned long coh_start,
			    unsigned long coh_limit);

void plat_configure_mmu_el1(unsigned long total_base,
			    unsigned long total_size,
			    unsigned long ro_start,
			    unsigned long ro_limit,
			    unsigned long coh_start,
			    unsigned long coh_limit);

void plat_delay_timer_init(void);
void plat_io_setup(void);

#endif /* __PLAT_PRIVATE_H__ */
