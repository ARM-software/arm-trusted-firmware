/*
 * Copyright 2018-2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <assert.h>

#include <arch.h>
#include <arch_helpers.h>
#include <common/debug.h>
#include <lib/mmio.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <mmu_def.h>
#include <plat/common/platform.h>

#include "plat_common.h"
#include "platform_def.h"

const mmap_region_t *plat_ls_get_mmap(void);

/*
 * Table of memory regions for various BL stages to map using the MMU.
 * This doesn't include Trusted SRAM as arm_setup_page_tables() already
 * takes care of mapping it.
 *
 * The flash needs to be mapped as writable in order to erase the FIP's Table of
 * Contents in case of unrecoverable error (see plat_error_handler()).
 */
#ifdef IMAGE_BL2
const mmap_region_t plat_ls_mmap[] = {
	LS_MAP_CCSR,
	{0}
};
#endif

#ifdef IMAGE_BL31
const mmap_region_t plat_ls_mmap[] = {
	LS_MAP_CCSR,
#ifdef NXP_DCSR_ADDR
	LS_MAP_DCSR,
#endif
	LS_MAP_OCRAM,
	{0}
};
#endif
#ifdef IMAGE_BL32
const mmap_region_t plat_ls_mmap[] = {
	LS_MAP_CCSR,
	LS_MAP_BL32_SEC_MEM,
	{0}
};
#endif

/* Weak definitions may be overridden in specific NXP SoC */
#pragma weak plat_get_ns_image_entrypoint
#pragma weak plat_ls_get_mmap

#if defined(IMAGE_BL31) || !defined(CONFIG_DDR_FIP_IMAGE)
static void mmap_add_ddr_regions_statically(void)
{
	int i = 0;
	dram_regions_info_t *info_dram_regions = get_dram_regions_info();
	/* MMU map for Non-Secure DRAM Regions */
	VERBOSE("DRAM Region %d: %p - %p\n", i,
			(void *) info_dram_regions->region[i].addr,
			(void *) (info_dram_regions->region[i].addr
				+ info_dram_regions->region[i].size
				- 1));
	mmap_add_region(info_dram_regions->region[i].addr,
			info_dram_regions->region[i].addr,
			info_dram_regions->region[i].size,
			MT_MEMORY | MT_RW | MT_NS);

	/* MMU map for Secure DDR Region on DRAM-0 */
	if (info_dram_regions->region[i].size >
		(NXP_SECURE_DRAM_SIZE + NXP_SP_SHRD_DRAM_SIZE)) {
		VERBOSE("Secure DRAM Region %d: %p - %p\n", i,
			(void *) (info_dram_regions->region[i].addr
				+ info_dram_regions->region[i].size),
			(void *) (info_dram_regions->region[i].addr
				+ info_dram_regions->region[i].size
				+ NXP_SECURE_DRAM_SIZE
				+ NXP_SP_SHRD_DRAM_SIZE
				- 1));
		mmap_add_region((info_dram_regions->region[i].addr
				+ info_dram_regions->region[i].size),
				(info_dram_regions->region[i].addr
				+ info_dram_regions->region[i].size),
				(NXP_SECURE_DRAM_SIZE + NXP_SP_SHRD_DRAM_SIZE),
				MT_MEMORY | MT_RW | MT_SECURE);
	}

#ifdef IMAGE_BL31
	for (i = 1; i < info_dram_regions->num_dram_regions; i++) {
		if (info_dram_regions->region[i].size == 0)
			break;
		VERBOSE("DRAM Region %d: %p - %p\n", i,
			(void *) info_dram_regions->region[i].addr,
			(void *) (info_dram_regions->region[i].addr
				+ info_dram_regions->region[i].size
				- 1));
		mmap_add_region(info_dram_regions->region[i].addr,
				info_dram_regions->region[i].addr,
				info_dram_regions->region[i].size,
				MT_MEMORY | MT_RW | MT_NS);
	}
#endif
}
#endif

#if defined(PLAT_XLAT_TABLES_DYNAMIC)
void mmap_add_ddr_region_dynamically(void)
{
	int i = 0;
	dram_regions_info_t *info_dram_regions = get_dram_regions_info();
	/* MMU map for Non-Secure DRAM Regions */
	VERBOSE("DRAM Region %d: %p - %p\n", i,
			(void *) info_dram_regions->region[i].addr,
			(void *) (info_dram_regions->region[i].addr
				+ info_dram_regions->region[i].size
				- 1));
	mmap_add_dynamic_region(info_dram_regions->region[i].addr,
			info_dram_regions->region[i].addr,
			info_dram_regions->region[i].size,
			MT_MEMORY | MT_RW | MT_NS);

	/* MMU map for Secure DDR Region on DRAM-0 */
	if (info_dram_regions->region[i].size >
		(NXP_SECURE_DRAM_SIZE + NXP_SP_SHRD_DRAM_SIZE)) {
		VERBOSE("Secure DRAM Region %d: %p - %p\n", i,
			(void *) (info_dram_regions->region[i].addr
				+ info_dram_regions->region[i].size),
			(void *) (info_dram_regions->region[i].addr
				+ info_dram_regions->region[i].size
				+ NXP_SECURE_DRAM_SIZE
				+ NXP_SP_SHRD_DRAM_SIZE
				- 1));
		mmap_add_dynamic_region((info_dram_regions->region[i].addr
				+ info_dram_regions->region[i].size),
				(info_dram_regions->region[i].addr
				+ info_dram_regions->region[i].size),
				(NXP_SECURE_DRAM_SIZE + NXP_SP_SHRD_DRAM_SIZE),
				MT_MEMORY | MT_RW | MT_SECURE);
	}

#ifdef IMAGE_BL31
	for (i = 1; i < info_dram_regions->num_dram_regions; i++) {
		if (info_dram_regions->region[i].size == 0) {
			break;
		}
		VERBOSE("DRAM Region %d: %p - %p\n", i,
			(void *) info_dram_regions->region[i].addr,
			(void *) (info_dram_regions->region[i].addr
				+ info_dram_regions->region[i].size
				- 1));
		mmap_add_dynamic_region(info_dram_regions->region[i].addr,
				info_dram_regions->region[i].addr,
				info_dram_regions->region[i].size,
				MT_MEMORY | MT_RW | MT_NS);
	}
#endif
}
#endif

/*
 * Set up the page tables for the generic and platform-specific memory regions.
 * The extents of the generic memory regions are specified by the function
 * arguments and consist of:
 * - Trusted SRAM seen by the BL image;
 * - Code section;
 * - Read-only data section;
 * - Coherent memory region, if applicable.
 */
void ls_setup_page_tables(uintptr_t total_base,
			   size_t total_size,
			   uintptr_t code_start,
			   uintptr_t code_limit,
			   uintptr_t rodata_start,
			   uintptr_t rodata_limit
#if USE_COHERENT_MEM
			   ,
			   uintptr_t coh_start,
			   uintptr_t coh_limit
#endif
			   )
{
	/*
	 * Map the Trusted SRAM with appropriate memory attributes.
	 * Subsequent mappings will adjust the attributes for specific regions.
	 */
	VERBOSE("Memory seen by this BL image: %p - %p\n",
		(void *) total_base, (void *) (total_base + total_size));
	mmap_add_region(total_base, total_base,
			total_size,
			MT_MEMORY | MT_RW | MT_SECURE);

	/* Re-map the code section */
	VERBOSE("Code region: %p - %p\n",
		(void *) code_start, (void *) code_limit);
	mmap_add_region(code_start, code_start,
			code_limit - code_start,
			MT_CODE | MT_SECURE);

	/* Re-map the read-only data section */
	VERBOSE("Read-only data region: %p - %p\n",
		(void *) rodata_start, (void *) rodata_limit);
	mmap_add_region(rodata_start, rodata_start,
			rodata_limit - rodata_start,
			MT_RO_DATA | MT_SECURE);

#if USE_COHERENT_MEM
	/* Re-map the coherent memory region */
	VERBOSE("Coherent region: %p - %p\n",
		(void *) coh_start, (void *) coh_limit);
	mmap_add_region(coh_start, coh_start,
			coh_limit - coh_start,
			MT_DEVICE | MT_RW | MT_SECURE);
#endif

	/* Now (re-)map the platform-specific memory regions */
	mmap_add(plat_ls_get_mmap());


#if defined(IMAGE_BL31) || !defined(CONFIG_DDR_FIP_IMAGE)
	mmap_add_ddr_regions_statically();
#endif

	/* Create the page tables to reflect the above mappings */
	init_xlat_tables();
}

/*******************************************************************************
 * Returns NXP platform specific memory map regions.
 ******************************************************************************/
const mmap_region_t *plat_ls_get_mmap(void)
{
	return plat_ls_mmap;
}
