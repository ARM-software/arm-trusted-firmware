/*
 * Copyright (c) 2024, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <platform_def.h>

#include <mtk_mmap_pool.h>

static const mmap_region_t plat_mmap[] = {
	MAP_REGION_FLAT(MT_GIC_BASE, MTK_GIC_REG_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(MTK_DEV_RNG1_BASE, MTK_DEV_RNG1_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(DP_SEC_BASE, DP_SEC_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(EDP_SEC_BASE, EDP_SEC_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(APU_MBOX0, APU_MBOX0_SZ,
			MT_DEVICE | MT_RW | MT_SECURE),
	{ 0 }
};
DECLARE_MTK_MMAP_REGIONS(plat_mmap);
