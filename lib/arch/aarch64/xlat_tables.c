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


#define UNSET_DESC	~0ul

#define NUM_L1_ENTRIES (ADDR_SPACE_SIZE >> L1_XLAT_ADDRESS_SHIFT)

uint64_t l1_xlation_table[NUM_L1_ENTRIES]
__attribute__ ((aligned(NUM_L1_ENTRIES * sizeof(uint64_t))));

static uint64_t xlat_tables[MAX_XLAT_TABLES][XLAT_TABLE_ENTRIES]
__attribute__ ((aligned(XLAT_TABLE_SIZE), section("xlat_table")));

static unsigned next_xlat = 0;

static mmap_region mmap[MAX_MMAP_REGIONS+1];


void add_mmap_region(unsigned long base, unsigned long size, unsigned attr)
{
	mmap_region *mm = mmap;
	mmap_region *mm_last = mm + sizeof(mmap) / sizeof(mmap[0]) - 1;

	assert(IS_PAGE_ALIGNED(base));
	assert(IS_PAGE_ALIGNED(size));

	if (!size)
		return;

	/* Find place in mmap for new region */
	while (mm->base < base && mm->size)
		++mm;

	/* Check there is space for another region */
	assert(mm_last[-1].size == 0);

	/* Insert new region */
	memmove(mm + 1, mm, (uintptr_t)mm_last - (uintptr_t)mm);
	mm->base = base;
	mm->size = size;
	mm->attr = attr;
}

void add_mmap(const mmap_region *mm)
{
	while (mm->size) {
		add_mmap_region(mm->base, mm->size, mm->attr);
		++mm;
	}
}

static unsigned long mmap_desc(unsigned attr, unsigned long addr,
					unsigned level)
	{
	unsigned long desc = addr;

	desc |= level == 3 ? TABLE_DESC : BLOCK_DESC;

	desc |= attr & mt_ns ? LOWER_ATTRS(NS) : 0;

	desc |= attr & mt_rw ? LOWER_ATTRS(AP_RW) : LOWER_ATTRS(AP_RO);

	desc |= LOWER_ATTRS(ACCESS_FLAG);

	if (attr & mt_memory) {
		desc |= LOWER_ATTRS(ATTR_IWBWA_OWBWA_NTR_INDEX | ISH);
		if (attr & mt_rw)
			desc |= UPPER_ATTRS(XN);
	} else {
		desc |= LOWER_ATTRS(ATTR_DEVICE_INDEX | OSH);
		desc |= UPPER_ATTRS(XN);
	}

	return desc;
}

static int mmap_region_attr(mmap_region *mm, unsigned long base,
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

		if (mm->base > base || mm->base + mm->size < base + size )
			return -1; /* Region doesn't fully cover our area */
	}
}

static mmap_region *init_xlation_table(mmap_region *mm, unsigned long base, 
					unsigned long* table, unsigned level)
{
	unsigned level_size_shift = L1_XLAT_ADDRESS_SHIFT - (level - 1) * 
						XLAT_TABLE_ENTRIES_SHIFT;
	unsigned level_size = 1 << level_size_shift;
	unsigned level_index_mask = XLAT_TABLE_ENTRIES_MASK << level_size_shift;

	assert(level <= 3);

	do  {
		unsigned long desc = UNSET_DESC;

		if (mm->base + mm->size <= base) {
			/* Area now after the region so skip it */
			++mm;
			continue;
		}

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

		*table++ = desc;
		base += level_size;
	} while (mm->size && (base & level_index_mask));

	return mm;
}

void init_xlat_tables(void)
{
	init_xlation_table(mmap, 0, l1_xlation_table, 1);
}
