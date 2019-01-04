/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>

#include <common/debug.h>
#include <lib/xlat_tables/xlat_tables_v2.h>

#define SQ_REG_REGION_BASE	0x20000000ULL
#define SQ_REG_REGION_SIZE	0x60000000ULL

void sq_mmap_setup(uintptr_t total_base, size_t total_size,
			 const struct mmap_region *mmap)
{
	VERBOSE("Trusted RAM seen by this BL image: %p - %p\n",
		(void *)total_base, (void *)(total_base + total_size));
	mmap_add_region(total_base, total_base,
			total_size,
			MT_NON_CACHEABLE | MT_RW | MT_SECURE);

	/* remap the code section */
	VERBOSE("Code region: %p - %p\n",
		(void *)BL_CODE_BASE, (void *)BL_CODE_END);
	mmap_add_region(BL_CODE_BASE, BL_CODE_BASE,
			round_up(BL_CODE_END, PAGE_SIZE) - BL_CODE_BASE,
			MT_NON_CACHEABLE | MT_RO | MT_SECURE);

	/* Re-map the read-only data section */
	VERBOSE("Read-only data region: %p - %p\n",
		(void *)BL_RO_DATA_BASE, (void *)BL_RO_DATA_END);
	mmap_add_region(BL_RO_DATA_BASE, BL_RO_DATA_BASE,
			round_up(BL_RO_DATA_END, PAGE_SIZE) - BL_RO_DATA_BASE,
			(MT_NON_CACHEABLE | MT_RO | MT_EXECUTE_NEVER |
			 MT_SECURE));

	/* remap the coherent memory region */
	VERBOSE("Coherent region: %p - %p\n",
		(void *)BL_COHERENT_RAM_BASE, (void *)BL_COHERENT_RAM_END);
	mmap_add_region(BL_COHERENT_RAM_BASE, BL_COHERENT_RAM_BASE,
			BL_COHERENT_RAM_END - BL_COHERENT_RAM_BASE,
			MT_DEVICE | MT_RW | MT_SECURE);

	/* register region */
	mmap_add_region(SQ_REG_REGION_BASE, SQ_REG_REGION_BASE,
			SQ_REG_REGION_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE);

	/* additional regions if needed */
	if (mmap)
		mmap_add(mmap);

	init_xlat_tables();
}
