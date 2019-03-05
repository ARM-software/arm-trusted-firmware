/*
 * Copyright (c) 2018-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>

#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <lib/xlat_tables/xlat_tables_compat.h>
#include <plat/common/platform.h>

/*
 * The following platform functions are weakly defined. The Platforms
 * may redefine with strong definition.
 */
#pragma weak bl2_el3_plat_prepare_exit
#pragma weak plat_error_handler
#pragma weak bl2_plat_preload_setup
#pragma weak bl2_plat_handle_pre_image_load
#pragma weak bl2_plat_handle_post_image_load
#pragma weak plat_try_next_boot_source

void bl2_el3_plat_prepare_exit(void)
{
}

void __dead2 plat_error_handler(int err)
{
	while (1)
		wfi();
}

void bl2_plat_preload_setup(void)
{
}

int bl2_plat_handle_pre_image_load(unsigned int image_id)
{
	return 0;
}

int bl2_plat_handle_post_image_load(unsigned int image_id)
{
	return 0;
}

int plat_try_next_boot_source(void)
{
	return 0;
}

/*
 * Set up the page tables for the generic and platform-specific memory regions.
 * The size of the Trusted SRAM seen by the BL image must be specified as well
 * as an array specifying the generic memory regions which can be;
 * - Code section;
 * - Read-only data section;
 * - Init code section, if applicable
 * - Coherent memory region, if applicable.
 */

void __init setup_page_tables(const mmap_region_t *bl_regions,
			      const mmap_region_t *plat_regions)
{
#if LOG_LEVEL >= LOG_LEVEL_VERBOSE
	const mmap_region_t *regions = bl_regions;

	while (regions->size != 0U) {
		VERBOSE("Region: 0x%lx - 0x%lx has attributes 0x%x\n",
				regions->base_va,
				regions->base_va + regions->size,
				regions->attr);
		regions++;
	}
#endif
	/*
	 * Map the Trusted SRAM with appropriate memory attributes.
	 * Subsequent mappings will adjust the attributes for specific regions.
	 */
	mmap_add(bl_regions);

	/* Now (re-)map the platform-specific memory regions */
	mmap_add(plat_regions);

	/* Create the page tables to reflect the above mappings */
	init_xlat_tables();
}
