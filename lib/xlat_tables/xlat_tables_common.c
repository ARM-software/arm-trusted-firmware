/*
 * Copyright (c) 2016-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_helpers.h>
#include <assert.h>
#include <cassert.h>
#include <common_def.h>
#include <debug.h>
#include <platform_def.h>
#include <string.h>
#include <types.h>
#include <utils.h>
#include <xlat_tables.h>
#include "xlat_tables_private.h"

#if LOG_LEVEL >= LOG_LEVEL_VERBOSE
#define LVL0_SPACER ""
#define LVL1_SPACER "  "
#define LVL2_SPACER "    "
#define LVL3_SPACER "      "
#define get_level_spacer(level)		\
			(((level) == U(0)) ? LVL0_SPACER : \
			(((level) == U(1)) ? LVL1_SPACER : \
			(((level) == U(2)) ? LVL2_SPACER : LVL3_SPACER)))
#define debug_print(...) tf_printf(__VA_ARGS__)
#else
#define debug_print(...) ((void)0)
#endif

#define UNSET_DESC	~0ull

#ifdef PLAT_XLAT_BASE
#define XLAT_TABLE_REQUIRED_ENTRIES	(XLAT_TABLE_ENTRIES * MAX_XLAT_TABLES)
CASSERT(PLAT_XLAT_SIZE == sizeof(uint64_t) * XLAT_TABLE_REQUIRED_ENTRIES,
	invalid_plat_xlat_size);
CASSERT(!(PLAT_XLAT_BASE & (XLAT_TABLE_SIZE - 1)), invalid_plat_xlat_base);
#define xlat_tables	((uint64_t (*)[XLAT_TABLE_ENTRIES]) \
				(unsigned long *)PLAT_XLAT_BASE)
#else
static uint64_t xlat_tables[MAX_XLAT_TABLES][XLAT_TABLE_ENTRIES]
			__aligned(XLAT_TABLE_SIZE) __section("xlat_table");
#endif

static unsigned int next_xlat;
static unsigned long long xlat_max_pa;
static uintptr_t xlat_max_va;

static uint64_t execute_never_mask;

/*
 * Array of all memory regions stored in order of ascending base address.
 * The list is terminated by the first entry with size == 0.
 */
static mmap_region_t mmap[MAX_MMAP_REGIONS + 1];


void print_mmap(void)
{
#if LOG_LEVEL >= LOG_LEVEL_VERBOSE
	debug_print("mmap:\n");
	mmap_region_t *mm = mmap;
	while (mm->size) {
		debug_print(" VA:%p  PA:0x%llx  size:0x%zx  attr:0x%x\n",
				(void *)mm->base_va, mm->base_pa,
				mm->size, mm->attr);
		++mm;
	};
	debug_print("\n");
#endif
}

void mmap_add_region(unsigned long long base_pa, uintptr_t base_va,
			size_t size, mmap_attr_t attr)
{
	mmap_region_t *mm = mmap;
	mmap_region_t *mm_last = mm + ARRAY_SIZE(mmap) - 1;
	unsigned long long end_pa = base_pa + size - 1;
	uintptr_t end_va = base_va + size - 1;

	assert(IS_PAGE_ALIGNED(base_pa));
	assert(IS_PAGE_ALIGNED(base_va));
	assert(IS_PAGE_ALIGNED(size));

	if (!size)
		return;

	assert(base_pa < end_pa); /* Check for overflows */
	assert(base_va < end_va);

	assert((base_va + (uintptr_t)size - (uintptr_t)1) <=
					(PLAT_VIRT_ADDR_SPACE_SIZE - 1));
	assert((base_pa + (unsigned long long)size - 1ULL) <=
					(PLAT_PHY_ADDR_SPACE_SIZE - 1));

#if ENABLE_ASSERTIONS

	/* Check for PAs and VAs overlaps with all other regions */
	for (mm = mmap; mm->size; ++mm) {

		uintptr_t mm_end_va = mm->base_va + mm->size - 1;

		/*
		 * Check if one of the regions is completely inside the other
		 * one.
		 */
		int fully_overlapped_va =
			((base_va >= mm->base_va) && (end_va <= mm_end_va)) ||
			((mm->base_va >= base_va) && (mm_end_va <= end_va));

		/*
		 * Full VA overlaps are only allowed if both regions are
		 * identity mapped (zero offset) or have the same VA to PA
		 * offset. Also, make sure that it's not the exact same area.
		 */
		if (fully_overlapped_va) {
			assert((mm->base_va - mm->base_pa) ==
			       (base_va - base_pa));
			assert((base_va != mm->base_va) || (size != mm->size));
		} else {
			/*
			 * If the regions do not have fully overlapping VAs,
			 * then they must have fully separated VAs and PAs.
			 * Partial overlaps are not allowed
			 */

			unsigned long long mm_end_pa =
						     mm->base_pa + mm->size - 1;

			int separated_pa =
				(end_pa < mm->base_pa) || (base_pa > mm_end_pa);
			int separated_va =
				(end_va < mm->base_va) || (base_va > mm_end_va);

			assert(separated_va && separated_pa);
		}
	}

	mm = mmap; /* Restore pointer to the start of the array */

#endif /* ENABLE_ASSERTIONS */

	/* Find correct place in mmap to insert new region */
	while (mm->base_va < base_va && mm->size)
		++mm;

	/*
	 * If a section is contained inside another one with the same base
	 * address, it must be placed after the one it is contained in:
	 *
	 * 1st |-----------------------|
	 * 2nd |------------|
	 * 3rd |------|
	 *
	 * This is required for mmap_region_attr() to get the attributes of the
	 * small region correctly.
	 */
	while ((mm->base_va == base_va) && (mm->size > size))
		++mm;

	/* Make room for new region by moving other regions up by one place */
	memmove(mm + 1, mm, (uintptr_t)mm_last - (uintptr_t)mm);

	/* Check we haven't lost the empty sentinal from the end of the array */
	assert(mm_last->size == 0);

	mm->base_pa = base_pa;
	mm->base_va = base_va;
	mm->size = size;
	mm->attr = attr;

	if (end_pa > xlat_max_pa)
		xlat_max_pa = end_pa;
	if (end_va > xlat_max_va)
		xlat_max_va = end_va;
}

void mmap_add(const mmap_region_t *mm)
{
	while (mm->size) {
		mmap_add_region(mm->base_pa, mm->base_va, mm->size, mm->attr);
		++mm;
	}
}

static uint64_t mmap_desc(mmap_attr_t attr, unsigned long long addr_pa,
							unsigned int level)
{
	uint64_t desc;
	int mem_type;

	/* Make sure that the granularity is fine enough to map this address. */
	assert((addr_pa & XLAT_BLOCK_MASK(level)) == 0);

	desc = addr_pa;
	/*
	 * There are different translation table descriptors for level 3 and the
	 * rest.
	 */
	desc |= (level == XLAT_TABLE_LEVEL_MAX) ? PAGE_DESC : BLOCK_DESC;
	desc |= (attr & MT_NS) ? LOWER_ATTRS(NS) : 0;
	desc |= (attr & MT_RW) ? LOWER_ATTRS(AP_RW) : LOWER_ATTRS(AP_RO);
	desc |= LOWER_ATTRS(ACCESS_FLAG);

	/*
	 * Deduce shareability domain and executability of the memory region
	 * from the memory type.
	 *
	 * Data accesses to device memory and non-cacheable normal memory are
	 * coherent for all observers in the system, and correspondingly are
	 * always treated as being Outer Shareable. Therefore, for these 2 types
	 * of memory, it is not strictly needed to set the shareability field
	 * in the translation tables.
	 */
	mem_type = MT_TYPE(attr);
	if (mem_type == MT_DEVICE) {
		desc |= LOWER_ATTRS(ATTR_DEVICE_INDEX | OSH);
		/*
		 * Always map device memory as execute-never.
		 * This is to avoid the possibility of a speculative instruction
		 * fetch, which could be an issue if this memory region
		 * corresponds to a read-sensitive peripheral.
		 */
		desc |= execute_never_mask;

	} else { /* Normal memory */
		/*
		 * Always map read-write normal memory as execute-never.
		 * (Trusted Firmware doesn't self-modify its code, therefore
		 * R/W memory is reserved for data storage, which must not be
		 * executable.)
		 * Note that setting the XN bit here is for consistency only.
		 * The function that enables the MMU sets the SCTLR_ELx.WXN bit,
		 * which makes any writable memory region to be treated as
		 * execute-never, regardless of the value of the XN bit in the
		 * translation table.
		 *
		 * For read-only memory, rely on the MT_EXECUTE/MT_EXECUTE_NEVER
		 * attribute to figure out the value of the XN bit.
		 */
		if ((attr & MT_RW) || (attr & MT_EXECUTE_NEVER)) {
			desc |= execute_never_mask;
		}

		if (mem_type == MT_MEMORY) {
			desc |= LOWER_ATTRS(ATTR_IWBWA_OWBWA_NTR_INDEX | ISH);
		} else {
			assert(mem_type == MT_NON_CACHEABLE);
			desc |= LOWER_ATTRS(ATTR_NON_CACHEABLE_INDEX | OSH);
		}
	}

	debug_print((mem_type == MT_MEMORY) ? "MEM" :
		((mem_type == MT_NON_CACHEABLE) ? "NC" : "DEV"));
	debug_print(attr & MT_RW ? "-RW" : "-RO");
	debug_print(attr & MT_NS ? "-NS" : "-S");
	debug_print(attr & MT_EXECUTE_NEVER ? "-XN" : "-EXEC");
	return desc;
}

/*
 * Look for the innermost region that contains the area at `base_va` with size
 * `size`. Populate *attr with the attributes of this region.
 *
 * On success, this function returns 0.
 * If there are partial overlaps (meaning that a smaller size is needed) or if
 * the region can't be found in the given area, it returns -1. In this case the
 * value pointed by attr should be ignored by the caller.
 */
static int mmap_region_attr(mmap_region_t *mm, uintptr_t base_va,
					size_t size, mmap_attr_t *attr)
{
	/* Don't assume that the area is contained in the first region */
	int ret = -1;

	/*
	 * Get attributes from last (innermost) region that contains the
	 * requested area. Don't stop as soon as one region doesn't contain it
	 * because there may be other internal regions that contain this area:
	 *
	 * |-----------------------------1-----------------------------|
	 * |----2----|     |-------3-------|    |----5----|
	 *                   |--4--|
	 *
	 *                   |---| <- Area we want the attributes of.
	 *
	 * In this example, the area is contained in regions 1, 3 and 4 but not
	 * in region 2. The loop shouldn't stop at region 2 as inner regions
	 * have priority over outer regions, it should stop at region 5.
	 */
	for (;; ++mm) {

		if (!mm->size)
			return ret; /* Reached end of list */

		if (mm->base_va > base_va + size - 1)
			return ret; /* Next region is after area so end */

		if (mm->base_va + mm->size - 1 < base_va)
			continue; /* Next region has already been overtaken */

		if (!ret && mm->attr == *attr)
			continue; /* Region doesn't override attribs so skip */

		if (mm->base_va > base_va ||
			mm->base_va + mm->size - 1 < base_va + size - 1)
			return -1; /* Region doesn't fully cover our area */

		*attr = mm->attr;
		ret = 0;
	}
	return ret;
}

static mmap_region_t *init_xlation_table_inner(mmap_region_t *mm,
					uintptr_t base_va,
					uint64_t *table,
					unsigned int level)
{
	assert(level >= XLAT_TABLE_LEVEL_MIN && level <= XLAT_TABLE_LEVEL_MAX);

	unsigned int level_size_shift =
		       L0_XLAT_ADDRESS_SHIFT - level * XLAT_TABLE_ENTRIES_SHIFT;
	u_register_t level_size = (u_register_t)1 << level_size_shift;
	u_register_t level_index_mask =
		((u_register_t)XLAT_TABLE_ENTRIES_MASK) << level_size_shift;

	debug_print("New xlat table:\n");

	do  {
		uint64_t desc = UNSET_DESC;

		if (!mm->size) {
			/* Done mapping regions; finish zeroing the table */
			desc = INVALID_DESC;
		} else if (mm->base_va + mm->size - 1 < base_va) {
			/* This area is after the region so get next region */
			++mm;
			continue;
		}

		debug_print("%s VA:%p size:0x%llx ", get_level_spacer(level),
			(void *)base_va, (unsigned long long)level_size);

		if (mm->base_va > base_va + level_size - 1) {
			/* Next region is after this area. Nothing to map yet */
			desc = INVALID_DESC;
		/* Make sure that the current level allows block descriptors */
		} else if (level >= XLAT_BLOCK_LEVEL_MIN) {
			/*
			 * Try to get attributes of this area. It will fail if
			 * there are partially overlapping regions. On success,
			 * it will return the innermost region's attributes.
			 */
			mmap_attr_t attr;
			int r = mmap_region_attr(mm, base_va, level_size, &attr);

			if (!r) {
				desc = mmap_desc(attr,
					base_va - mm->base_va + mm->base_pa,
					level);
			}
		}

		if (desc == UNSET_DESC) {
			/* Area not covered by a region so need finer table */
			uint64_t *new_table = xlat_tables[next_xlat++];
			assert(next_xlat <= MAX_XLAT_TABLES);
			desc = TABLE_DESC | (uintptr_t)new_table;

			/* Recurse to fill in new table */
			mm = init_xlation_table_inner(mm, base_va,
						new_table, level+1);
		}

		debug_print("\n");

		*table++ = desc;
		base_va += level_size;
	} while ((base_va & level_index_mask) &&
		 (base_va - 1 < PLAT_VIRT_ADDR_SPACE_SIZE - 1));

	return mm;
}

void init_xlation_table(uintptr_t base_va, uint64_t *table,
			unsigned int level, uintptr_t *max_va,
			unsigned long long *max_pa)
{
	execute_never_mask = xlat_arch_get_xn_desc(xlat_arch_current_el());

#ifdef PLAT_BASE_XLAT_BASE
	inv_dcache_range(PLAT_BASE_XLAT_BASE, PLAT_BASE_XLAT_SIZE);
	zeromem((void *)PLAT_BASE_XLAT_BASE, PLAT_BASE_XLAT_SIZE);
#endif
#ifdef PLAT_XLAT_BASE
	inv_dcache_range(PLAT_XLAT_BASE, PLAT_XLAT_SIZE);
	zeromem((void *)PLAT_XLAT_BASE, PLAT_XLAT_SIZE);
#endif

	init_xlation_table_inner(mmap, base_va, table, level);
	*max_va = xlat_max_va;
	*max_pa = xlat_max_pa;
}
