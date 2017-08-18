/*
 * Copyright (c) 2016-2017, Linaro Limited. All rights reserved.
 * Copyright (c) 2014-2017, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2014, STMicroelectronics International N.V.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_helpers.h>
#include <assert.h>
#include <cassert.h>
#include <debug.h>
#include <platform_def.h>
#include <string.h>
#include <utils.h>
#include <xlat_tables.h>
#include "../xlat_tables_private.h"

CASSERT(ARCH_IS_ARMV7_WITHOUT_LPAE, non_lpae_support_misconfiguration);

#ifndef DEBUG_XLAT_TABLE
#define DEBUG_XLAT_TABLE	0
#endif

#if DEBUG_XLAT_TABLE
#include <stdio.h>
#define debug_print(...) tf_printf(__VA_ARGS__)
#else
#define debug_print(...) ((void)0)
#endif

CASSERT(PLAT_VIRT_ADDR_SPACE_SIZE == (1ULL << 32), invalid_vaddr_space_size);
CASSERT(PLAT_PHY_ADDR_SPACE_SIZE == (1ULL << 32), invalid_paddr_space_size);

#define MMU32B_UNSET_DESC	~0ul
#define MMU32B_INVALID_DESC	0ul

/*
 * MMU related values
 */

/* Sharable */
#define MMU32B_TTB_S           (1 << 1)

/* Not Outer Sharable */
#define MMU32B_TTB_NOS         (1 << 5)

/* Normal memory, Inner Non-cacheable */
#define MMU32B_TTB_IRGN_NC     0

/* Normal memory, Inner Write-Back Write-Allocate Cacheable */
#define MMU32B_TTB_IRGN_WBWA   (1 << 6)

/* Normal memory, Inner Write-Through Cacheable */
#define MMU32B_TTB_IRGN_WT     1

/* Normal memory, Inner Write-Back no Write-Allocate Cacheable */
#define MMU32B_TTB_IRGN_WB     (1 | (1 << 6))

/* Normal memory, Outer Write-Back Write-Allocate Cacheable */
#define MMU32B_TTB_RNG_WBWA    (1 << 3)

#define MMU32B_DEFAULT_ATTRS \
		(MMU32B_TTB_S | MMU32B_TTB_NOS | \
		 MMU32B_TTB_IRGN_WBWA | MMU32B_TTB_RNG_WBWA)

/* armv7 memory mapping attributes: section mapping */
#define SECTION_SECURE			(0 << 19)
#define SECTION_NOTSECURE		(1 << 19)
#define SECTION_SHARED			(1 << 16)
#define SECTION_NOTGLOBAL		(1 << 17)
#define SECTION_ACCESS_FLAG		(1 << 10)
#define SECTION_UNPRIV			(1 << 11)
#define SECTION_RO			(1 << 15)
#define SECTION_TEXCB(texcb)		((((texcb) >> 2) << 12) | \
					 ((((texcb) >> 1) & 0x1) << 3) | \
					 (((texcb) & 0x1) << 2))
#define SECTION_DEVICE			SECTION_TEXCB(MMU32B_ATTR_DEVICE_INDEX)
#define SECTION_NORMAL			SECTION_TEXCB(MMU32B_ATTR_DEVICE_INDEX)
#define SECTION_NORMAL_CACHED		SECTION_TEXCB(MMU32B_ATTR_IWBWA_OWBWA_INDEX)

#define SECTION_XN			(1 << 4)
#define SECTION_PXN			(1 << 0)
#define SECTION_SECTION			(2 << 0)

#define SECTION_PT_NOTSECURE		(1 << 3)
#define SECTION_PT_PT			(1 << 0)

#define SMALL_PAGE_SMALL_PAGE		(1 << 1)
#define SMALL_PAGE_SHARED		(1 << 10)
#define SMALL_PAGE_NOTGLOBAL		(1 << 11)
#define SMALL_PAGE_TEXCB(texcb)		((((texcb) >> 2) << 6) | \
					 ((((texcb) >> 1) & 0x1) << 3) | \
					 (((texcb) & 0x1) << 2))
#define SMALL_PAGE_DEVICE		SMALL_PAGE_TEXCB(MMU32B_ATTR_DEVICE_INDEX)
#define SMALL_PAGE_NORMAL		SMALL_PAGE_TEXCB(MMU32B_ATTR_DEVICE_INDEX)
#define SMALL_PAGE_NORMAL_CACHED	SMALL_PAGE_TEXCB(MMU32B_ATTR_IWBWA_OWBWA_INDEX)
#define SMALL_PAGE_ACCESS_FLAG		(1 << 4)
#define SMALL_PAGE_UNPRIV		(1 << 5)
#define SMALL_PAGE_RO			(1 << 9)
#define SMALL_PAGE_XN			(1 << 0)

/* The TEX, C and B bits concatenated */
#define MMU32B_ATTR_DEVICE_INDEX		0x0
#define MMU32B_ATTR_IWBWA_OWBWA_INDEX		0x1

#define MMU32B_PRRR_IDX(idx, tr, nos)		(((tr) << (2 * (idx))) | \
					 ((uint32_t)(nos) << ((idx) + 24)))
#define MMU32B_NMRR_IDX(idx, ir, or)		(((ir) << (2 * (idx))) | \
					 ((uint32_t)(or) << (2 * (idx) + 16)))
#define MMU32B_PRRR_DS0			(1 << 16)
#define MMU32B_PRRR_DS1			(1 << 17)
#define MMU32B_PRRR_NS0			(1 << 18)
#define MMU32B_PRRR_NS1			(1 << 19)

#define MMU32B_ATTR_DEVICE_PRRR		MMU32B_PRRR_IDX(MMU32B_ATTR_DEVICE_INDEX, 1, 0)
#define MMU32B_ATTR_DEVICE_NMRR		MMU32B_NMRR_IDX(MMU32B_ATTR_DEVICE_INDEX, 0, 0)

#define MMU32B_ATTR_IWBWA_OWBWA_PRRR	MMU32B_PRRR_IDX(MMU32B_ATTR_IWBWA_OWBWA_INDEX, 2, 1)
#define MMU32B_ATTR_IWBWA_OWBWA_NMRR	MMU32B_NMRR_IDX(MMU32B_ATTR_IWBWA_OWBWA_INDEX, 1, 1)

#define DACR_DOMAIN(num, perm)		((perm) << ((num) * 2))
#define DACR_DOMAIN_PERM_NO_ACCESS	0x0
#define DACR_DOMAIN_PERM_CLIENT		0x1
#define DACR_DOMAIN_PERM_MANAGER	0x3

#define NUM_1MB_IN_4GB		(1 << 12)
#define NUM_4K_IN_1MB		(1 << 8)

#define ONE_MB_SHIFT		20

/* mmu 32b integration */
#define MMU32B_L1_TABLE_SIZE		(NUM_1MB_IN_4GB * 4)
#define MMU32B_L2_TABLE_SIZE		(NUM_4K_IN_1MB * 4)
#define MMU32B_L1_TABLE_ALIGN		(1 << 14)
#define MMU32B_L2_TABLE_ALIGN		(1 << 10)

static unsigned next_xlat;
static unsigned long long xlat_max_pa;
static uintptr_t xlat_max_va;

static uint32_t mmu_l1_base[NUM_1MB_IN_4GB]
	__aligned(MMU32B_L1_TABLE_ALIGN) __attribute__((section("xlat_table")));

static uint32_t mmu_l2_base[MAX_XLAT_TABLES][NUM_4K_IN_1MB]
	__aligned(MMU32B_L2_TABLE_ALIGN) __attribute__((section("xlat_table")));

/*
 * Array of all memory regions stored in order of ascending base address.
 * The list is terminated by the first entry with size == 0.
 */
static mmap_region_t mmap[MAX_MMAP_REGIONS + 1];

void print_mmap(void)
{
#if LOG_LEVEL >= LOG_LEVEL_VERBOSE
	mmap_region_t *mm = mmap;

	debug_print("init xlat - l1:%p  l2:%p (%d)\n",
		    (void *)mmu_l1_base, (void *)mmu_l2_base, MAX_XLAT_TABLES);
	debug_print("mmap:\n");
	while (mm->size) {
		debug_print(" VA:%p  PA:0x%llx  size:0x%zx  attr:0x%x\n",
				(void *)mm->base_va, mm->base_pa,
				mm->size, mm->attr);
		++mm;
	};
	debug_print("\n");
#endif
}

void mmap_add(const mmap_region_t *mm)
{
	for (; mm->size; mm++)
		mmap_add_region(mm->base_pa, mm->base_va, mm->size, mm->attr);
}

void mmap_add_region(unsigned long long base_pa, uintptr_t base_va,
			size_t size, unsigned int attr)
{
	mmap_region_t *mm = mmap;
	mmap_region_t *mm_last = mm + ARRAY_SIZE(mmap) - 1;
	unsigned long long end_pa = base_pa + size - 1;
	uintptr_t end_va = base_va + size - 1;

	debug_print("add region: pa %llx, va %p, size %lx, attr %lx\n",
				base_pa, (void *)base_va,
				(unsigned long)size, (unsigned long)attr);

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
	while (mm->base_va < base_va && mm->size) {
		++mm;
		assert(mm <= mmap + ARRAY_SIZE(mmap));
	}

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
	while ((mm->base_va == base_va) && (mm->size > size)) {
		++mm;
		assert(mm <= mmap + ARRAY_SIZE(mmap));
	}

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

/* map all memory as shared/global/domain0/no-usr access */
static unsigned long mmap_desc(unsigned attr, unsigned long addr_pa,
					unsigned level)
{
	unsigned long desc;

	switch (level) {
	case 1:
		assert(!(addr_pa & (MMU32B_L1_TABLE_ALIGN - 1)));

		desc = SECTION_SECTION | SECTION_SHARED;

		desc |= attr & MT_NS ? SECTION_NOTSECURE : 0;

		desc |= SECTION_ACCESS_FLAG;
		desc |= attr & MT_RW ? 0 : SECTION_RO;

		desc |= attr & MT_MEMORY ? SECTION_NORMAL_CACHED : SECTION_DEVICE;

		if ((attr & MT_RW) || !(attr & MT_MEMORY))
			desc |= SECTION_XN;
		break;
	case 2:
		assert(!(addr_pa & (MMU32B_L2_TABLE_ALIGN - 1)));

		desc = SMALL_PAGE_SMALL_PAGE | SMALL_PAGE_SHARED;

		desc |= SMALL_PAGE_ACCESS_FLAG;
		desc |= attr & MT_RW ? 0 : SMALL_PAGE_RO;

		desc |= attr & MT_MEMORY ? SMALL_PAGE_NORMAL_CACHED : SMALL_PAGE_DEVICE;

		if ((attr & MT_RW) || !(attr & MT_MEMORY))
			desc |= SMALL_PAGE_XN;
		break;
	default:
		panic();
	}

	/* dump only the non-lpae level 2 tables */
	if (level == 2) {
		debug_print(attr & MT_MEMORY ? "MEM" : "dev");
		debug_print(attr & MT_RW ? "-rw" : "-RO");
		debug_print(attr & MT_NS ? "-NS" : "-S");
	}

	return desc | addr_pa;
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

static mmap_region_t *init_xlation_table_inner(mmap_region_t *mm,
					unsigned long base_va,
					unsigned long *table, unsigned level)
{
	unsigned level_size_shift = (level == 1) ? ONE_MB_SHIFT : FOUR_KB_SHIFT;
	unsigned level_size = 1 << level_size_shift;
	unsigned long level_index_mask = (level == 1) ?
					(NUM_1MB_IN_4GB - 1) << ONE_MB_SHIFT :
					(NUM_4K_IN_1MB - 1) << FOUR_KB_SHIFT;

	assert(level == 1 || level == 2);

	debug_print("init xlat table at %p (level%1d)\n", (void *)table, level);

	do  {
		unsigned long desc = MMU32B_UNSET_DESC;

		if (mm->base_va + mm->size <= base_va) {
			/* Area now after the region so skip it */
			++mm;
			continue;
		}

		/* dump only non-lpae level 2 tables content */
		if (level == 2)
			debug_print("      0x%lx %x " + 6 - 2 * level,
						base_va, level_size);

		if (mm->base_va >= base_va + level_size) {
			/* Next region is after area so nothing to map yet */
			desc = MMU32B_INVALID_DESC;
		} else if (mm->base_va <= base_va && mm->base_va + mm->size >=
				base_va + level_size) {
			/* Next region covers all of area */
			int attr = mmap_region_attr(mm, base_va, level_size);

			if (attr >= 0) {
				desc = mmap_desc(attr,
					base_va - mm->base_va + mm->base_pa,
					level);
			}
		}
		/* else Next region only partially covers area, so need */

		if (desc == MMU32B_UNSET_DESC) {
			unsigned long xlat_table;

			/*
			 * Area not covered by a region so need finer table
			 * Reuse next level table if any (assert attrib matching).
			 * Otherwise allocate a xlat table.
			 */
			if (*table) {
				assert((*table & 3) == SECTION_PT_PT);
				assert(!(*table & SECTION_PT_NOTSECURE) ==
							!(mm->attr & MT_NS));

				xlat_table = (*table) &
						~(MMU32B_L1_TABLE_ALIGN - 1);
				desc = *table;
			} else {
				xlat_table = (unsigned long)mmu_l2_base +
						next_xlat * MMU32B_L2_TABLE_SIZE;
				assert(++next_xlat <= MAX_XLAT_TABLES);
				memset((char *)xlat_table, 0,
						MMU32B_L2_TABLE_SIZE);

				desc = xlat_table | SECTION_PT_PT;
				desc |= mm->attr & MT_NS
					? SECTION_PT_NOTSECURE : 0;
			}
			/* Recurse to fill in new table */
			mm = init_xlation_table_inner(mm, base_va,
							(unsigned long *)xlat_table,
							level + 1);
		}

		/* dump only non-lpae level 2 tables content */
		if (level == 2)
			debug_print("\n");

		*table++ = desc;
		base_va += level_size;
	} while (mm->size && (base_va & level_index_mask));

	return mm;
}

void init_xlat_tables(void)
{
	print_mmap();

	assert(!((unsigned)mmu_l1_base & (MMU32B_L1_TABLE_ALIGN - 1)));
	assert(!((unsigned)mmu_l2_base & (MMU32B_L2_TABLE_ALIGN - 1)));

	memset(mmu_l1_base, 0, MMU32B_L1_TABLE_SIZE);

	init_xlation_table_inner(mmap, 0, (unsigned long *)mmu_l1_base, 1);

	debug_print("init xlat - max_va=%p, max_pa=%llx\n",
			(void *)xlat_max_va, xlat_max_pa);
	assert(xlat_max_va <= PLAT_VIRT_ADDR_SPACE_SIZE - 1);
	assert(xlat_max_pa <= PLAT_VIRT_ADDR_SPACE_SIZE - 1);
}

/*******************************************************************************
 * Function for enabling the MMU in Secure PL1, assuming that the
 * page-tables have already been created.
 ******************************************************************************/
void enable_mmu_secure(unsigned int flags)
{
	unsigned int prrr;
	unsigned int nmrr;
	unsigned int sctlr;

	assert(IS_IN_SECURE());
	assert((read_sctlr() & SCTLR_M_BIT) == 0);

	/* Enable Access flag (simplified access permissions) and TEX remap */
	write_sctlr(read_sctlr() | SCTLR_AFE_BIT | SCTLR_TRE_BIT);

	prrr = MMU32B_ATTR_DEVICE_PRRR | MMU32B_ATTR_IWBWA_OWBWA_PRRR;
	nmrr = MMU32B_ATTR_DEVICE_NMRR | MMU32B_ATTR_IWBWA_OWBWA_NMRR;

	prrr |= MMU32B_PRRR_NS1 | MMU32B_PRRR_DS1;

	write_prrr(prrr);
	write_nmrr(nmrr);

	/* Program Domain access control register: domain 0 only */
	write_dacr(DACR_DOMAIN(0, DACR_DOMAIN_PERM_CLIENT));

	/* Invalidate TLBs at the current exception level */
	tlbiall();

	/* set MMU base xlat table entry (use only TTBR0) */
	write_ttbr0((uint32_t)mmu_l1_base | MMU32B_DEFAULT_ATTRS);
	write_ttbr1(0);

	/*
	 * Ensure all translation table writes have drained
	 * into memory, the TLB invalidation is complete,
	 * and translation register writes are committed
	 * before enabling the MMU
	 */
	dsb();
	isb();

	sctlr = read_sctlr();
	sctlr |= SCTLR_M_BIT;
#if ARCH_IS_ARMV7_WITHOUT_VE
	sctlr |= SCTLR_WXN_BIT;
#endif

	if (flags & DISABLE_DCACHE)
		sctlr &= ~SCTLR_C_BIT;
	else
		sctlr |= SCTLR_C_BIT;

	write_sctlr(sctlr);

	/* Ensure the MMU enable takes effect immediately */
	isb();
}
