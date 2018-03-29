/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arm_def.h>
#include <bl_common.h>
#include <ccn.h>
#include <debug.h>
#include <plat_arm.h>
#include <platform.h>
#include "../../../../bl1/bl1_private.h"

#if USE_COHERENT_MEM
/*
 * The next 2 constants identify the extents of the coherent memory region.
 * These addresses are used by the MMU setup code and therefore they must be
 * page-aligned.  It is the responsibility of the linker script to ensure that
 * __COHERENT_RAM_START__ and __COHERENT_RAM_END__ linker symbols
 * refer to page-aligned addresses.
 */
#define BL1_COHERENT_RAM_BASE (unsigned long)(&__COHERENT_RAM_START__)
#define BL1_COHERENT_RAM_LIMIT (unsigned long)(&__COHERENT_RAM_END__)
#define BL2_COHERENT_RAM_BASE (unsigned long)(&__COHERENT_RAM_START__)
#define BL2_COHERENT_RAM_LIMIT (unsigned long)(&__COHERENT_RAM_END__)

#define BL31_COHERENT_RAM_BASE (uintptr_t)(&__COHERENT_RAM_START__)
#define BL31_COHERENT_RAM_LIMIT (uintptr_t)(&__COHERENT_RAM_END__)
#endif

#define SGI_MAP_FLASH0_RO	MAP_REGION_FLAT(V2M_FLASH0_BASE,\
						V2M_FLASH0_SIZE,	\
						MT_DEVICE | MT_RO | MT_SECURE)
/*
 * Table of regions for different BL stages to map using the MMU.
 * This doesn't include Trusted RAM as the 'mem_layout' argument passed to
 * arm_configure_mmu_elx() will give the available subset of that.
 *
 * Replace or extend the below regions as required
 */
#if IMAGE_BL1
const mmap_region_t plat_arm_mmap[] = {
	ARM_MAP_SHARED_RAM,
	SGI_MAP_FLASH0_RO,
	CSS_SGI_MAP_DEVICE,
	SOC_CSS_MAP_DEVICE,
	{0}
};
#endif
#if IMAGE_BL2
const mmap_region_t plat_arm_mmap[] = {
	ARM_MAP_SHARED_RAM,
	SGI_MAP_FLASH0_RO,
	CSS_SGI_MAP_DEVICE,
	SOC_CSS_MAP_DEVICE,
	ARM_MAP_NS_DRAM1,
#if ARM_BL31_IN_DRAM
	ARM_MAP_BL31_SEC_DRAM,
#endif
	{0}
};
#endif
#if IMAGE_BL31
const mmap_region_t plat_arm_mmap[] = {
	ARM_MAP_SHARED_RAM,
	V2M_MAP_IOFPGA,
	CSS_SGI_MAP_DEVICE,
	SOC_CSS_MAP_DEVICE,
	{0}
};
#endif

ARM_CASSERT_MMAP

/*
 * Set up the page tables for the generic and platform-specific memory regions.
 * The extents of the generic memory regions are specified by the function
 * arguments and consist of:
 * - Trusted SRAM seen by the BL image;
 * - Code section;
 * - Read-only data section;
 * - Coherent memory region, if applicable.
 */

#if IMAGE_BL1
void bl1_plat_arch_setup(void)
{
	arm_setup_page_tables(ARM_BL_RAM_BASE,
			      ARM_BL_RAM_SIZE,
			      BL_CODE_BASE,
			      BL1_CODE_END,
			      BL1_RO_DATA_BASE,
			      BL1_RO_DATA_END
#if USE_COHERENT_MEM
			      , BL1_COHERENT_RAM_BASE,
			      BL1_COHERENT_RAM_LIMIT
#endif /* USE_COHERENT_MEM */
			     );

	enable_mmu_el3(0);
}
#endif /* IMAGE_BL1 */

#if IMAGE_BL2
void bl2_plat_arch_setup(void)
{
	arm_setup_page_tables(BL2_BASE,
			      BL2_LIMIT-BL2_BASE,
			      BL_CODE_BASE,
			      BL_CODE_END,
			      BL_RO_DATA_BASE,
			      BL_RO_DATA_END
#if USE_COHERENT_MEM
			      , BL2_COHERENT_RAM_BASE,
			      BL2_COHERENT_RAM_LIMIT
#endif /* USE_COHERENT_MEM */
			      );
	enable_mmu_el1(0);
}
#endif /* IMAGE_BL2 */
