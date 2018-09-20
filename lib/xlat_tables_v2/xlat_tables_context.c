/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <debug.h>
#include <platform_def.h>
#include <xlat_tables_defs.h>
#include <xlat_tables_v2.h>

#include "xlat_tables_private.h"

/*
 * MMU configuration register values for the active translation context. Used
 * from the MMU assembly helpers.
 */
uint64_t mmu_cfg_params[MMU_CFG_PARAM_MAX];

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

#if PLAT_XLAT_TABLES_DYNAMIC

int mmap_add_dynamic_region(unsigned long long base_pa, uintptr_t base_va,
			    size_t size, unsigned int attr)
{
	mmap_region_t mm = MAP_REGION(base_pa, base_va, size, attr);

	return mmap_add_dynamic_region_ctx(&tf_xlat_ctx, &mm);
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

#if !ERROR_DEPRECATED
void enable_mmu_secure(unsigned int flags)
{
	enable_mmu_svc_mon(flags);
}

void enable_mmu_direct(unsigned int flags)
{
	enable_mmu_direct_svc_mon(flags);
}
#endif

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
