/*
 * Copyright (c) 2017-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
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

#if PLAT_RO_XLAT_TABLES
/* Change the memory attributes of the descriptors which resolve the address
 * range that belongs to the translation tables themselves, which are by default
 * mapped as part of read-write data in the BL image's memory.
 *
 * Since the translation tables map themselves via these level 3 (page)
 * descriptors, any change applied to them with the MMU on would introduce a
 * chicken and egg problem because of the break-before-make sequence.
 * Eventually, it would reach the descriptor that resolves the very table it
 * belongs to and the invalidation (break step) would cause the subsequent write
 * (make step) to it to generate an MMU fault. Therefore, the MMU is disabled
 * before making the change.
 *
 * No assumption is made about what data this function needs, therefore all the
 * caches are flushed in order to ensure coherency. A future optimization would
 * be to only flush the required data to main memory.
 */
int xlat_make_tables_readonly(void)
{
	assert(tf_xlat_ctx.initialized == true);
#ifdef __aarch64__
	if (tf_xlat_ctx.xlat_regime == EL1_EL0_REGIME) {
		disable_mmu_el1();
	} else if (tf_xlat_ctx.xlat_regime == EL3_REGIME) {
		disable_mmu_el3();
	} else {
		assert(tf_xlat_ctx.xlat_regime == EL2_REGIME);
		return -1;
	}

	/* Flush all caches. */
	dcsw_op_all(DCCISW);
#else /* !__aarch64__ */
	assert(tf_xlat_ctx.xlat_regime == EL1_EL0_REGIME);
	/* On AArch32, we flush the caches before disabling the MMU. The reason
	 * for this is that the dcsw_op_all AArch32 function pushes some
	 * registers onto the stack under the assumption that it is writing to
	 * cache, which is not true with the MMU off. This would result in the
	 * stack becoming corrupted and a wrong/junk value for the LR being
	 * restored at the end of the routine.
	 */
	dcsw_op_all(DC_OP_CISW);
	disable_mmu_secure();
#endif

	int rc = xlat_change_mem_attributes_ctx(&tf_xlat_ctx,
				(uintptr_t)tf_xlat_ctx.tables,
				tf_xlat_ctx.tables_num * XLAT_TABLE_SIZE,
				MT_RO_DATA | MT_SECURE);

#ifdef __aarch64__
	if (tf_xlat_ctx.xlat_regime == EL1_EL0_REGIME) {
		enable_mmu_el1(0U);
	} else {
		assert(tf_xlat_ctx.xlat_regime == EL3_REGIME);
		enable_mmu_el3(0U);
	}
#else /* !__aarch64__ */
	enable_mmu_svc_mon(0U);
#endif

	if (rc == 0) {
		tf_xlat_ctx.readonly_tables = true;
	}

	return rc;
}
#endif /* PLAT_RO_XLAT_TABLES */

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

#ifdef __aarch64__

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

void enable_mmu(unsigned int flags)
{
	switch (get_current_el_maybe_constant()) {
	case 1:
		enable_mmu_el1(flags);
		break;
	case 2:
		enable_mmu_el2(flags);
		break;
	case 3:
		enable_mmu_el3(flags);
		break;
	default:
		panic();
	}
}

#else /* !__aarch64__ */

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

#endif /* __aarch64__ */
