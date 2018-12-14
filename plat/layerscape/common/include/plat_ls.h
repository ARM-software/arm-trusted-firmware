/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLAT_LS_H
#define PLAT_LS_H

#include <stdint.h>

#include <lib/el3_runtime/cpu_data.h>

/* BL1 utility functions */
void ls_bl1_platform_setup(void);
void ls_bl1_early_platform_setup(void);

/* BL2 utility functions */
void ls_bl2_early_platform_setup(meminfo_t *mem_layout);
uint32_t ls_get_spsr_for_bl32_entry(void);
uint32_t ls_get_spsr_for_bl33_entry(void);

/* BL3 utility functions */
void ls_bl31_early_platform_setup(void *from_bl2,
				void *plat_params_from_bl2);

/* IO storage utility functions */
void plat_ls_io_setup(void);


void ls_setup_page_tables(uintptr_t total_base,
			size_t total_size,
			uintptr_t code_start,
			uintptr_t code_limit,
			uintptr_t rodata_start,
			uintptr_t rodata_limit
#if USE_COHERENT_MEM
			, uintptr_t coh_start,
			uintptr_t coh_limit
#endif
);

/* PSCI utility functions */
int ls_check_mpidr(u_register_t mpidr);

/* Security utility functions */
int tzc380_setup(void);

/* Timer utility functions */
uint64_t ls_get_timer(uint64_t start);
void ls_delay_timer_init(void);

/* TSP utility functions */
void ls_tsp_early_platform_setup(void);

/* Helper functions */
unsigned int plat_ls_calc_core_pos(u_register_t mpidr);

/* others */
unsigned int plat_ls_get_cluster_core_count(u_register_t mpidr);

#endif /* PLAT_LS_H */
