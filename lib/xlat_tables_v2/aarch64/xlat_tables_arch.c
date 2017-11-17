/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_helpers.h>
#include <assert.h>
#include <bl_common.h>
#include <cassert.h>
#include <common_def.h>
#include <sys/types.h>
#include <utils.h>
#include <utils_def.h>
#include <xlat_tables_v2.h>
#include "../xlat_tables_private.h"

unsigned long long tcr_physical_addr_size_bits(unsigned long long max_addr)
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

unsigned long long xlat_arch_get_max_supported_pa(void)
{
	u_register_t pa_range = read_id_aa64mmfr0_el1() &
						ID_AA64MMFR0_EL1_PARANGE_MASK;

	/* All other values are reserved */
	assert(pa_range < ARRAY_SIZE(pa_range_bits_arr));

	return (1ull << pa_range_bits_arr[pa_range]) - 1ull;
}
#endif /* ENABLE_ASSERTIONS*/

int is_mmu_enabled_ctx(const xlat_ctx_t *ctx)
{
	if (ctx->xlat_regime == EL1_EL0_REGIME) {
		assert(xlat_arch_current_el() >= 1);
		return (read_sctlr_el1() & SCTLR_M_BIT) != 0;
	} else {
		assert(ctx->xlat_regime == EL3_REGIME);
		assert(xlat_arch_current_el() >= 3);
		return (read_sctlr_el3() & SCTLR_M_BIT) != 0;
	}
}


void xlat_arch_tlbi_va(uintptr_t va)
{
#if IMAGE_EL == 1
	assert(IS_IN_EL(1));
	xlat_arch_tlbi_va_regime(va, EL1_EL0_REGIME);
#elif IMAGE_EL == 3
	assert(IS_IN_EL(3));
	xlat_arch_tlbi_va_regime(va, EL3_REGIME);
#endif
}

void xlat_arch_tlbi_va_regime(uintptr_t va, xlat_regime_t xlat_regime)
{
	/*
	 * Ensure the translation table write has drained into memory before
	 * invalidating the TLB entry.
	 */
	dsbishst();

	/*
	 * This function only supports invalidation of TLB entries for the EL3
	 * and EL1&0 translation regimes.
	 *
	 * Also, it is architecturally UNDEFINED to invalidate TLBs of a higher
	 * exception level (see section D4.9.2 of the ARM ARM rev B.a).
	 */
	if (xlat_regime == EL1_EL0_REGIME) {
		assert(xlat_arch_current_el() >= 1);
		tlbivaae1is(TLBI_ADDR(va));
	} else {
		assert(xlat_regime == EL3_REGIME);
		assert(xlat_arch_current_el() >= 3);
		tlbivae3is(TLBI_ADDR(va));
	}
}

void xlat_arch_tlbi_va_sync(void)
{
	/*
	 * A TLB maintenance instruction can complete at any time after
	 * it is issued, but is only guaranteed to be complete after the
	 * execution of DSB by the PE that executed the TLB maintenance
	 * instruction. After the TLB invalidate instruction is
	 * complete, no new memory accesses using the invalidated TLB
	 * entries will be observed by any observer of the system
	 * domain. See section D4.8.2 of the ARMv8 (issue k), paragraph
	 * "Ordering and completion of TLB maintenance instructions".
	 */
	dsbish();

	/*
	 * The effects of a completed TLB maintenance instruction are
	 * only guaranteed to be visible on the PE that executed the
	 * instruction after the execution of an ISB instruction by the
	 * PE that executed the TLB maintenance instruction.
	 */
	isb();
}

int xlat_arch_current_el(void)
{
	int el = GET_EL(read_CurrentEl());

	assert(el > 0);

	return el;
}

/*******************************************************************************
 * Macro generating the code for the function enabling the MMU in the given
 * exception level, assuming that the pagetables have already been created.
 *
 *   _el:		Exception level at which the function will run
 *   _tlbi_fct:		Function to invalidate the TLBs at the current
 *			exception level
 ******************************************************************************/
#define DEFINE_ENABLE_MMU_EL(_el, _tlbi_fct)				\
	static void enable_mmu_internal_el##_el(int flags,		\
						uint64_t mair,		\
						uint64_t tcr,		\
						uint64_t ttbr)		\
	{								\
		uint32_t sctlr = read_sctlr_el##_el();			\
		assert((sctlr & SCTLR_M_BIT) == 0);			\
									\
		/* Invalidate TLBs at the current exception level */	\
		_tlbi_fct();						\
									\
		write_mair_el##_el(mair);				\
		write_tcr_el##_el(tcr);					\
									\
		/* Set TTBR bits as well */				\
		if (ARM_ARCH_AT_LEAST(8, 2)) {				\
			/* Enable CnP bit so as to share page tables */	\
			/* with all PEs. This is mandatory for */	\
			/* ARMv8.2 implementations. */			\
			ttbr |= TTBR_CNP_BIT;				\
		}							\
		write_ttbr0_el##_el(ttbr);				\
									\
		/* Ensure all translation table writes have drained */	\
		/* into memory, the TLB invalidation is complete, */	\
		/* and translation register writes are committed */	\
		/* before enabling the MMU */				\
		dsbish();						\
		isb();							\
									\
		sctlr |= SCTLR_WXN_BIT | SCTLR_M_BIT;			\
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
#if IMAGE_EL == 1
DEFINE_ENABLE_MMU_EL(1, tlbivmalle1)
#elif IMAGE_EL == 3
DEFINE_ENABLE_MMU_EL(3, tlbialle3)
#endif

void enable_mmu_arch(unsigned int flags,
		uint64_t *base_table,
		unsigned long long max_pa,
		uintptr_t max_va)
{
	uint64_t mair, ttbr, tcr;

	/* Set attributes in the right indices of the MAIR. */
	mair = MAIR_ATTR_SET(ATTR_DEVICE, ATTR_DEVICE_INDEX);
	mair |= MAIR_ATTR_SET(ATTR_IWBWA_OWBWA_NTR, ATTR_IWBWA_OWBWA_NTR_INDEX);
	mair |= MAIR_ATTR_SET(ATTR_NON_CACHEABLE, ATTR_NON_CACHEABLE_INDEX);

	ttbr = (uint64_t) base_table;

	/*
	 * Set TCR bits as well.
	 */

	/*
	 * Limit the input address ranges and memory region sizes translated
	 * using TTBR0 to the given virtual address space size.
	 */
	assert(max_va < UINTPTR_MAX);
	uintptr_t virtual_addr_space_size = max_va + 1;
	assert(CHECK_VIRT_ADDR_SPACE_SIZE(virtual_addr_space_size));
	/*
	 * __builtin_ctzll(0) is undefined but here we are guaranteed that
	 * virtual_addr_space_size is in the range [1,UINTPTR_MAX].
	 */
	tcr = 64 - __builtin_ctzll(virtual_addr_space_size);

	/*
	 * Set the cacheability and shareability attributes for memory
	 * associated with translation table walks.
	 */
	if (flags & XLAT_TABLE_NC) {
		/* Inner & outer non-cacheable non-shareable. */
		tcr |= TCR_SH_NON_SHAREABLE |
			TCR_RGN_OUTER_NC | TCR_RGN_INNER_NC;
	} else {
		/* Inner & outer WBWA & shareable. */
		tcr |= TCR_SH_INNER_SHAREABLE |
			TCR_RGN_OUTER_WBA | TCR_RGN_INNER_WBA;
	}

	/*
	 * It is safer to restrict the max physical address accessible by the
	 * hardware as much as possible.
	 */
	unsigned long long tcr_ps_bits = tcr_physical_addr_size_bits(max_pa);

#if IMAGE_EL == 1
	assert(IS_IN_EL(1));
	/*
	 * TCR_EL1.EPD1: Disable translation table walk for addresses that are
	 * translated using TTBR1_EL1.
	 */
	tcr |= TCR_EPD1_BIT | (tcr_ps_bits << TCR_EL1_IPS_SHIFT);
	enable_mmu_internal_el1(flags, mair, tcr, ttbr);
#elif IMAGE_EL == 3
	assert(IS_IN_EL(3));
	tcr |= TCR_EL3_RES1 | (tcr_ps_bits << TCR_EL3_PS_SHIFT);
	enable_mmu_internal_el3(flags, mair, tcr, ttbr);
#endif
}
