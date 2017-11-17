/*
 * Copyright (c) 2014-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_helpers.h>
#include <assert.h>
#include <bl_common.h>
#include <common_def.h>
#include <platform_def.h>
#include <sys/types.h>
#include <utils.h>
#include <xlat_tables.h>
#include <xlat_tables_arch.h>
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

#if ENABLE_ASSERTIONS
/* Physical Address ranges supported in the AArch64 Memory Model */
static const unsigned int pa_range_bits_arr[] = {
	PARANGE_0000, PARANGE_0001, PARANGE_0010, PARANGE_0011, PARANGE_0100,
	PARANGE_0101,
#if ARM_ARCH_AT_LEAST(8, 2)
	PARANGE_0110,
#endif
};

static unsigned long long get_max_supported_pa(void)
{
	u_register_t pa_range = read_id_aa64mmfr0_el1() &
						ID_AA64MMFR0_EL1_PARANGE_MASK;

	/* All other values are reserved */
	assert(pa_range < ARRAY_SIZE(pa_range_bits_arr));

	return (1ULL << pa_range_bits_arr[pa_range]) - 1ULL;
}
#endif /* ENABLE_ASSERTIONS */

int xlat_arch_current_el(void)
{
	int el = GET_EL(read_CurrentEl());

	assert(el > 0);

	return el;
}

uint64_t xlat_arch_get_xn_desc(int el)
{
	if (el == 3) {
		return UPPER_ATTRS(XN);
	} else {
		assert(el == 1);
		return UPPER_ATTRS(PXN);
	}
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
		/* Set T0SZ to (64 - width of virtual address space) */	\
		if (flags & XLAT_TABLE_NC) {				\
			/* Inner & outer non-cacheable non-shareable. */\
			tcr = TCR_SH_NON_SHAREABLE |			\
				TCR_RGN_OUTER_NC | TCR_RGN_INNER_NC |	\
				(64 - __builtin_ctzll(PLAT_VIRT_ADDR_SPACE_SIZE));\
		} else {						\
			/* Inner & outer WBWA & shareable. */		\
			tcr = TCR_SH_INNER_SHAREABLE |			\
				TCR_RGN_OUTER_WBA | TCR_RGN_INNER_WBA |	\
				(64 - __builtin_ctzll(PLAT_VIRT_ADDR_SPACE_SIZE));\
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
		/*
		 * TCR_EL1.EPD1: Disable translation table walk for addresses
		 * that are translated using TTBR1_EL1.
		 */
		TCR_EPD1_BIT | (tcr_ps_bits << TCR_EL1_IPS_SHIFT),
		tlbivmalle1)
DEFINE_ENABLE_MMU_EL(3,
		TCR_EL3_RES1 | (tcr_ps_bits << TCR_EL3_PS_SHIFT),
		tlbialle3)
