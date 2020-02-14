/*
 * Copyright (c) 2017-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <platform_def.h>

#include <common/debug.h>
#include <lib/xlat_tables/xlat_tables_v2.h>

#include "uniphier.h"

struct uniphier_reg_region {
	uintptr_t base;
	size_t size;
};

static const struct uniphier_reg_region uniphier_reg_region[] = {
	[UNIPHIER_SOC_LD11] = {
		.base = 0x50000000UL,
		.size = 0x20000000UL,
	},
	[UNIPHIER_SOC_LD20] = {
		.base = 0x50000000UL,
		.size = 0x20000000UL,
	},
	[UNIPHIER_SOC_PXS3] = {
		.base = 0x50000000UL,
		.size = 0x20000000UL,
	},
};

void uniphier_mmap_setup(unsigned int soc)
{
	VERBOSE("Trusted RAM seen by this BL image: %p - %p\n",
		(void *)BL_CODE_BASE, (void *)BL_END);
	mmap_add_region(BL_CODE_BASE, BL_CODE_BASE,
			round_up(BL_END, PAGE_SIZE) - BL_CODE_BASE,
			MT_MEMORY | MT_RW | MT_SECURE);

	/* remap the code section */
	VERBOSE("Code region: %p - %p\n",
		(void *)BL_CODE_BASE, (void *)BL_CODE_END);
	mmap_add_region(BL_CODE_BASE, BL_CODE_BASE,
			round_up(BL_CODE_END, PAGE_SIZE) - BL_CODE_BASE,
			MT_CODE | MT_SECURE);

	/* remap the coherent memory region */
	VERBOSE("Coherent region: %p - %p\n",
		(void *)BL_COHERENT_RAM_BASE, (void *)BL_COHERENT_RAM_END);
	mmap_add_region(BL_COHERENT_RAM_BASE, BL_COHERENT_RAM_BASE,
			BL_COHERENT_RAM_END - BL_COHERENT_RAM_BASE,
			MT_DEVICE | MT_RW | MT_SECURE);

	/* register region */
	assert(soc < ARRAY_SIZE(uniphier_reg_region));
	mmap_add_region(uniphier_reg_region[soc].base,
			uniphier_reg_region[soc].base,
			uniphier_reg_region[soc].size,
			MT_DEVICE | MT_RW | MT_SECURE);

	init_xlat_tables();
}
