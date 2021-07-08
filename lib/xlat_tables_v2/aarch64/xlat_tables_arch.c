/*
 * Copyright (c) 2017-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#include <arch.h>
#include <arch_features.h>
#include <arch_helpers.h>
#include <lib/cassert.h>
#include <lib/utils_def.h>
#include <lib/xlat_tables/xlat_tables_v2.h>

#include "../xlat_tables_private.h"

/*
 * Returns true if the provided granule size is supported, false otherwise.
 */
bool xlat_arch_is_granule_size_supported(size_t size)
{
	u_register_t id_aa64mmfr0_el1 = read_id_aa64mmfr0_el1();

	if (size == PAGE_SIZE_4KB) {
		return ((id_aa64mmfr0_el1 >> ID_AA64MMFR0_EL1_TGRAN4_SHIFT) &
			 ID_AA64MMFR0_EL1_TGRAN4_MASK) ==
			 ID_AA64MMFR0_EL1_TGRAN4_SUPPORTED;
	} else if (size == PAGE_SIZE_16KB) {
		return ((id_aa64mmfr0_el1 >> ID_AA64MMFR0_EL1_TGRAN16_SHIFT) &
			 ID_AA64MMFR0_EL1_TGRAN16_MASK) ==
			 ID_AA64MMFR0_EL1_TGRAN16_SUPPORTED;
	} else if (size == PAGE_SIZE_64KB) {
		return ((id_aa64mmfr0_el1 >> ID_AA64MMFR0_EL1_TGRAN64_SHIFT) &
			 ID_AA64MMFR0_EL1_TGRAN64_MASK) ==
			 ID_AA64MMFR0_EL1_TGRAN64_SUPPORTED;
	} else {
		return 0;
	}
}

size_t xlat_arch_get_max_supported_granule_size(void)
{
	if (xlat_arch_is_granule_size_supported(PAGE_SIZE_64KB)) {
		return PAGE_SIZE_64KB;
	} else if (xlat_arch_is_granule_size_supported(PAGE_SIZE_16KB)) {
		return PAGE_SIZE_16KB;
	} else {
		assert(xlat_arch_is_granule_size_supported(PAGE_SIZE_4KB));
		return PAGE_SIZE_4KB;
	}
}

/*
 * Determine the physical address space encoded in the 'attr' parameter.
 *
 * The physical address will fall into one of four spaces; secure,
 * nonsecure, root, or realm if RME is enabled, or one of two spaces;
 * secure and nonsecure otherwise.
 */
uint32_t xlat_arch_get_pas(uint32_t attr)
{
	uint32_t pas = MT_PAS(attr);

	switch (pas) {
#if ENABLE_RME
	/* TTD.NSE = 1 and TTD.NS = 1 for Realm PAS */
	case MT_REALM:
		return LOWER_ATTRS(EL3_S1_NSE | NS);
	/* TTD.NSE = 1 and TTD.NS = 0 for Root PAS */
	case MT_ROOT:
		return LOWER_ATTRS(EL3_S1_NSE);
#endif
	case MT_NS:
		return LOWER_ATTRS(NS);
	default: /* MT_SECURE */
		return 0U;
	}
}

unsigned long long tcr_physical_addr_size_bits(unsigned long long max_addr)
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

unsigned long long xlat_arch_get_max_supported_pa(void)
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
uintptr_t xlat_get_min_virt_addr_space_size(void)
{
	uintptr_t ret;

	if (is_armv8_4_ttst_present())
		ret = MIN_VIRT_ADDR_SPACE_SIZE_TTST;
	else
		ret = MIN_VIRT_ADDR_SPACE_SIZE;

	return ret;
}
#endif /* ENABLE_ASSERTIONS*/

bool is_mmu_enabled_ctx(const xlat_ctx_t *ctx)
{
	if (ctx->xlat_regime == EL1_EL0_REGIME) {
		assert(xlat_arch_current_el() >= 1U);
		return (read_sctlr_el1() & SCTLR_M_BIT) != 0U;
	} else if (ctx->xlat_regime == EL2_REGIME) {
		assert(xlat_arch_current_el() >= 2U);
		return (read_sctlr_el2() & SCTLR_M_BIT) != 0U;
	} else {
		assert(ctx->xlat_regime == EL3_REGIME);
		assert(xlat_arch_current_el() >= 3U);
		return (read_sctlr_el3() & SCTLR_M_BIT) != 0U;
	}
}

bool is_dcache_enabled(void)
{
	unsigned int el = get_current_el_maybe_constant();

	if (el == 1U) {
		return (read_sctlr_el1() & SCTLR_C_BIT) != 0U;
	} else if (el == 2U) {
		return (read_sctlr_el2() & SCTLR_C_BIT) != 0U;
	} else {
		return (read_sctlr_el3() & SCTLR_C_BIT) != 0U;
	}
}

uint64_t xlat_arch_regime_get_xn_desc(int xlat_regime)
{
	if (xlat_regime == EL1_EL0_REGIME) {
		return UPPER_ATTRS(UXN) | UPPER_ATTRS(PXN);
	} else {
		assert((xlat_regime == EL2_REGIME) ||
		       (xlat_regime == EL3_REGIME));
		return UPPER_ATTRS(XN);
	}
}

void xlat_arch_tlbi_va(uintptr_t va, int xlat_regime)
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
		assert(xlat_arch_current_el() >= 1U);
		tlbivaae1is(TLBI_ADDR(va));
	} else if (xlat_regime == EL2_REGIME) {
		assert(xlat_arch_current_el() >= 2U);
		tlbivae2is(TLBI_ADDR(va));
	} else {
		assert(xlat_regime == EL3_REGIME);
		assert(xlat_arch_current_el() >= 3U);
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

unsigned int xlat_arch_current_el(void)
{
	unsigned int el = (unsigned int)GET_EL(read_CurrentEl());

	assert(el > 0U);

	return el;
}

void setup_mmu_cfg(uint64_t *params, unsigned int flags,
		   const uint64_t *base_table, unsigned long long max_pa,
		   uintptr_t max_va, int xlat_regime)
{
	uint64_t mair, ttbr0, tcr;
	uintptr_t virtual_addr_space_size;

	/* Set attributes in the right indices of the MAIR. */
	mair = MAIR_ATTR_SET(ATTR_DEVICE, ATTR_DEVICE_INDEX);
	mair |= MAIR_ATTR_SET(ATTR_IWBWA_OWBWA_NTR, ATTR_IWBWA_OWBWA_NTR_INDEX);
	mair |= MAIR_ATTR_SET(ATTR_NON_CACHEABLE, ATTR_NON_CACHEABLE_INDEX);

	/*
	 * Limit the input address ranges and memory region sizes translated
	 * using TTBR0 to the given virtual address space size.
	 */
	assert(max_va < ((uint64_t)UINTPTR_MAX));

	virtual_addr_space_size = (uintptr_t)max_va + 1U;

	assert(virtual_addr_space_size >=
		xlat_get_min_virt_addr_space_size());
	assert(virtual_addr_space_size <= MAX_VIRT_ADDR_SPACE_SIZE);
	assert(IS_POWER_OF_TWO(virtual_addr_space_size));

	/*
	 * __builtin_ctzll(0) is undefined but here we are guaranteed that
	 * virtual_addr_space_size is in the range [1,UINTPTR_MAX].
	 */
	int t0sz = 64 - __builtin_ctzll(virtual_addr_space_size);

	tcr = (uint64_t)t0sz << TCR_T0SZ_SHIFT;

	/*
	 * Set the cacheability and shareability attributes for memory
	 * associated with translation table walks.
	 */
	if ((flags & XLAT_TABLE_NC) != 0U) {
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

	if (xlat_regime == EL1_EL0_REGIME) {
		/*
		 * TCR_EL1.EPD1: Disable translation table walk for addresses
		 * that are translated using TTBR1_EL1.
		 */
		tcr |= TCR_EPD1_BIT | (tcr_ps_bits << TCR_EL1_IPS_SHIFT);
	} else if (xlat_regime == EL2_REGIME) {
		tcr |= TCR_EL2_RES1 | (tcr_ps_bits << TCR_EL2_PS_SHIFT);
	} else {
		assert(xlat_regime == EL3_REGIME);
		tcr |= TCR_EL3_RES1 | (tcr_ps_bits << TCR_EL3_PS_SHIFT);
	}

	/* Set TTBR bits as well */
	ttbr0 = (uint64_t) base_table;

	if (is_armv8_2_ttcnp_present()) {
		/* Enable CnP bit so as to share page tables with all PEs. */
		ttbr0 |= TTBR_CNP_BIT;
	}

	params[MMU_CFG_MAIR] = mair;
	params[MMU_CFG_TCR] = tcr;
	params[MMU_CFG_TTBR0] = ttbr0;
}
