/*
 * Copyright (c) 2016-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_helpers.h>
#include <assert.h>
#include <platform_def.h>
#include <utils.h>
#include <xlat_tables_arch.h>
#include <xlat_tables.h>
#include "../xlat_tables_private.h"

#if ARM_ARCH_MAJOR == 7 && !defined(ARMV7_SUPPORTS_LARGE_PAGE_ADDRESSING)
#error ARMv7 target does not support LPAE MMU descriptors
#endif

#define XLAT_TABLE_LEVEL_BASE	\
       GET_XLAT_TABLE_LEVEL_BASE(PLAT_VIRT_ADDR_SPACE_SIZE)

#define NUM_BASE_LEVEL_ENTRIES	\
       GET_NUM_BASE_LEVEL_ENTRIES(PLAT_VIRT_ADDR_SPACE_SIZE)

static uint64_t base_xlation_table[NUM_BASE_LEVEL_ENTRIES]
		__aligned(NUM_BASE_LEVEL_ENTRIES * sizeof(uint64_t));

uint32_t mmu_cfg_params[MMU_CFG_PARAM_MAX];

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
static void setup_mmu_cfg(unsigned int flags)
{
	unsigned int mair0, ttbcr;
	uint64_t ttbr0;

	assert(IS_IN_SECURE());
	assert((read_sctlr() & SCTLR_M_BIT) == 0);

	/* Set attributes in the right indices of the MAIR */
	mair0 = MAIR0_ATTR_SET(ATTR_DEVICE, ATTR_DEVICE_INDEX);
	mair0 |= MAIR0_ATTR_SET(ATTR_IWBWA_OWBWA_NTR,
			ATTR_IWBWA_OWBWA_NTR_INDEX);
	mair0 |= MAIR0_ATTR_SET(ATTR_NON_CACHEABLE,
			ATTR_NON_CACHEABLE_INDEX);

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

	/* Set TTBR0 bits as well */
	ttbr0 = (uintptr_t) base_xlation_table;

	/* Now populate MMU configuration */
	mmu_cfg_params[MMU_CFG_MAIR0] = mair0;
	mmu_cfg_params[MMU_CFG_TCR] = ttbcr;
	mmu_cfg_params[MMU_CFG_TTBR0_LO] = (uint32_t) ttbr0;
	mmu_cfg_params[MMU_CFG_TTBR0_HI] = ttbr0 >> 32;
}

void enable_mmu_secure(unsigned int flags)
{
	setup_mmu_cfg(flags);
	enable_mmu_direct(flags);
}
