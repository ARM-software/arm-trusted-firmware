/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <arch.h>
#include <arch_helpers.h>
#include <assert.h>
#include <bl_common.h>
#include <cassert.h>
#include <debug.h>
#include <platform_def.h>
#include <string.h>
#include <xlat_tables.h>

#if LOG_LEVEL >= LOG_LEVEL_VERBOSE
#define LVL0_SPACER ""
#define LVL1_SPACER "  "
#define LVL2_SPACER "    "
#define LVL3_SPACER "      "
#define get_level_spacer(level)		\
			(((level) == 0) ? LVL0_SPACER : \
			(((level) == 1) ? LVL1_SPACER : \
			(((level) == 2) ? LVL2_SPACER : LVL3_SPACER)))
#define debug_print(...) tf_printf(__VA_ARGS__)
#else
#define debug_print(...) ((void)0)
#endif

#define UNSET_DESC	~0ul

static uint64_t xlat_tables[MAX_XLAT_TABLES][XLAT_TABLE_ENTRIES]
			__aligned(XLAT_TABLE_SIZE) __section("xlat_table");

static unsigned next_xlat;
static unsigned long long xlat_max_pa;
static uintptr_t xlat_max_va;

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
			size_t size, unsigned int attr)
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

#if DEBUG

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

#endif /* DEBUG */

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

static uint64_t mmap_desc(unsigned attr, unsigned long long addr_pa,
							int level)
{
	uint64_t desc = addr_pa;
	int mem_type;

	desc |= level == 3 ? TABLE_DESC : BLOCK_DESC;

	desc |= attr & MT_NS ? LOWER_ATTRS(NS) : 0;

	desc |= attr & MT_RW ? LOWER_ATTRS(AP_RW) : LOWER_ATTRS(AP_RO);

	desc |= LOWER_ATTRS(ACCESS_FLAG);

	mem_type = MT_TYPE(attr);
	if (mem_type == MT_MEMORY) {
		desc |= LOWER_ATTRS(ATTR_IWBWA_OWBWA_NTR_INDEX | ISH);
		if (attr & MT_RW)
			desc |= UPPER_ATTRS(XN);
	} else if (mem_type == MT_NON_CACHEABLE) {
		desc |= LOWER_ATTRS(ATTR_NON_CACHEABLE_INDEX | OSH);
		if (attr & MT_RW)
			desc |= UPPER_ATTRS(XN);
	} else {
		assert(mem_type == MT_DEVICE);
		desc |= LOWER_ATTRS(ATTR_DEVICE_INDEX | OSH);
		desc |= UPPER_ATTRS(XN);
	}

	debug_print((mem_type == MT_MEMORY) ? "MEM" :
		((mem_type == MT_NON_CACHEABLE) ? "NC" : "DEV"));
	debug_print(attr & MT_RW ? "-RW" : "-RO");
	debug_print(attr & MT_NS ? "-NS" : "-S");

	return desc;
}

/*
 * Returns attributes of area at `base_va` with size `size`. It returns the
 * attributes of the innermost region that contains it. If there are partial
 * overlaps, it returns -1, as a smaller size is needed.
 */
static int mmap_region_attr(mmap_region_t *mm, uintptr_t base_va,
					size_t size)
{
	/* Don't assume that the area is contained in the first region */
	int attr = -1;

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
			return attr; /* Reached end of list */

		if (mm->base_va >= base_va + size)
			return attr; /* Next region is after area so end */

		if (mm->base_va + mm->size <= base_va)
			continue; /* Next region has already been overtaken */

		if (mm->attr == attr)
			continue; /* Region doesn't override attribs so skip */

		if (mm->base_va > base_va ||
			mm->base_va + mm->size < base_va + size)
			return -1; /* Region doesn't fully cover our area */

		attr = mm->attr;
	}
}

static mmap_region_t *init_xlation_table_inner(mmap_region_t *mm,
					uintptr_t base_va,
					uint64_t *table,
					int level)
{
	unsigned level_size_shift = L1_XLAT_ADDRESS_SHIFT - (level - 1) *
						XLAT_TABLE_ENTRIES_SHIFT;
	unsigned level_size = 1 << level_size_shift;
	unsigned long long level_index_mask =
		((unsigned long long) XLAT_TABLE_ENTRIES_MASK)
		<< level_size_shift;

	assert(level > 0 && level <= 3);

	debug_print("New xlat table:\n");

	do  {
		uint64_t desc = UNSET_DESC;

		if (!mm->size) {
			/* Done mapping regions; finish zeroing the table */
			desc = INVALID_DESC;
		} else if (mm->base_va + mm->size <= base_va) {
			/* This area is after the region so get next region */
			++mm;
			continue;
		}

		debug_print("%s VA:%p size:0x%x ", get_level_spacer(level),
				(void *)base_va, level_size);

		if (mm->base_va >= base_va + level_size) {
			/* Next region is after this area. Nothing to map yet */
			desc = INVALID_DESC;
		} else {
			/*
			 * Try to get attributes of this area. It will fail if
			 * there are partially overlapping regions. On success,
			 * it will return the innermost region's attributes.
			 */
			int attr = mmap_region_attr(mm, base_va, level_size);
			if (attr >= 0) {
				desc = mmap_desc(attr,
					base_va - mm->base_va + mm->base_pa,
					level);
			}
		}

		if (desc == UNSET_DESC) {
			/* Area not covered by a region so need finer table */
			uint64_t *new_table = xlat_tables[next_xlat++];
			assert(next_xlat <= MAX_XLAT_TABLES);
			desc = TABLE_DESC | (uint64_t)new_table;

			/* Recurse to fill in new table */
			mm = init_xlation_table_inner(mm, base_va,
						new_table, level+1);
		}

		debug_print("\n");

		*table++ = desc;
		base_va += level_size;
	} while ((base_va & level_index_mask) && (base_va < ADDR_SPACE_SIZE));

	return mm;
}

void init_xlation_table(uintptr_t base_va, uint64_t *table,
			int level, uintptr_t *max_va,
			unsigned long long *max_pa)
{

	init_xlation_table_inner(mmap, base_va, table, level);
	*max_va = xlat_max_va;
	*max_pa = xlat_max_pa;
}
