/*
 * Copyright (c) 2022, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <mtk_mmap_pool.h>
#include <platform_def.h>

#include <tegrabl_cpubl_params.h>
#include <tegrabl_hv_info.h>

static const mmap_region_t plat_mmap[] = {
	MAP_REGION_FLAT(MTK_DEV_RNG0_BASE, MTK_DEV_RNG0_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(MTK_DEV_RNG1_BASE, MTK_DEV_RNG1_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(CPUBL_PARAMS_BASE_PHY, CPUBL_PARAMS_BASE_SIZE,
			MT_MEMORY | MT_RW | MT_NS),
	MAP_REGION_FLAT(HV_INFO_BASE_PHY, HV_INFO_SIZE,
			MT_MEMORY | MT_RW | MT_NS),
	{ 0 }
};
DECLARE_MTK_MMAP_REGIONS(plat_mmap);
