/*
 * Copyright (c) 2022, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <drivers/console.h>
#include <lib/xlat_tables/xlat_tables_compat.h>
#include <mtk_mmap_pool.h>

IMPORT_SYM(uintptr_t, __MTK_MMAP_POINTER_POOL_START__, MTK_MMAP_POINTER_POOL_START);
IMPORT_SYM(uintptr_t, __MTK_MMAP_POINTER_POOL_END_UNALIGNED__, MTK_MMAP_POINTER_POOL_END_UNALIGNED);
IMPORT_SYM(uintptr_t, __RW_START__, RW_START);
IMPORT_SYM(uintptr_t, __DATA_START__, DATA_START);

#define MAP_MTK_SECTIONS MAP_REGION_FLAT(RW_START, \
					 DATA_START - RW_START, \
					 MT_MEMORY | MT_RO | MT_SECURE)


static void print_mmap(const mmap_region_t *regions)
{
	while (regions->size != 0U) {
		VERBOSE("Region: 0x%lx - 0x%lx has attributes 0x%x\n",
			regions->base_va,
			regions->base_va + regions->size,
			regions->attr);
		regions++;
	}
}

void mtk_xlat_init(const mmap_region_t *bl_regions)
{
	struct mtk_mmap_descriptor *iter;
	const mmap_region_t *regions = bl_regions;

	print_mmap(regions);
	mmap_add(bl_regions);
	if (MTK_MMAP_POINTER_POOL_START != MTK_MMAP_POINTER_POOL_END_UNALIGNED) {
		for (iter = (struct mtk_mmap_descriptor *)MTK_MMAP_POINTER_POOL_START;
		     (char *)iter < (char *)MTK_MMAP_POINTER_POOL_END_UNALIGNED;
		     iter++) {
			regions = iter->mmap_ptr;
			INFO("mmap_name: %s\n", iter->mmap_name);
			INFO("mmap_size: 0x%x\n", iter->mmap_size);
			print_mmap(regions);
			mmap_add(regions);
		}
	}
	init_xlat_tables();
	enable_mmu_el3(0);
}
