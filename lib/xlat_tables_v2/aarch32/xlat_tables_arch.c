/*
 * Copyright (c) 2017-2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdbool.h>

#include <platform_def.h>

#include <arch.h>
#include <arch_features.h>
#include <arch_helpers.h>
#include <lib/cassert.h>
#include <lib/utils_def.h>
#include <lib/xlat_tables/xlat_tables_v2.h>

#include "../xlat_tables_private.h"

#if (ARM_ARCH_MAJOR == 7) && !defined(ARMV7_SUPPORTS_LARGE_PAGE_ADDRESSING)
#error ARMv7 target does not support LPAE MMU descriptors
#endif

/*
 * Returns true if the provided granule size is supported, false otherwise.
 */
bool xlat_arch_is_granule_size_supported(size_t size)
{
	/*
	 * The library uses the long descriptor translation table format, which
	 * supports 4 KiB pages only.
	 */
	return size == PAGE_SIZE_4KB;
}

size_t xlat_arch_get_max_supported_granule_size(void)
{
	return PAGE_SIZE_4KB;
}

/*
 * Determine the physical address space encoded in the 'attr' parameter.
 *
 * The physical address will fall into one of two spaces; secure or
 * nonsecure.
 */
uint32_t xlat_arch_get_pas(uint32_t attr)
{
	uint32_t pas = MT_PAS(attr);

	if (pas == MT_NS) {
		return LOWER_ATTRS(NS);
	} else { /* MT_SECURE */
		return 0U;
	}
}

#if ENABLE_ASSERTIONS
unsigned long long xlat_arch_get_max_supported_pa(void)
{
	/* Physical address space size for long descriptor format. */
	return (1ULL << 40) - 1ULL;
}

/*
 * Return minimum virtual address space size supported by the architecture
 */
uintptr_t xlat_get_min_virt_addr_space_size(void)
{
	return MIN_VIRT_ADDR_SPACE_SIZE;
}
#endif /* ENABLE_ASSERTIONS*/

bool is_mmu_enabled_ctx(const xlat_ctx_t *ctx)
{
	if (ctx->xlat_regime == EL1_EL0_REGIME) {
		assert(xlat_arch_current_el() == 1U);
		return (read_sctlr() & SCTLR_M_BIT) != 0U;
	} else {
		assert(ctx->xlat_regime == EL2_REGIME);
		assert(xlat_arch_current_el() == 2U);
		return (read_hsctlr() & HSCTLR_M_BIT) != 0U;
	}
}

bool is_dcache_enabled(void)
{
	if (IS_IN_EL2()) {
		return (read_hsctlr() & HSCTLR_C_BIT) != 0U;
	} else {
		return (read_sctlr() & SCTLR_C_BIT) != 0U;
	}
}

uint64_t xlat_arch_regime_get_xn_desc(int xlat_regime)
{
	if (xlat_regime == EL1_EL0_REGIME) {
		return UPPER_ATTRS(XN) | UPPER_ATTRS(PXN);
	} else {
		assert(xlat_regime == EL2_REGIME);
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

	if (xlat_regime == EL1_EL0_REGIME) {
		tlbimvaais(TLBI_ADDR(va));
	} else {
		assert(xlat_regime == EL2_REGIME);
		tlbimvahis(TLBI_ADDR(va));
	}
}

void xlat_arch_tlbi_va_sync(void)
{
	/* Invalidate all entries from branch predictors. */
	bpiallis();

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
	if (IS_IN_HYP()) {
		return 2U;
	} else {
		assert(IS_IN_SVC() || IS_IN_MON());
		/*
		 * If EL3 is in AArch32 mode, all secure PL1 modes (Monitor,
		 * System, SVC, Abort, UND, IRQ and FIQ modes) execute at EL3.
		 *
		 * The PL1&0 translation regime in AArch32 behaves like the
		 * EL1&0 regime in AArch64 except for the XN bits, but we set
		 * and unset them at the same time, so there's no difference in
		 * practice.
		 */
		return 1U;
	}
}

/*******************************************************************************
 * Function for enabling the MMU in PL1 or PL2, assuming that the page tables
 * have already been created.
 ******************************************************************************/
void setup_mmu_cfg(uint64_t *params, unsigned int flags,
		   const uint64_t *base_table, unsigned long long max_pa,
		   uintptr_t max_va, __unused int xlat_regime)
{
	uint64_t mair, ttbr0;
	uint32_t ttbcr;

	/* Set attributes in the right indices of the MAIR */
	mair = MAIR0_ATTR_SET(ATTR_DEVICE, ATTR_DEVICE_INDEX);
	mair |= MAIR0_ATTR_SET(ATTR_IWBWA_OWBWA_NTR,
			ATTR_IWBWA_OWBWA_NTR_INDEX);
	mair |= MAIR0_ATTR_SET(ATTR_NON_CACHEABLE,
			ATTR_NON_CACHEABLE_INDEX);

	/*
	 * Configure the control register for stage 1 of the PL1&0 or EL2
	 * translation regimes.
	 */

	/* Use the Long-descriptor translation table format. */
	ttbcr = TTBCR_EAE_BIT;

	if (xlat_regime == EL1_EL0_REGIME) {
		assert(IS_IN_SVC() || IS_IN_MON());
		/*
		 * Disable translation table walk for addresses that are
		 * translated using TTBR1. Therefore, only TTBR0 is used.
		 */
		ttbcr |= TTBCR_EPD1_BIT;
	} else {
		assert(xlat_regime == EL2_REGIME);
		assert(IS_IN_HYP());

		/*
		 * Set HTCR bits as well. Set HTTBR table properties
		 * as Inner & outer WBWA & shareable.
		 */
		ttbcr |= HTCR_RES1 |
			 HTCR_SH0_INNER_SHAREABLE | HTCR_RGN0_OUTER_WBA |
			 HTCR_RGN0_INNER_WBA;
	}

	/*
	 * Limit the input address ranges and memory region sizes translated
	 * using TTBR0 to the given virtual address space size, if smaller than
	 * 32 bits.
	 */
	if (max_va != UINT32_MAX) {
		uintptr_t virtual_addr_space_size = max_va + 1U;

		assert(virtual_addr_space_size >=
			xlat_get_min_virt_addr_space_size());
		assert(IS_POWER_OF_TWO(virtual_addr_space_size));

		/*
		 * __builtin_ctzll(0) is undefined but here we are guaranteed
		 * that virtual_addr_space_size is in the range [1, UINT32_MAX].
		 */
		int t0sz = 32 - __builtin_ctzll(virtual_addr_space_size);

		ttbcr |= (uint32_t) t0sz;
	}

	/*
	 * Set the cacheability and shareability attributes for memory
	 * associated with translation table walks using TTBR0.
	 */
	if ((flags & XLAT_TABLE_NC) != 0U) {
		/* Inner & outer non-cacheable non-shareable. */
		ttbcr |= TTBCR_SH0_NON_SHAREABLE | TTBCR_RGN0_OUTER_NC |
			TTBCR_RGN0_INNER_NC;
	} else {
		/* Inner & outer WBWA & shareable. */
		ttbcr |= TTBCR_SH0_INNER_SHAREABLE | TTBCR_RGN0_OUTER_WBA |
			TTBCR_RGN0_INNER_WBA;
	}

	/* Set TTBR0 bits as well */
	ttbr0 = (uint64_t)(uintptr_t) base_table;

	if (is_armv8_2_ttcnp_present()) {
		/* Enable CnP bit so as to share page tables with all PEs. */
		ttbr0 |= TTBR_CNP_BIT;
	}

	/* Now populate MMU configuration */
	params[MMU_CFG_MAIR] = mair;
	params[MMU_CFG_TCR] = (uint64_t) ttbcr;
	params[MMU_CFG_TTBR0] = ttbr0;
}
