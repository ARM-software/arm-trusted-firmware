/*
 * Copyright (C) 2016 Marvell International Ltd.
 *
 * SPDX-License-Identifier:	BSD-3-Clause
 * https://spdx.org/licenses
 */

#ifndef PLAT_MARVELL_H
#define PLAT_MARVELL_H

#include <stdint.h>

#include <common/bl_common.h>
#include <lib/cassert.h>
#include <lib/el3_runtime/cpu_data.h>
#include <lib/xlat_tables/xlat_tables_v2.h>

/*
 * Extern declarations common to Marvell standard platforms
 */
extern const mmap_region_t plat_marvell_mmap[];

#define MARVELL_CASSERT_MMAP						\
	CASSERT((ARRAY_SIZE(plat_marvell_mmap) + MARVELL_BL_REGIONS)	\
		<= MAX_MMAP_REGIONS,					\
		assert_max_mmap_regions)

/*
 * Utility functions common to Marvell standard platforms
 */
void marvell_setup_page_tables(uintptr_t total_base,
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

/* Console utility functions */
void marvell_console_boot_init(void);
void marvell_console_boot_end(void);
void marvell_console_runtime_init(void);
void marvell_console_runtime_end(void);

/* IO storage utility functions */
void marvell_io_setup(void);

/* Systimer utility function */
void marvell_configure_sys_timer(void);

/* Topology utility function */
int marvell_check_mpidr(u_register_t mpidr);

/* BL1 utility functions */
void marvell_bl1_early_platform_setup(void);
void marvell_bl1_platform_setup(void);
void marvell_bl1_plat_arch_setup(void);

/* BL2 utility functions */
void marvell_bl2_early_platform_setup(meminfo_t *mem_layout);
void marvell_bl2_platform_setup(void);
void marvell_bl2_plat_arch_setup(void);
uint32_t marvell_get_spsr_for_bl32_entry(void);
uint32_t marvell_get_spsr_for_bl33_entry(void);

/* BL31 utility functions */
void marvell_bl31_early_platform_setup(void *from_bl2,
				       uintptr_t soc_fw_config,
				       uintptr_t hw_config,
				       void *plat_params_from_bl2);
void marvell_bl31_platform_setup(void);
void marvell_bl31_plat_runtime_setup(void);
void marvell_bl31_plat_arch_setup(void);

/* FIP TOC validity check */
int marvell_io_is_toc_valid(void);

/*
 * PSCI functionality
 */
void marvell_psci_arch_init(int idx);
void plat_marvell_system_reset(void);

/*
 * Optional functions required in Marvell standard platforms
 */
void plat_marvell_io_setup(void);
int plat_marvell_get_alt_image_source(
	unsigned int image_id,
	uintptr_t *dev_handle,
	uintptr_t *image_spec);
unsigned int plat_marvell_calc_core_pos(u_register_t mpidr);

void plat_marvell_interconnect_init(void);
void plat_marvell_interconnect_enter_coherency(void);

const mmap_region_t *plat_marvell_get_mmap(void);

#endif /* PLAT_MARVELL_H */
