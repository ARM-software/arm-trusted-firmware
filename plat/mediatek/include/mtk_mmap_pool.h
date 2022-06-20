/*
 * Copyright (c) 2022, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MTK_MMAP_POOL_H
#define MTK_MMAP_POOL_H

#include <cdefs.h>
#include <lib/xlat_tables/xlat_tables_compat.h>

struct mtk_mmap_descriptor {
	const char *mmap_name;
	const mmap_region_t *mmap_ptr;
	const uint32_t mmap_size;
};

#define MTK_MMAP_SECTION \
	__used \
	__aligned(sizeof(void *)) \
	__section(".mtk_mmap_lists")

#define DECLARE_MTK_MMAP_REGIONS(_mmap_array) \
	static const struct mtk_mmap_descriptor _mtk_mmap_descriptor_##_mmap_array \
	__used \
	__aligned(sizeof(void *)) \
	__section(".mtk_mmap_pool") \
	= { \
		.mmap_name = #_mmap_array, \
		.mmap_ptr = _mmap_array, \
		.mmap_size = ARRAY_SIZE(_mmap_array) \
	}

#define MAP_BL_RW MAP_REGION_FLAT( \
		DATA_START, \
		BL_END - DATA_START, \
		MT_MEMORY | MT_RW | MT_SECURE)

#if SEPARATE_CODE_AND_RODATA
#define MAP_BL_RO \
	MAP_REGION_FLAT( \
		BL_CODE_BASE, \
		BL_CODE_END - BL_CODE_BASE, \
		MT_CODE | MT_SECURE), \
	MAP_REGION_FLAT( \
		BL_RO_DATA_BASE, \
		BL_RO_DATA_END - BL_RO_DATA_BASE, \
		MT_RO_DATA | MT_SECURE)
#else
#define MAP_BL_RO MAP_REGION_FLAT(BL_CODE_BASE, \
				  BL_CODE_END - BL_CODE_BASE,	\
				  MT_CODE | MT_SECURE)
#endif

void mtk_xlat_init(const mmap_region_t *bl_regions);

#endif /* MTK_MMAP_POOL_H */
