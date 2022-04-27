/*
 * Copyright (c) 2023, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/xlat_tables/xlat_tables_v2.h>
#include <platform_def.h>

/* Place translation tables by default along with the ones used by BL31. */
#ifndef PLAT_SP_IMAGE_XLAT_SECTION_NAME
#define PLAT_SP_IMAGE_XLAT_SECTION_NAME	".xlat_table"
#endif
#ifndef PLAT_SP_IMAGE_BASE_XLAT_SECTION_NAME
#define PLAT_SP_IMAGE_BASE_XLAT_SECTION_NAME	".bss"
#endif

/* Allocate and initialise the translation context for the secure partitions. */
REGISTER_XLAT_CONTEXT2(sp,
		       PLAT_SP_IMAGE_MMAP_REGIONS,
		       PLAT_SP_IMAGE_MAX_XLAT_TABLES,
		       PLAT_VIRT_ADDR_SPACE_SIZE, PLAT_PHY_ADDR_SPACE_SIZE,
		       EL1_EL0_REGIME, PLAT_SP_IMAGE_XLAT_SECTION_NAME,
		       PLAT_SP_IMAGE_BASE_XLAT_SECTION_NAME);

/* Get handle of Secure Partition translation context */
xlat_ctx_t *spm_get_sp_xlat_context(void)
{
	return &sp_xlat_ctx;
};
