/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_helpers.h>
#include <assert.h>
#include <common_def.h>
#include <debug.h>
#include <errno.h>
#include <platform_def.h>
#include <string.h>
#include <types.h>
#include <utils.h>
#include <xlat_tables_arch.h>
#include <xlat_tables_v2.h>

#include "xlat_tables_private.h"

/*
 * Each platform can define the size of its physical and virtual address spaces.
 * If the platform hasn't defined one or both of them, default to
 * ADDR_SPACE_SIZE. The latter is deprecated, though.
 */
#if ERROR_DEPRECATED
# ifdef ADDR_SPACE_SIZE
#  error "ADDR_SPACE_SIZE is deprecated. Use PLAT_xxx_ADDR_SPACE_SIZE instead."
# endif
#elif defined(ADDR_SPACE_SIZE)
# ifndef PLAT_PHY_ADDR_SPACE_SIZE
#  define PLAT_PHY_ADDR_SPACE_SIZE	ADDR_SPACE_SIZE
# endif
# ifndef PLAT_VIRT_ADDR_SPACE_SIZE
#  define PLAT_VIRT_ADDR_SPACE_SIZE	ADDR_SPACE_SIZE
# endif
#endif

/*
 * Allocate and initialise the default translation context for the BL image
 * currently executing.
 */
REGISTER_XLAT_CONTEXT(tf, MAX_MMAP_REGIONS, MAX_XLAT_TABLES,
		PLAT_VIRT_ADDR_SPACE_SIZE, PLAT_PHY_ADDR_SPACE_SIZE);

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

	assert(tf_xlat_ctx.max_va <= tf_xlat_ctx.va_max_address);
	assert(tf_xlat_ctx.max_pa <= tf_xlat_ctx.pa_max_address);

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
