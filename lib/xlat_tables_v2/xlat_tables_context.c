/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <platform_def.h>

#include <common/debug.h>
#include <lib/xlat_tables/xlat_tables_defs.h>
#include <lib/xlat_tables/xlat_tables_v2.h>

#include "xlat_tables_private.h"

/*
 * MMU configuration register values for the active translation context. Used
 * from the MMU assembly helpers.
 */
uint64_t mmu_cfg_params[MMU_CFG_PARAM_MAX];

/*
 * Allocate and initialise the default translation context for the BL image
 * currently executing.
 */
REGISTER_XLAT_CONTEXT(tf, MAX_MMAP_REGIONS, MAX_XLAT_TABLES,
		PLAT_VIRT_ADDR_SPACE_SIZE, PLAT_PHY_ADDR_SPACE_SIZE);

void mmap_add_region(unsigned long long base_pa, uintptr_t base_va, size_t size,
		     unsigned int attr)
{
	mmap_region_t mm = MAP_REGION(base_pa, base_va, size, attr);

	mmap_add_region_ctx(&tf_xlat_ctx, &mm);
}

void mmap_add(const mmap_region_t *mm)
{
	mmap_add_ctx(&tf_xlat_ctx, mm);
}

void mmap_add_region_alloc_va(unsigned long long base_pa, uintptr_t *base_va,
			      size_t size, unsigned int attr)
{
	mmap_region_t mm = MAP_REGION_ALLOC_VA(base_pa, size, attr);

	mmap_add_region_alloc_va_ctx(&tf_xlat_ctx, &mm);

	*base_va = mm.base_va;
}

void mmap_add_alloc_va(mmap_region_t *mm)
{
	while (mm->granularity != 0U) {
		assert(mm->base_va == 0U);
		mmap_add_region_alloc_va_ctx(&tf_xlat_ctx, mm);
		mm++;
	}
}

#if PLAT_XLAT_TABLES_DYNAMIC

int mmap_add_dynamic_region(unsigned long long base_pa, uintptr_t base_va,
			    size_t size, unsigned int attr)
{
	mmap_region_t mm = MAP_REGION(base_pa, base_va, size, attr);

	return mmap_add_dynamic_region_ctx(&tf_xlat_ctx, &mm);
}

int mmap_add_dynamic_region_alloc_va(unsigned long long base_pa,
				     uintptr_t *base_va, size_t size,
				     unsigned int attr)
{
	mmap_region_t mm = MAP_REGION_ALLOC_VA(base_pa, size, attr);

	int rc = mmap_add_dynamic_region_alloc_va_ctx(&tf_xlat_ctx, &mm);

	*base_va = mm.base_va;

	return rc;
}


int mmap_remove_dynamic_region(uintptr_t base_va, size_t size)
{
	return mmap_remove_dynamic_region_ctx(&tf_xlat_ctx,
					base_va, size);
}

#endif /* PLAT_XLAT_TABLES_DYNAMIC */

void __init init_xlat_tables(void)
{
	assert(tf_xlat_ctx.xlat_regime == EL_REGIME_INVALID);

	unsigned int current_el = xlat_arch_current_el();

	if (current_el == 1U) {
		tf_xlat_ctx.xlat_regime = EL1_EL0_REGIME;
	} else if (current_el == 2U) {
		tf_xlat_ctx.xlat_regime = EL2_REGIME;
	} else {
		assert(current_el == 3U);
		tf_xlat_ctx.xlat_regime = EL3_REGIME;
	}

	init_xlat_tables_ctx(&tf_xlat_ctx);
}

int xlat_get_mem_attributes(uintptr_t base_va, uint32_t *attr)
{
	return xlat_get_mem_attributes_ctx(&tf_xlat_ctx, base_va, attr);
}

int xlat_change_mem_attributes(uintptr_t base_va, size_t size, uint32_t attr)
{
	return xlat_change_mem_attributes_ctx(&tf_xlat_ctx, base_va, size, attr);
}

/*
 * If dynamic allocation of new regions is disabled then by the time we call the
 * function enabling the MMU, we'll have registered all the memory regions to
 * map for the system's lifetime. Therefore, at this point we know the maximum
 * physical address that will ever be mapped.
 *
 * If dynamic allocation is enabled then we can't make any such assumption
 * because the maximum physical address could get pushed while adding a new
 * region. Therefore, in this case we have to assume that the whole address
 * space size might be mapped.
 */
#ifdef PLAT_XLAT_TABLES_DYNAMIC
#define MAX_PHYS_ADDR	tf_xlat_ctx.pa_max_address
#else
#define MAX_PHYS_ADDR	tf_xlat_ctx.max_pa
#endif

#ifdef AARCH32

void enable_mmu_svc_mon(unsigned int flags)
{
	setup_mmu_cfg((uint64_t *)&mmu_cfg_params, flags,
		      tf_xlat_ctx.base_table, MAX_PHYS_ADDR,
		      tf_xlat_ctx.va_max_address, EL1_EL0_REGIME);
	enable_mmu_direct_svc_mon(flags);
}

void enable_mmu_hyp(unsigned int flags)
{
	setup_mmu_cfg((uint64_t *)&mmu_cfg_params, flags,
		      tf_xlat_ctx.base_table, MAX_PHYS_ADDR,
		      tf_xlat_ctx.va_max_address, EL2_REGIME);
	enable_mmu_direct_hyp(flags);
}

#else

void enable_mmu_el1(unsigned int flags)
{
	setup_mmu_cfg((uint64_t *)&mmu_cfg_params, flags,
		      tf_xlat_ctx.base_table, MAX_PHYS_ADDR,
		      tf_xlat_ctx.va_max_address, EL1_EL0_REGIME);
	enable_mmu_direct_el1(flags);
}

void enable_mmu_el2(unsigned int flags)
{
	setup_mmu_cfg((uint64_t *)&mmu_cfg_params, flags,
		      tf_xlat_ctx.base_table, MAX_PHYS_ADDR,
		      tf_xlat_ctx.va_max_address, EL2_REGIME);
	enable_mmu_direct_el2(flags);
}

void enable_mmu_el3(unsigned int flags)
{
	setup_mmu_cfg((uint64_t *)&mmu_cfg_params, flags,
		      tf_xlat_ctx.base_table, MAX_PHYS_ADDR,
		      tf_xlat_ctx.va_max_address, EL3_REGIME);
	enable_mmu_direct_el3(flags);
}

#endif /* AARCH32 */
