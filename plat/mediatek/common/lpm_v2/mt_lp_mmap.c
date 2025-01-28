/*
 * Copyright (c) 2025, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>

#include <mtk_mmap_pool.h>

static const mmap_region_t lpm_sram_mmap[] MTK_MMAP_SECTION = {
#if (MTK_LPM_SRAM_BASE && MTK_LPM_SRAM_MAP_SIZE)
	/* LPM used syssram */
	MAP_REGION_FLAT(MTK_LPM_SRAM_BASE, MTK_LPM_SRAM_MAP_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
#endif
	{0}
};
DECLARE_MTK_MMAP_REGIONS(lpm_sram_mmap);
