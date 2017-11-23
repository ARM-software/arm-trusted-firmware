/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_helpers.h>
#include <assert.h>
#include <cassert.h>
#include <platform_def.h>
#include <utils.h>
#include <utils_def.h>
#include <xlat_tables_v2.h>
#include "../xlat_tables_private.h"

#if ARM_ARCH_MAJOR == 7 && !defined(ARMV7_SUPPORTS_LARGE_PAGE_ADDRESSING)
#error ARMv7 target does not support LPAE MMU descriptors
#endif

#if ENABLE_ASSERTIONS
unsigned long long xlat_arch_get_max_supported_pa(void)
{
	/* Physical address space size for long descriptor format. */
	return (1ull << 40) - 1ull;
}
#endif /* ENABLE_ASSERTIONS*/

int is_mmu_enabled_ctx(const xlat_ctx_t *ctx __unused)
{
	return (read_sctlr() & SCTLR_M_BIT) != 0;
}

void xlat_arch_tlbi_va(uintptr_t va)
{
	/*
	 * Ensure the translation table write has drained into memory before
	 * invalidating the TLB entry.
	 */
	dsbishst();

	tlbimvaais(TLBI_ADDR(va));
}

void xlat_arch_tlbi_va_regime(uintptr_t va, xlat_regime_t xlat_regime __unused)
{
	/*
	 * Ensure the translation table write has drained into memory before
	 * invalidating the TLB entry.
	 */
	dsbishst();

	tlbimvaais(TLBI_ADDR(va));
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

int xlat_arch_current_el(void)
{
	/*
	 * If EL3 is in AArch32 mode, all secure PL1 modes (Monitor, System,
	 * SVC, Abort, UND, IRQ and FIQ modes) execute at EL3.
	 */
	return 3;
}

/*******************************************************************************
 * Function for enabling the MMU in Secure PL1, assuming that the page tables
 * have already been created.
 ******************************************************************************/
void enable_mmu_arch(unsigned int flags,
		uint64_t *base_table,
		unsigned long long max_pa,
		uintptr_t max_va)
{
	u_register_t mair0, ttbcr, sctlr;
	uint64_t ttbr0;

	assert(IS_IN_SECURE());

	sctlr = read_sctlr();
	assert((sctlr & SCTLR_M_BIT) == 0);

	/* Invalidate TLBs at the current exception level */
	tlbiall();

	/* Set attributes in the right indices of the MAIR */
	mair0 = MAIR0_ATTR_SET(ATTR_DEVICE, ATTR_DEVICE_INDEX);
	mair0 |= MAIR0_ATTR_SET(ATTR_IWBWA_OWBWA_NTR,
			ATTR_IWBWA_OWBWA_NTR_INDEX);
	mair0 |= MAIR0_ATTR_SET(ATTR_NON_CACHEABLE,
			ATTR_NON_CACHEABLE_INDEX);

	/*
	 * Configure the control register for stage 1 of the PL1&0 translation
	 * regime.
	 */

	/* Use the Long-descriptor translation table format. */
	ttbcr = TTBCR_EAE_BIT;

	/*
	 * Disable translation table walk for addresses that are translated
	 * using TTBR1. Therefore, only TTBR0 is used.
	 */
	ttbcr |= TTBCR_EPD1_BIT;

	/*
	 * Limit the input address ranges and memory region sizes translated
	 * using TTBR0 to the given virtual address space size, if smaller than
	 * 32 bits.
	 */
	if (max_va != UINT32_MAX) {
		uintptr_t virtual_addr_space_size = max_va + 1;
		assert(CHECK_VIRT_ADDR_SPACE_SIZE(virtual_addr_space_size));
		/*
		 * __builtin_ctzll(0) is undefined but here we are guaranteed
		 * that virtual_addr_space_size is in the range [1, UINT32_MAX].
		 */
		ttbcr |= 32 - __builtin_ctzll(virtual_addr_space_size);
	}

	/*
	 * Set the cacheability and shareability attributes for memory
	 * associated with translation table walks using TTBR0.
	 */
	if (flags & XLAT_TABLE_NC) {
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
#if ARM_ARCH_AT_LEAST(8, 2)
	/*
	 * Enable CnP bit so as to share page tables with all PEs.
	 * Mandatory for ARMv8.2 implementations.
	 */
	ttbr0 |= TTBR_CNP_BIT;
#endif

	/* Now program the relevant system registers */
	write_mair0(mair0);
	write_ttbcr(ttbcr);
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

	sctlr |= SCTLR_WXN_BIT | SCTLR_M_BIT;

	if (flags & DISABLE_DCACHE)
		sctlr &= ~SCTLR_C_BIT;
	else
		sctlr |= SCTLR_C_BIT;

	write_sctlr(sctlr);

	/* Ensure the MMU enable takes effect immediately */
	isb();
}
