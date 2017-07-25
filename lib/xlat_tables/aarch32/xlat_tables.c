/*
 * Copyright (c) 2016-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_helpers.h>
#include <assert.h>
#include <cassert.h>
#include <platform_def.h>
#include <utils.h>
#include <xlat_tables.h>
#include "../xlat_tables_private.h"

/*
 * Each platform can define the size of the virtual address space, which is
 * defined in PLAT_VIRT_ADDR_SPACE_SIZE. TTBCR.TxSZ is calculated as 32 minus
 * the width of said address space. The value of TTBCR.TxSZ must be in the
 * range 0 to 7 [1], which means that the virtual address space width must be
 * in the range 32 to 25 bits.
 *
 * Here we calculate the initial lookup level from the value of
 * PLAT_VIRT_ADDR_SPACE_SIZE. For a 4 KB page size, level 1 supports virtual
 * address spaces of widths 32 to 31 bits, and level 2 from 30 to 25. Wider or
 * narrower address spaces are not supported. As a result, level 3 cannot be
 * used as initial lookup level with 4 KB granularity [1].
 *
 * For example, for a 31-bit address space (i.e. PLAT_VIRT_ADDR_SPACE_SIZE ==
 * 1 << 31), TTBCR.TxSZ will be programmed to (32 - 31) = 1. According to Table
 * G4-5 in the ARM ARM, the initial lookup level for an address space like that
 * is 1.
 *
 * See the ARMv8-A Architecture Reference Manual (DDI 0487A.j) for more
 * information:
 * [1] Section G4.6.5
 */

#if PLAT_VIRT_ADDR_SPACE_SIZE > (1ULL << (32 - TTBCR_TxSZ_MIN))

# error "PLAT_VIRT_ADDR_SPACE_SIZE is too big."

#elif PLAT_VIRT_ADDR_SPACE_SIZE > (1 << L1_XLAT_ADDRESS_SHIFT)

# define XLAT_TABLE_LEVEL_BASE	1
# define NUM_BASE_LEVEL_ENTRIES	\
		(PLAT_VIRT_ADDR_SPACE_SIZE >> L1_XLAT_ADDRESS_SHIFT)

#elif PLAT_VIRT_ADDR_SPACE_SIZE >= (1 << (32 - TTBCR_TxSZ_MAX))

# define XLAT_TABLE_LEVEL_BASE	2
# define NUM_BASE_LEVEL_ENTRIES	\
		(PLAT_VIRT_ADDR_SPACE_SIZE >> L2_XLAT_ADDRESS_SHIFT)

#else

# error "PLAT_VIRT_ADDR_SPACE_SIZE is too small."

#endif

static uint64_t base_xlation_table[NUM_BASE_LEVEL_ENTRIES]
		__aligned(NUM_BASE_LEVEL_ENTRIES * sizeof(uint64_t));

#if ENABLE_ASSERTIONS
static unsigned long long get_max_supported_pa(void)
{
	/* Physical address space size for long descriptor format. */
	return (1ULL << 40) - 1ULL;
}
#endif /* ENABLE_ASSERTIONS */

int xlat_arch_current_el(void)
{
	/*
	 * If EL3 is in AArch32 mode, all secure PL1 modes (Monitor, System,
	 * SVC, Abort, UND, IRQ and FIQ modes) execute at EL3.
	 */
	return 3;
}

uint64_t xlat_arch_get_xn_desc(int el __unused)
{
	return UPPER_ATTRS(XN);
}

void init_xlat_tables(void)
{
	unsigned long long max_pa;
	uintptr_t max_va;
	print_mmap();
	init_xlation_table(0, base_xlation_table, XLAT_TABLE_LEVEL_BASE,
						&max_va, &max_pa);

	assert(max_va <= PLAT_VIRT_ADDR_SPACE_SIZE - 1);
	assert(max_pa <= PLAT_PHY_ADDR_SPACE_SIZE - 1);
	assert((PLAT_PHY_ADDR_SPACE_SIZE - 1) <= get_max_supported_pa());
}

/*******************************************************************************
 * Function for enabling the MMU in Secure PL1, assuming that the
 * page-tables have already been created.
 ******************************************************************************/
void enable_mmu_secure(unsigned int flags)
{
	unsigned int mair0, ttbcr, sctlr;
	uint64_t ttbr0;

	assert(IS_IN_SECURE());
	assert((read_sctlr() & SCTLR_M_BIT) == 0);

	/* Set attributes in the right indices of the MAIR */
	mair0 = MAIR0_ATTR_SET(ATTR_DEVICE, ATTR_DEVICE_INDEX);
	mair0 |= MAIR0_ATTR_SET(ATTR_IWBWA_OWBWA_NTR,
			ATTR_IWBWA_OWBWA_NTR_INDEX);
	mair0 |= MAIR0_ATTR_SET(ATTR_NON_CACHEABLE,
			ATTR_NON_CACHEABLE_INDEX);
	write_mair0(mair0);

	/* Invalidate TLBs at the current exception level */
	tlbiall();

	/*
	 * Set TTBCR bits as well. Set TTBR0 table properties. Disable TTBR1.
	 */
	if (flags & XLAT_TABLE_NC) {
		/* Inner & outer non-cacheable non-shareable. */
		ttbcr = TTBCR_EAE_BIT |
			TTBCR_SH0_NON_SHAREABLE | TTBCR_RGN0_OUTER_NC |
			TTBCR_RGN0_INNER_NC |
			(32 - __builtin_ctzll(PLAT_VIRT_ADDR_SPACE_SIZE));
	} else {
		/* Inner & outer WBWA & shareable. */
		ttbcr = TTBCR_EAE_BIT |
			TTBCR_SH0_INNER_SHAREABLE | TTBCR_RGN0_OUTER_WBA |
			TTBCR_RGN0_INNER_WBA |
			(32 - __builtin_ctzll(PLAT_VIRT_ADDR_SPACE_SIZE));
	}
	ttbcr |= TTBCR_EPD1_BIT;
	write_ttbcr(ttbcr);

	/* Set TTBR0 bits as well */
	ttbr0 = (uintptr_t) base_xlation_table;
	write64_ttbr0(ttbr0);
	write64_ttbr1(0);

	/*
	 * Ensure all translation table writes have drained
	 * into memory, the TLB invalidation is complete,
	 * and translation register writes are committed
	 * before enabling the MMU
	 */
	dsbish();
	isb();

	sctlr = read_sctlr();
	sctlr |= SCTLR_WXN_BIT | SCTLR_M_BIT;

	if (flags & DISABLE_DCACHE)
		sctlr &= ~SCTLR_C_BIT;
	else
		sctlr |= SCTLR_C_BIT;

	write_sctlr(sctlr);

	/* Ensure the MMU enable takes effect immediately */
	isb();
}
