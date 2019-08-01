/*
 * Copyright (c) 2019, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_PRIVATE_H
#define PLATFORM_PRIVATE_H

/*******************************************************************************
 * Function and variable prototypes
 ******************************************************************************/
void socfgpa_configure_mmu_el3(unsigned long total_base,
			unsigned long total_size,
			unsigned long ro_start,
			unsigned long ro_limit,
			unsigned long coh_start,
			unsigned long coh_limit);


void socfpga_configure_mmu_el1(unsigned long total_base,
			unsigned long total_size,
			unsigned long ro_start,
			unsigned long ro_limit,
			unsigned long coh_start,
			unsigned long coh_limit);

void socfpga_delay_timer_init(void);

void socfpga_gic_driver_init(void);

uint32_t socfpga_get_spsr_for_bl32_entry(void);

uint32_t socfpga_get_spsr_for_bl33_entry(void);

unsigned long socfpga_get_ns_image_entrypoint(void);


#endif /* PLATFORM_PRIVATE_H */
