/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_helpers.h>
#include <assert.h>
#include <cassert.h>
#include <common_def.h>
#include <debug.h>
#include <errno.h>
#include <platform_def.h>
#include <string.h>
#include <types.h>
#include <utils.h>
#include <xlat_tables_v2.h>
#ifdef AARCH32
# include "aarch32/xlat_tables_arch.h"
#else
# include "aarch64/xlat_tables_arch.h"
#endif
#include "xlat_tables_private.h"

/*
 * Private variables used by the TF
 */
static mmap_region_t tf_mmap[MAX_MMAP_REGIONS + 1];

static uint64_t tf_xlat_tables[MAX_XLAT_TABLES][XLAT_TABLE_ENTRIES]
			__aligned(XLAT_TABLE_SIZE) __section("xlat_table");

static uint64_t tf_base_xlat_table[NUM_BASE_LEVEL_ENTRIES]
		__aligned(NUM_BASE_LEVEL_ENTRIES * sizeof(uint64_t));

#if PLAT_XLAT_TABLES_DYNAMIC
static int xlat_tables_mapped_regions[MAX_XLAT_TABLES];
#endif /* PLAT_XLAT_TABLES_DYNAMIC */

xlat_ctx_t tf_xlat_ctx = {

	.pa_max_address = PLAT_PHY_ADDR_SPACE_SIZE - 1,
	.va_max_address = PLAT_VIRT_ADDR_SPACE_SIZE - 1,

	.mmap = tf_mmap,
	.mmap_num = MAX_MMAP_REGIONS,

	.tables = tf_xlat_tables,
	.tables_num = MAX_XLAT_TABLES,
#if PLAT_XLAT_TABLES_DYNAMIC
	.tables_mapped_regions = xlat_tables_mapped_regions,
#endif /* PLAT_XLAT_TABLES_DYNAMIC */

	.base_table = tf_base_xlat_table,
	.base_table_entries = NUM_BASE_LEVEL_ENTRIES,

	.max_pa = 0,
	.max_va = 0,

	.next_table = 0,

	.base_level = XLAT_TABLE_LEVEL_BASE,

	.initialized = 0
};

void mmap_add_region(unsigned long long base_pa, uintptr_t base_va,
			size_t size, mmap_attr_t attr)
{
	mmap_region_t mm = {
		.base_va = base_va,
		.base_pa = base_pa,
		.size = size,
		.attr = attr,
	};
	mmap_add_region_ctx(&tf_xlat_ctx, (mmap_region_t *)&mm);
}

void mmap_add(const mmap_region_t *mm)
{
	while (mm->size) {
		mmap_add_region_ctx(&tf_xlat_ctx, (mmap_region_t *)mm);
		mm++;
	}
}

#if PLAT_XLAT_TABLES_DYNAMIC

int mmap_add_dynamic_region(unsigned long long base_pa,
			    uintptr_t base_va, size_t size, mmap_attr_t attr)
{
	mmap_region_t mm = {
		.base_va = base_va,
		.base_pa = base_pa,
		.size = size,
		.attr = attr,
	};
	return mmap_add_dynamic_region_ctx(&tf_xlat_ctx, &mm);
}

int mmap_remove_dynamic_region(uintptr_t base_va, size_t size)
{
	return mmap_remove_dynamic_region_ctx(&tf_xlat_ctx, base_va, size);
}

#endif /* PLAT_XLAT_TABLES_DYNAMIC */

void init_xlat_tables(void)
{
	assert(!is_mmu_enabled());
	assert(!tf_xlat_ctx.initialized);
	print_mmap(tf_xlat_ctx.mmap);
	tf_xlat_ctx.execute_never_mask =
			xlat_arch_get_xn_desc(xlat_arch_current_el());
	init_xlation_table(&tf_xlat_ctx);
	xlat_tables_print(&tf_xlat_ctx);

	assert(tf_xlat_ctx.max_va <= PLAT_VIRT_ADDR_SPACE_SIZE - 1);
	assert(tf_xlat_ctx.max_pa <= PLAT_PHY_ADDR_SPACE_SIZE - 1);

	init_xlat_tables_arch(tf_xlat_ctx.max_pa);
}

#ifdef AARCH32

void enable_mmu_secure(unsigned int flags)
{
	enable_mmu_arch(flags, tf_xlat_ctx.base_table);
}

#else

void enable_mmu_el1(unsigned int flags)
{
	enable_mmu_arch(flags, tf_xlat_ctx.base_table);
}

void enable_mmu_el3(unsigned int flags)
{
	enable_mmu_arch(flags, tf_xlat_ctx.base_table);
}

#endif /* AARCH32 */
