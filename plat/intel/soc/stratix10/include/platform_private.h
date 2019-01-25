/*
 * Copyright (c) 2019, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __PLATFORM_PRIVATE_H__
#define __PLATFORM_PRIVATE_H__
#include <common/bl_common.h>

/*******************************************************************************
 * Function and variable prototypes
 ******************************************************************************/
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

void plat_gic_driver_init(void);

void plat_arm_gic_init(void);

void plat_delay_timer_init(void);

unsigned long plat_get_ns_image_entrypoint(void);

uint32_t plat_get_spsr_for_bl32_entry(void);

uint32_t plat_get_spsr_for_bl33_entry(void);

#endif /* __PLATFORM_PRIVATE_H__ */
