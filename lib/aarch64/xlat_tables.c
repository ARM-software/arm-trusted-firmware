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

#include <arch.h>
#include <arch_helpers.h>
#include <assert.h>
#include <bl_common.h>
#include <cassert.h>
#include <platform_def.h>
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

CASSERT(ADDR_SPACE_SIZE > 0, assert_valid_addr_space_size);

#define UNSET_DESC	~0ul

#define NUM_L1_ENTRIES (ADDR_SPACE_SIZE >> L1_XLAT_ADDRESS_SHIFT)

static uint64_t l1_xlation_table[NUM_L1_ENTRIES]
__aligned(NUM_L1_ENTRIES * sizeof(uint64_t));

static uint64_t xlat_tables[MAX_XLAT_TABLES][XLAT_TABLE_ENTRIES]
__aligned(XLAT_TABLE_SIZE) __attribute__((section("xlat_table")));

static unsigned next_xlat;
static unsigned long max_pa;
static unsigned long max_va;
static unsigned long tcr_ps_bits;

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
		debug_print(" %010lx %010lx %10lx %x\n", mm->base_va,
					mm->base_pa, mm->size, mm->attr);
		++mm;
	};
	debug_print("\n");
#endif
}

void mmap_add_region(unsigned long base_pa, unsigned long base_va,
			unsigned long size, unsigned attr)
{
	mmap_region_t *mm = mmap;
	mmap_region_t *mm_last = mm + ARRAY_SIZE(mmap) - 1;
	unsigned long pa_end = base_pa + size - 1;
	unsigned long va_end = base_va + size - 1;

	assert(IS_PAGE_ALIGNED(base_pa));
	assert(IS_PAGE_ALIGNED(base_va));
	assert(IS_PAGE_ALIGNED(size));

	if (!size)
		return;

	/* Find correct place in mmap to insert new region */
	while (mm->base_va < base_va && mm->size)
		++mm;

	/* Make room for new region by moving other regions up by one place */
	memmove(mm + 1, mm, (uintptr_t)mm_last - (uintptr_t)mm);

	/* Check we haven't lost the empty sentinal from the end of the array */
	assert(mm_last->size == 0);

	mm->base_pa = base_pa;
	mm->base_va = base_va;
	mm->size = size;
	mm->attr = attr;

	if (pa_end > max_pa)
		max_pa = pa_end;
	if (va_end > max_va)
		max_va = va_end;
}

void mmap_add(const mmap_region_t *mm)
{
	while (mm->size) {
		mmap_add_region(mm->base_pa, mm->base_va, mm->size, mm->attr);
		++mm;
	}
}

static unsigned long mmap_desc(unsigned attr, unsigned long addr_pa,
					unsigned level)
{
	unsigned long desc = addr_pa;

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

static int mmap_region_attr(mmap_region_t *mm, unsigned long base_va,
					unsigned long size)
{
	int attr = mm->attr;

	for (;;) {
		++mm;

		if (!mm->size)
			return attr; /* Reached end of list */

		if (mm->base_va >= base_va + size)
			return attr; /* Next region is after area so end */

		if (mm->base_va + mm->size <= base_va)
			continue; /* Next region has already been overtaken */

		if ((mm->attr & attr) == attr)
			continue; /* Region doesn't override attribs so skip */

		attr &= mm->attr;

		if (mm->base_va > base_va ||
			mm->base_va + mm->size < base_va + size)
			return -1; /* Region doesn't fully cover our area */
	}
}

static mmap_region_t *init_xlation_table(mmap_region_t *mm,
					unsigned long base_va,
					unsigned long *table, unsigned level)
{
	unsigned level_size_shift = L1_XLAT_ADDRESS_SHIFT - (level - 1) *
						XLAT_TABLE_ENTRIES_SHIFT;
	unsigned level_size = 1 << level_size_shift;
	unsigned long level_index_mask = XLAT_TABLE_ENTRIES_MASK << level_size_shift;

	assert(level <= 3);

	debug_print("New xlat table:\n");

	do  {
		unsigned long desc = UNSET_DESC;

		if (mm->base_va + mm->size <= base_va) {
			/* Area now after the region so skip it */
			++mm;
			continue;
		}

		debug_print("      %010lx %8lx " + 6 - 2 * level, base_va,
				level_size);

		if (mm->base_va >= base_va + level_size) {
			/* Next region is after area so nothing to map yet */
			desc = INVALID_DESC;
		} else if (mm->base_va <= base_va && mm->base_va + mm->size >=
				base_va + level_size) {
			/* Next region covers all of area */
			int attr = mmap_region_attr(mm, base_va, level_size);
			if (attr >= 0)
				desc = mmap_desc(attr,
					base_va - mm->base_va + mm->base_pa,
					level);
		}
		/* else Next region only partially covers area, so need */

		if (desc == UNSET_DESC) {
			/* Area not covered by a region so need finer table */
			unsigned long *new_table = xlat_tables[next_xlat++];
			assert(next_xlat <= MAX_XLAT_TABLES);
			desc = TABLE_DESC | (unsigned long)new_table;

			/* Recurse to fill in new table */
			mm = init_xlation_table(mm, base_va,
						new_table, level+1);
		}

		debug_print("\n");

		*table++ = desc;
		base_va += level_size;
	} while (mm->size && (base_va & level_index_mask));

	return mm;
}

static unsigned int calc_physical_addr_size_bits(unsigned long max_addr)
{
	/* Physical address can't exceed 48 bits */
	assert((max_addr & ADDR_MASK_48_TO_63) == 0);

	/* 48 bits address */
	if (max_addr & ADDR_MASK_44_TO_47)
		return TCR_PS_BITS_256TB;

	/* 44 bits address */
	if (max_addr & ADDR_MASK_42_TO_43)
		return TCR_PS_BITS_16TB;

	/* 42 bits address */
	if (max_addr & ADDR_MASK_40_TO_41)
		return TCR_PS_BITS_4TB;

	/* 40 bits address */
	if (max_addr & ADDR_MASK_36_TO_39)
		return TCR_PS_BITS_1TB;

	/* 36 bits address */
	if (max_addr & ADDR_MASK_32_TO_35)
		return TCR_PS_BITS_64GB;

	return TCR_PS_BITS_4GB;
}

void init_xlat_tables(void)
{
	print_mmap();
	init_xlation_table(mmap, 0, l1_xlation_table, 1);
	tcr_ps_bits = calc_physical_addr_size_bits(max_pa);
	assert(max_va < ADDR_SPACE_SIZE);
}

/*******************************************************************************
 * Macro generating the code for the function enabling the MMU in the given
 * exception level, assuming that the pagetables have already been created.
 *
 *   _el:		Exception level at which the function will run
 *   _tcr_extra:	Extra bits to set in the TCR register. This mask will
 *			be OR'ed with the default TCR value.
 *   _tlbi_fct:		Function to invalidate the TLBs at the current
 *			exception level
 ******************************************************************************/
#define DEFINE_ENABLE_MMU_EL(_el, _tcr_extra, _tlbi_fct)		\
	void enable_mmu_el##_el(uint32_t flags)				\
	{								\
		uint64_t mair, tcr, ttbr;				\
		uint32_t sctlr;						\
									\
		assert(IS_IN_EL(_el));					\
		assert((read_sctlr_el##_el() & SCTLR_M_BIT) == 0);	\
									\
		/* Set attributes in the right indices of the MAIR */	\
		mair = MAIR_ATTR_SET(ATTR_DEVICE, ATTR_DEVICE_INDEX);	\
		mair |= MAIR_ATTR_SET(ATTR_IWBWA_OWBWA_NTR,		\
				ATTR_IWBWA_OWBWA_NTR_INDEX);		\
		write_mair_el##_el(mair);				\
									\
		/* Invalidate TLBs at the current exception level */	\
		_tlbi_fct();						\
									\
		/* Set TCR bits as well. */				\
		/* Inner & outer WBWA & shareable + T0SZ = 32 */	\
		tcr = TCR_SH_INNER_SHAREABLE | TCR_RGN_OUTER_WBA |	\
			TCR_RGN_INNER_WBA |				\
			(64 - __builtin_ctzl(ADDR_SPACE_SIZE));		\
		tcr |= _tcr_extra;					\
		write_tcr_el##_el(tcr);					\
									\
		/* Set TTBR bits as well */				\
		ttbr = (uint64_t) l1_xlation_table;			\
		write_ttbr0_el##_el(ttbr);				\
									\
		/* Ensure all translation table writes have drained */	\
		/* into memory, the TLB invalidation is complete, */	\
		/* and translation register writes are committed */	\
		/* before enabling the MMU */				\
		dsb();							\
		isb();							\
									\
		sctlr = read_sctlr_el##_el();				\
		sctlr |= SCTLR_WXN_BIT | SCTLR_M_BIT;			\
									\
		if (flags & DISABLE_DCACHE)				\
			sctlr &= ~SCTLR_C_BIT;				\
		else							\
			sctlr |= SCTLR_C_BIT;				\
									\
		write_sctlr_el##_el(sctlr);				\
									\
		/* Ensure the MMU enable takes effect immediately */	\
		isb();							\
	}

/* Define EL1 and EL3 variants of the function enabling the MMU */
DEFINE_ENABLE_MMU_EL(1,
		(tcr_ps_bits << TCR_EL1_IPS_SHIFT),
		tlbivmalle1)
DEFINE_ENABLE_MMU_EL(3,
		TCR_EL3_RES1 | (tcr_ps_bits << TCR_EL3_PS_SHIFT),
		tlbialle3)
