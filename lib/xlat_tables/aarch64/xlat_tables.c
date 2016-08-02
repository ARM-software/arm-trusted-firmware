/*
 * Copyright (c) 2014-2016, ARM Limited and Contributors. All rights reserved.
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
#include <cassert.h>
#include <platform_def.h>
#include <utils.h>
#include <xlat_tables.h>
#include "../xlat_tables_private.h"

/*
 * Each platform can define the size of the virtual address space, which is
 * defined in ADDR_SPACE_SIZE. TCR.TxSZ is calculated as 64 minus the width of
 * said address space. The value of TCR.TxSZ must be in the range 16 to 39 [1],
 * which means that the virtual address space width must be in the range 48 to
 * 25 bits.
 *
 * Here we calculate the initial lookup level from the value of ADDR_SPACE_SIZE.
 * For a 4 KB page size, level 0 supports virtual address spaces of widths 48 to
 * 40 bits, level 1 from 39 to 31, and level 2 from 30 to 25. Wider or narrower
 * address spaces are not supported. As a result, level 3 cannot be used as
 * initial lookup level with 4 KB granularity. [2]
 *
 * For example, for a 35-bit address space (i.e. ADDR_SPACE_SIZE == 1 << 35),
 * TCR.TxSZ will be programmed to (64 - 35) = 29. According to Table D4-11 in
 * the ARM ARM, the initial lookup level for such an address space is 1.
 *
 * See the ARMv8-A Architecture Reference Manual (DDI 0487A.j) for more
 * information:
 * [1] Page 1730: 'Input address size', 'For all translation stages'.
 * [2] Section D4.2.5
 */

#if ADDR_SPACE_SIZE > (1ULL << (64 - TCR_TxSZ_MIN))

# error "ADDR_SPACE_SIZE is too big."

#elif ADDR_SPACE_SIZE > (1ULL << L0_XLAT_ADDRESS_SHIFT)

# define XLAT_TABLE_LEVEL_BASE	0
# define NUM_BASE_LEVEL_ENTRIES	(ADDR_SPACE_SIZE >> L0_XLAT_ADDRESS_SHIFT)

#elif ADDR_SPACE_SIZE > (1 << L1_XLAT_ADDRESS_SHIFT)

# define XLAT_TABLE_LEVEL_BASE	1
# define NUM_BASE_LEVEL_ENTRIES	(ADDR_SPACE_SIZE >> L1_XLAT_ADDRESS_SHIFT)

#elif ADDR_SPACE_SIZE >= (1 << (64 - TCR_TxSZ_MAX))

# define XLAT_TABLE_LEVEL_BASE	2
# define NUM_BASE_LEVEL_ENTRIES	(ADDR_SPACE_SIZE >> L2_XLAT_ADDRESS_SHIFT)

#else

# error "ADDR_SPACE_SIZE is too small."

#endif

static uint64_t base_xlation_table[NUM_BASE_LEVEL_ENTRIES]
		__aligned(NUM_BASE_LEVEL_ENTRIES * sizeof(uint64_t));

static unsigned long long tcr_ps_bits;

static unsigned long long calc_physical_addr_size_bits(
					unsigned long long max_addr)
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
	unsigned long long max_pa;
	uintptr_t max_va;
	print_mmap();
	init_xlation_table(0, base_xlation_table, XLAT_TABLE_LEVEL_BASE,
			   &max_va, &max_pa);
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
	void enable_mmu_el##_el(unsigned int flags)				\
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
		mair |= MAIR_ATTR_SET(ATTR_NON_CACHEABLE,		\
				ATTR_NON_CACHEABLE_INDEX);		\
		write_mair_el##_el(mair);				\
									\
		/* Invalidate TLBs at the current exception level */	\
		_tlbi_fct();						\
									\
		/* Set TCR bits as well. */				\
		/* Inner & outer WBWA & shareable. */			\
		/* Set T0SZ to (64 - width of virtual address space) */	\
		tcr = TCR_SH_INNER_SHAREABLE | TCR_RGN_OUTER_WBA |	\
			TCR_RGN_INNER_WBA |				\
			(64 - __builtin_ctzl(ADDR_SPACE_SIZE));		\
		tcr |= _tcr_extra;					\
		write_tcr_el##_el(tcr);					\
									\
		/* Set TTBR bits as well */				\
		ttbr = (uint64_t) base_xlation_table;			\
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
