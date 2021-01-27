/*
 * Copyright (c) 2014-2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdint.h>

#include <platform_def.h>

#include <arch.h>
#include <arch_features.h>
#include <common/bl_common.h>
#include <lib/utils.h>
#include <lib/xlat_tables/xlat_tables.h>
#include <lib/xlat_tables/xlat_tables_arch.h>
#include <plat/common/common_def.h>

#include "../xlat_tables_private.h"

#define XLAT_TABLE_LEVEL_BASE	\
       GET_XLAT_TABLE_LEVEL_BASE(PLAT_VIRT_ADDR_SPACE_SIZE)

#define NUM_BASE_LEVEL_ENTRIES	\
       GET_NUM_BASE_LEVEL_ENTRIES(PLAT_VIRT_ADDR_SPACE_SIZE)

static uint64_t base_xlation_table[NUM_BASE_LEVEL_ENTRIES]
		__aligned(NUM_BASE_LEVEL_ENTRIES * sizeof(uint64_t));

static unsigned long long tcr_ps_bits;

static unsigned long long calc_physical_addr_size_bits(
					unsigned long long max_addr)
{
	/* Physical address can't exceed 48 bits */
	assert((max_addr & ADDR_MASK_48_TO_63) == 0U);

	/* 48 bits address */
	if ((max_addr & ADDR_MASK_44_TO_47) != 0U)
		return TCR_PS_BITS_256TB;

	/* 44 bits address */
	if ((max_addr & ADDR_MASK_42_TO_43) != 0U)
		return TCR_PS_BITS_16TB;

	/* 42 bits address */
	if ((max_addr & ADDR_MASK_40_TO_41) != 0U)
		return TCR_PS_BITS_4TB;

	/* 40 bits address */
	if ((max_addr & ADDR_MASK_36_TO_39) != 0U)
		return TCR_PS_BITS_1TB;

	/* 36 bits address */
	if ((max_addr & ADDR_MASK_32_TO_35) != 0U)
		return TCR_PS_BITS_64GB;

	return TCR_PS_BITS_4GB;
}

#if ENABLE_ASSERTIONS
/*
 * Physical Address ranges supported in the AArch64 Memory Model. Value 0b110 is
 * supported in ARMv8.2 onwards.
 */
static const unsigned int pa_range_bits_arr[] = {
	PARANGE_0000, PARANGE_0001, PARANGE_0010, PARANGE_0011, PARANGE_0100,
	PARANGE_0101, PARANGE_0110
};

static unsigned long long get_max_supported_pa(void)
{
	u_register_t pa_range = read_id_aa64mmfr0_el1() &
						ID_AA64MMFR0_EL1_PARANGE_MASK;

	/* All other values are reserved */
	assert(pa_range < ARRAY_SIZE(pa_range_bits_arr));

	return (1ULL << pa_range_bits_arr[pa_range]) - 1ULL;
}

/*
 * Return minimum virtual address space size supported by the architecture
 */
static uintptr_t xlat_get_min_virt_addr_space_size(void)
{
	uintptr_t ret;

	if (is_armv8_4_ttst_present())
		ret = MIN_VIRT_ADDR_SPACE_SIZE_TTST;
	else
		ret = MIN_VIRT_ADDR_SPACE_SIZE;

	return ret;
}
#endif /* ENABLE_ASSERTIONS */

unsigned int xlat_arch_current_el(void)
{
	unsigned int el = (unsigned int)GET_EL(read_CurrentEl());

	assert(el > 0U);

	return el;
}

uint64_t xlat_arch_get_xn_desc(unsigned int el)
{
	if (el == 3U) {
		return UPPER_ATTRS(XN);
	} else {
		assert(el == 1U);
		return UPPER_ATTRS(PXN);
	}
}

void init_xlat_tables(void)
{
	unsigned long long max_pa;
	uintptr_t max_va;

	assert(PLAT_VIRT_ADDR_SPACE_SIZE >=
		(xlat_get_min_virt_addr_space_size() - 1U));
	assert(PLAT_VIRT_ADDR_SPACE_SIZE <= MAX_VIRT_ADDR_SPACE_SIZE);
	assert(IS_POWER_OF_TWO(PLAT_VIRT_ADDR_SPACE_SIZE));

	print_mmap();
	init_xlation_table(0U, base_xlation_table, XLAT_TABLE_LEVEL_BASE,
			   &max_va, &max_pa);

	assert(max_va <= (PLAT_VIRT_ADDR_SPACE_SIZE - 1U));
	assert(max_pa <= (PLAT_PHY_ADDR_SPACE_SIZE - 1U));
	assert((PLAT_PHY_ADDR_SPACE_SIZE - 1U) <= get_max_supported_pa());

	tcr_ps_bits = calc_physical_addr_size_bits(max_pa);
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
	void enable_mmu_el##_el(unsigned int flags)			\
	{								\
		uint64_t mair, tcr, ttbr;				\
		uint32_t sctlr;						\
									\
		assert(IS_IN_EL(_el));					\
		assert((read_sctlr_el##_el() & SCTLR_M_BIT) == 0U);	\
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
		/* Set T0SZ to (64 - width of virtual address space) */	\
		int t0sz = 64 - __builtin_ctzll(PLAT_VIRT_ADDR_SPACE_SIZE);\
									\
		if ((flags & XLAT_TABLE_NC) != 0U) {			\
			/* Inner & outer non-cacheable non-shareable. */\
			tcr = TCR_SH_NON_SHAREABLE |			\
				TCR_RGN_OUTER_NC | TCR_RGN_INNER_NC |	\
				((uint64_t)t0sz << TCR_T0SZ_SHIFT);	\
		} else {						\
			/* Inner & outer WBWA & shareable. */		\
			tcr = TCR_SH_INNER_SHAREABLE |			\
				TCR_RGN_OUTER_WBA | TCR_RGN_INNER_WBA |	\
				((uint64_t)t0sz << TCR_T0SZ_SHIFT);	\
		}							\
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
		dsbish();						\
		isb();							\
									\
		sctlr = read_sctlr_el##_el();				\
		sctlr |= SCTLR_WXN_BIT | SCTLR_M_BIT;			\
									\
		if ((flags & DISABLE_DCACHE) != 0U)			\
			sctlr &= ~SCTLR_C_BIT;				\
		else							\
			sctlr |= SCTLR_C_BIT;				\
									\
		write_sctlr_el##_el(sctlr);				\
									\
		/* Ensure the MMU enable takes effect immediately */	\
		isb();							\
	}								\
									\
	void enable_mmu_direct_el##_el(unsigned int flags)		\
	{								\
		enable_mmu_el##_el(flags);				\
	}

/* Define EL1 and EL3 variants of the function enabling the MMU */
DEFINE_ENABLE_MMU_EL(1,
		/*
		 * TCR_EL1.EPD1: Disable translation table walk for addresses
		 * that are translated using TTBR1_EL1.
		 */
		TCR_EPD1_BIT | (tcr_ps_bits << TCR_EL1_IPS_SHIFT),
		tlbivmalle1)
DEFINE_ENABLE_MMU_EL(3,
		TCR_EL3_RES1 | (tcr_ps_bits << TCR_EL3_PS_SHIFT),
		tlbialle3)
