/*
 * Copyright (c) 2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <arch_features.h>
#include <common/debug.h>
#include <lib/utils_def.h>
#include <lib/xlat_tables/xlat_tables_defs.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include "xlat_mpu_private.h"

#include <fvp_r_arch_helpers.h>
#include <platform_def.h>

#warning "xlat_mpu library is currently experimental and its API may change in future."


/* Helper function that cleans the data cache only if it is enabled. */
static inline __attribute__((unused))
	void xlat_clean_dcache_range(uintptr_t addr, size_t size)
{
	if (is_dcache_enabled()) {
		clean_dcache_range(addr, size);
	}
}



/* Calculate region-attributes byte for PRBAR part of MPU-region descriptor: */
uint64_t prbar_attr_value(uint32_t attr)
{
	uint64_t retValue = UL(0);
	uint64_t extract;  /* temp var holding bit extracted from attr */

	/* Extract and stuff SH: */
	extract = (uint64_t) ((attr >> MT_SHAREABILITY_SHIFT)
				& MT_SHAREABILITY_MASK);
	retValue |= (extract << PRBAR_SH_SHIFT);

	/* Extract and stuff AP: */
	extract = (uint64_t) ((attr >> MT_PERM_SHIFT) & MT_PERM_MASK);
	if (extract == 0U) {
		retValue |= (UL(2) << PRBAR_AP_SHIFT);
	} else /* extract == 1 */ {
		retValue |= (UL(0) << PRBAR_AP_SHIFT);
	}

	/* Extract and stuff XN: */
	extract = (uint64_t) ((attr >> MT_EXECUTE_SHIFT) & MT_EXECUTE_MASK);
	retValue |= (extract << PRBAR_XN_SHIFT);
	/* However, also don't execute in peripheral space: */
	extract = (uint64_t) ((attr >> MT_TYPE_SHIFT) & MT_TYPE_MASK);
	if (extract == 0U) {
		retValue |= (UL(1) << PRBAR_XN_SHIFT);
	}
	return retValue;
}

/* Calculate region-attributes byte for PRLAR part of MPU-region descriptor: */
uint64_t prlar_attr_value(uint32_t attr)
{
	uint64_t retValue = UL(0);
	uint64_t extract;  /* temp var holding bit extracted from attr */

	/* Extract and stuff AttrIndx: */
	extract = (uint64_t) ((attr >> MT_TYPE_SHIFT)
				& MT_TYPE_MASK);
	switch (extract) {
	case UL(0):
		retValue |= (UL(1) << PRLAR_ATTR_SHIFT);
		break;
	case UL(2):
		/* 0, so OR in nothing */
		break;
	case UL(3):
		retValue |= (UL(2) << PRLAR_ATTR_SHIFT);
		break;
	default:
		retValue |= (extract << PRLAR_ATTR_SHIFT);
		break;
	}

	/* Stuff EN: */
	retValue |= (UL(1) << PRLAR_EN_SHIFT);

	/* Force NS to 0 (Secure);  v8-R64 only supports Secure: */
	extract = ~(1U << PRLAR_NS_SHIFT);
	retValue &= extract;

	return retValue;
}

/*
 * Function that writes an MPU "translation" into the MPU registers. If not
 * possible (e.g., if no more MPU regions available) boot is aborted.
 */
static void mpu_map_region(mmap_region_t *mm)
{
	uint64_t prenr_el2_value = 0UL;
	uint64_t prbar_attrs = 0UL;
	uint64_t prlar_attrs = 0UL;
	int region_to_use = 0;

	/* If all MPU regions in use, then abort boot: */
	prenr_el2_value = read_prenr_el2();
	assert(prenr_el2_value != 0xffffffff);

	/* Find and select first-available MPU region (PRENR has an enable bit
	 * for each MPU region, 1 for in-use or 0 for unused):
	 */
	for (region_to_use = 0;  region_to_use < N_MPU_REGIONS;
	     region_to_use++) {
		if (((prenr_el2_value >> region_to_use) & 1) == 0) {
			break;
		}
	}
	write_prselr_el2((uint64_t) (region_to_use));
	isb();

	/* Set base and limit addresses: */
	write_prbar_el2(mm->base_pa & PRBAR_PRLAR_ADDR_MASK);
	write_prlar_el2((mm->base_pa + mm->size - 1UL)
			& PRBAR_PRLAR_ADDR_MASK);
	dsbsy();
	isb();

	/* Set attributes: */
	prbar_attrs = prbar_attr_value(mm->attr);
	write_prbar_el2(read_prbar_el2() | prbar_attrs);
	prlar_attrs = prlar_attr_value(mm->attr);
	write_prlar_el2(read_prlar_el2() | prlar_attrs);
	dsbsy();
	isb();

	/* Mark this MPU region as used: */
	prenr_el2_value |= (1 << region_to_use);
	write_prenr_el2(prenr_el2_value);
	isb();
}

/*
 * Function that verifies that a region can be mapped.
 * Returns:
 *        0: Success, the mapping is allowed.
 *   EINVAL: Invalid values were used as arguments.
 *   ERANGE: The memory limits were surpassed.
 *   ENOMEM: There is not enough memory in the mmap array.
 *    EPERM: Region overlaps another one in an invalid way.
 */
static int mmap_add_region_check(const xlat_ctx_t *ctx, const mmap_region_t *mm)
{
	unsigned long long base_pa = mm->base_pa;
	uintptr_t base_va = mm->base_va;
	size_t size = mm->size;

	unsigned long long end_pa = base_pa + size - 1U;
	uintptr_t end_va = base_va + size - 1U;

	if (base_pa != base_va) {
		return -EINVAL;  /* MPU does not perform address translation */
	}
	if ((base_pa % 64ULL) != 0ULL) {
		return -EINVAL;  /* MPU requires 64-byte alignment */
	}
	/* Check for overflows */
	if ((base_pa > end_pa) || (base_va > end_va)) {
		return -ERANGE;
	}
	if (end_pa > ctx->pa_max_address) {
		return -ERANGE;
	}
	/* Check that there is space in the ctx->mmap array */
	if (ctx->mmap[ctx->mmap_num - 1].size != 0U) {
		return -ENOMEM;
	}
	/* Check for PAs and VAs overlaps with all other regions */
	for (const mmap_region_t *mm_cursor = ctx->mmap;
	     mm_cursor->size != 0U; ++mm_cursor) {

		uintptr_t mm_cursor_end_va =
			mm_cursor->base_va + mm_cursor->size - 1U;

		/*
		 * Check if one of the regions is completely inside the other
		 * one.
		 */
		bool fully_overlapped_va =
			((base_va >= mm_cursor->base_va) &&
					(end_va <= mm_cursor_end_va)) ||
			((mm_cursor->base_va >= base_va) &&
						(mm_cursor_end_va <= end_va));

		/*
		 * Full VA overlaps are only allowed if both regions are
		 * identity mapped (zero offset) or have the same VA to PA
		 * offset. Also, make sure that it's not the exact same area.
		 * This can only be done with static regions.
		 */
		if (fully_overlapped_va) {

#if PLAT_XLAT_TABLES_DYNAMIC
			if (((mm->attr & MT_DYNAMIC) != 0U) ||
			    ((mm_cursor->attr & MT_DYNAMIC) != 0U)) {
				return -EPERM;
			}
#endif /* PLAT_XLAT_TABLES_DYNAMIC */
			if ((mm_cursor->base_va - mm_cursor->base_pa)
					!= (base_va - base_pa)) {
				return -EPERM;
			}
			if ((base_va == mm_cursor->base_va) &&
					(size == mm_cursor->size)) {
				return -EPERM;
			}
		} else {
			/*
			 * If the regions do not have fully overlapping VAs,
			 * then they must have fully separated VAs and PAs.
			 * Partial overlaps are not allowed
			 */

			unsigned long long mm_cursor_end_pa =
				     mm_cursor->base_pa + mm_cursor->size - 1U;

			bool separated_pa = (end_pa < mm_cursor->base_pa) ||
				(base_pa > mm_cursor_end_pa);
			bool separated_va = (end_va < mm_cursor->base_va) ||
				(base_va > mm_cursor_end_va);

			if (!separated_va || !separated_pa) {
				return -EPERM;
			}
		}
	}

	return 0;
}

void mmap_add_region_ctx(xlat_ctx_t *ctx, const mmap_region_t *mm)
{
	mmap_region_t *mm_cursor = ctx->mmap, *mm_destination;
	const mmap_region_t *mm_end = ctx->mmap + ctx->mmap_num;
	const mmap_region_t *mm_last;
	unsigned long long end_pa = mm->base_pa + mm->size - 1U;
	uintptr_t end_va = mm->base_va + mm->size - 1U;
	int ret;

	/* Ignore empty regions */
	if (mm->size == 0U) {
		return;
	}

	/* Static regions must be added before initializing the xlat tables. */
	assert(!ctx->initialized);

	ret = mmap_add_region_check(ctx, mm);
	if (ret != 0) {
		ERROR("mmap_add_region_check() failed. error %d\n", ret);
		assert(false);
		return;
	}

	/*
	 * Find the last entry marker in the mmap
	 */
	mm_last = ctx->mmap;
	while ((mm_last->size != 0U) && (mm_last < mm_end)) {
		++mm_last;
	}

	/*
	 * Check if we have enough space in the memory mapping table.
	 * This shouldn't happen as we have checked in mmap_add_region_check
	 * that there is free space.
	 */
	assert(mm_last->size == 0U);

	/* Make room for new region by moving other regions up by one place */
	mm_destination = mm_cursor + 1;
	(void)memmove(mm_destination, mm_cursor,
		(uintptr_t)mm_last - (uintptr_t)mm_cursor);

	/*
	 * Check we haven't lost the empty sentinel from the end of the array.
	 * This shouldn't happen as we have checked in mmap_add_region_check
	 * that there is free space.
	 */
	assert(mm_end->size == 0U);

	*mm_cursor = *mm;

	if (end_pa > ctx->max_pa) {
		ctx->max_pa = end_pa;
	}
	if (end_va > ctx->max_va) {
		ctx->max_va = end_va;
	}
}

void mmap_add_ctx(xlat_ctx_t *ctx, const mmap_region_t *mm)
{
	const mmap_region_t *mm_cursor = mm;

	while (mm_cursor->granularity != 0U) {
		mmap_add_region_ctx(ctx, mm_cursor);
		mm_cursor++;
	}
}

void __init init_xlat_tables_ctx(xlat_ctx_t *ctx)
{
	uint64_t mair = UL(0);

	assert(ctx != NULL);
	assert(!ctx->initialized);
	assert((ctx->xlat_regime == EL2_REGIME) ||
		(ctx->xlat_regime == EL1_EL0_REGIME));
	/* Note:  Add EL3_REGIME if EL3 is supported in future v8-R64 cores. */
	assert(!is_mpu_enabled_ctx(ctx));

	mmap_region_t *mm = ctx->mmap;

	assert(ctx->va_max_address >=
		(xlat_get_min_virt_addr_space_size() - 1U));
	assert(ctx->va_max_address <= (MAX_VIRT_ADDR_SPACE_SIZE - 1U));
	assert(IS_POWER_OF_TWO(ctx->va_max_address + 1U));

	xlat_mmap_print(mm);

	/* All tables must be zeroed before mapping any region. */

	for (unsigned int i = 0U; i < ctx->base_table_entries; i++)
		ctx->base_table[i] = INVALID_DESC;

	/* Also mark all MPU regions as invalid in the MPU hardware itself: */
	write_prenr_el2(0);
		/* Sufficient for current, max-32-region implementations. */
	dsbsy();
	isb();
	while (mm->size != 0U) {
		if (read_prenr_el2() == ALL_MPU_EL2_REGIONS_USED) {
			ERROR("Not enough MPU regions to map region:\n"
				" VA:0x%lx  PA:0x%llx  size:0x%zx  attr:0x%x\n",
				mm->base_va, mm->base_pa, mm->size, mm->attr);
			panic();
		} else {
#if !(HW_ASSISTED_COHERENCY || WARMBOOT_ENABLE_DCACHE_EARLY)
			xlat_clean_dcache_range((uintptr_t)mm->base_va,
				mm->size);
#endif
			mpu_map_region(mm);
		}
		mm++;
	}

	ctx->initialized = true;

	xlat_tables_print(ctx);

	/* Set attributes in the right indices of the MAIR */
	mair = MAIR_ATTR_SET(ATTR_DEVICE, ATTR_DEVICE_INDEX);
	mair |= MAIR_ATTR_SET(ATTR_IWBWA_OWBWA_NTR,
			ATTR_IWBWA_OWBWA_NTR_INDEX);
	mair |= MAIR_ATTR_SET(ATTR_NON_CACHEABLE,
			ATTR_NON_CACHEABLE_INDEX);
	write_mair_el2(mair);
	dsbsy();
	isb();
}

/*
 * Function to wipe clean and disable all MPU regions.  This function expects
 * that the MPU has already been turned off, and caching concerns addressed,
 * but it nevertheless also explicitly turns off the MPU.
 */
void clear_all_mpu_regions(void)
{
	uint64_t sctlr_el2_value = 0UL;
	uint64_t region_n = 0UL;

	/*
	 * MPU should already be disabled, but explicitly disable it
	 * nevertheless:
	 */
	sctlr_el2_value = read_sctlr_el2() & ~(1UL);
	write_sctlr_el2(sctlr_el2_value);

	/* Disable all regions: */
	write_prenr_el2(0UL);

	/* Sequence through all regions, zeroing them out and turning off: */
	for (region_n = 0UL;  region_n < N_MPU_REGIONS;  region_n++) {
		write_prselr_el2(region_n);
		isb();
		write_prbar_el2((uint64_t) 0);
		write_prlar_el2((uint64_t) 0);
		dsbsy();
		isb();
	}
}
