/*
 * Copyright (c) 2022, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <mtk_mmap_pool.h>
#include <platform_def.h>

static const mmap_region_t mcusys_mmap[] MTK_MMAP_SECTION = {
	MAP_REGION_FLAT(MCUCFG_BASE, MCUCFG_REG_SIZE, MT_DEVICE | MT_RW | MT_SECURE),
	{0}
};
DECLARE_MTK_MMAP_REGIONS(mcusys_mmap);
