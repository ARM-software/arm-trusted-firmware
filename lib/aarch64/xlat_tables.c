/*
 * Copyright (c) 2014, ARM Limited and Contributors. All rights reserved.
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

#include <assert.h>
#include <platform.h>
#include <string.h>
#include <xlat_tables.h>


#ifndef DEBUG_XLAT_TABLE
#define DEBUG_XLAT_TABLE 0
#endif

#if DEBUG_XLAT_TABLE
#define debug_print(...) printf(__VA_ARGS__)
#else
#define debug_print(...) ((void)0)
#endif


#define UNSET_DESC	~0ul

#define NUM_L1_ENTRIES (ADDR_SPACE_SIZE >> L1_XLAT_ADDRESS_SHIFT)

uint64_t l1_xlation_table[NUM_L1_ENTRIES]
__aligned(NUM_L1_ENTRIES * sizeof(uint64_t));

static uint64_t xlat_tables[MAX_XLAT_TABLES][XLAT_TABLE_ENTRIES]
__aligned(XLAT_TABLE_SIZE) __attribute__((section("xlat_table")));

static unsigned next_xlat;

/*
 * Array of all memory regions stored in order of ascending base address.
 * The list is terminated by the first entry with size == 0.
 */
static mmap_region_t mmap[MAX_MMAP_REGIONS + 1];


static void print_mmap(void)
{
#if DEBUG_XLAT_TABLE
	debug_print("mmap:\n");
	mmap_region_t *mm = mmap;
	while (mm->size) {
		debug_print(" %010lx %10lx %x\n", mm->base, mm->size, mm->attr);
		++mm;
	};
	debug_print("\n");
#endif
}

void mmap_add_region(unsigned long base, unsigned long size, unsigned attr)
{
	mmap_region_t *mm = mmap;
	mmap_region_t *mm_last = mm + sizeof(mmap) / sizeof(mmap[0]) - 1;

	assert(IS_PAGE_ALIGNED(base));
	assert(IS_PAGE_ALIGNED(size));

	if (!size)
		return;

	/* Find correct place in mmap to insert new region */
	while (mm->base < base && mm->size)
		++mm;

	/* Make room for new region by moving other regions up by one place */
	memmove(mm + 1, mm, (uintptr_t)mm_last - (uintptr_t)mm);

	/* Check we haven't lost the empty sentinal from the end of the array */
	assert(mm_last->size == 0);

	mm->base = base;
	mm->size = size;
	mm->attr = attr;
}

void mmap_add(const mmap_region_t *mm)
{
	while (mm->size) {
		mmap_add_region(mm->base, mm->size, mm->attr);
		++mm;
	}
}

static unsigned long mmap_desc(unsigned attr, unsigned long addr,
					unsigned level)
{
	unsigned long desc = addr;

	desc |= level == 3 ? TABLE_DESC : BLOCK_DESC;

	desc |= attr & MT_NS ? LOWER_ATTRS(NS) : 0;

	desc |= attr & MT_RW ? LOWER_ATTRS(AP_RW) : LOWER_ATTRS(AP_RO);

	desc |= LOWER_ATTRS(ACCESS_FLAG);

	if (attr & MT_MEMORY) {
		desc |= LOWER_ATTRS(ATTR_IWBWA_OWBWA_NTR_INDEX | ISH);
		if (attr & MT_RW)
			desc |= UPPER_ATTRS(XN);
	} else {
		desc |= LOWER_ATTRS(ATTR_DEVICE_INDEX | OSH);
		desc |= UPPER_ATTRS(XN);
	}

	debug_print(attr & MT_MEMORY ? "MEM" : "DEV");
	debug_print(attr & MT_RW ? "-RW" : "-RO");
	debug_print(attr & MT_NS ? "-NS" : "-S");

	return desc;
}

static int mmap_region_attr(mmap_region_t *mm, unsigned long base,
					unsigned long size)
{
	int attr = mm->attr;

	for (;;) {
		++mm;

		if (!mm->size)
			return attr; /* Reached end of list */

		if (mm->base >= base + size)
			return attr; /* Next region is after area so end */

		if (mm->base + mm->size <= base)
			continue; /* Next region has already been overtaken */

		if ((mm->attr & attr) == attr)
			continue; /* Region doesn't override attribs so skip */

		attr &= mm->attr;

		if (mm->base > base || mm->base + mm->size < base + size)
			return -1; /* Region doesn't fully cover our area */
	}
}

static mmap_region_t *init_xlation_table(mmap_region_t *mm, unsigned long base,
					unsigned long *table, unsigned level)
{
	unsigned level_size_shift = L1_XLAT_ADDRESS_SHIFT - (level - 1) *
						XLAT_TABLE_ENTRIES_SHIFT;
	unsigned level_size = 1 << level_size_shift;
	unsigned level_index_mask = XLAT_TABLE_ENTRIES_MASK << level_size_shift;

	assert(level <= 3);

	debug_print("New xlat table:\n");

	do  {
		unsigned long desc = UNSET_DESC;

		if (mm->base + mm->size <= base) {
			/* Area now after the region so skip it */
			++mm;
			continue;
		}

		debug_print("      %010lx %8lx " + 6 - 2 * level, base, level_size);

		if (mm->base >= base + level_size) {
			/* Next region is after area so nothing to map yet */
			desc = INVALID_DESC;
		} else if (mm->base <= base &&
				mm->base + mm->size >= base + level_size) {
			/* Next region covers all of area */
			int attr = mmap_region_attr(mm, base, level_size);
			if (attr >= 0)
				desc = mmap_desc(attr, base, level);
		}
		/* else Next region only partially covers area, so need */

		if (desc == UNSET_DESC) {
			/* Area not covered by a region so need finer table */
			unsigned long *new_table = xlat_tables[next_xlat++];
			assert(next_xlat <= MAX_XLAT_TABLES);
			desc = TABLE_DESC | (unsigned long)new_table;

			/* Recurse to fill in new table */
			mm = init_xlation_table(mm, base, new_table, level+1);
		}

		debug_print("\n");

		*table++ = desc;
		base += level_size;
	} while (mm->size && (base & level_index_mask));

	return mm;
}

void init_xlat_tables(void)
{
	print_mmap();
	init_xlation_table(mmap, 0, l1_xlation_table, 1);
}
