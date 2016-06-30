/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
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
 * The virtual address space size must be a power of two. As we start the initial
 * lookup at level 1, it must also be between 2 GB and 4 GB. See section
 * G4.6.5 in the ARMv8-A Architecture Reference Manual (DDI 0487A.j) for more
 * information.
 */
CASSERT(ADDR_SPACE_SIZE >= (1ull << 31) && ADDR_SPACE_SIZE <= (1ull << 32) &&
	IS_POWER_OF_TWO(ADDR_SPACE_SIZE), assert_valid_addr_space_size);

#define NUM_L1_ENTRIES (ADDR_SPACE_SIZE >> L1_XLAT_ADDRESS_SHIFT)

static uint64_t l1_xlation_table[NUM_L1_ENTRIES]
		__aligned(NUM_L1_ENTRIES * sizeof(uint64_t));

void init_xlat_tables(void)
{
	unsigned long long max_pa;
	uintptr_t max_va;
	print_mmap();
	init_xlation_table(0, l1_xlation_table, 1, &max_va, &max_pa);
	assert(max_va < ADDR_SPACE_SIZE);
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
	 * Set TTBCR bits as well. Set TTBR0 table properties as Inner
	 * & outer WBWA & shareable. Disable TTBR1.
	 */
	ttbcr = TTBCR_EAE_BIT |
		TTBCR_SH0_INNER_SHAREABLE | TTBCR_RGN0_OUTER_WBA |
		TTBCR_RGN0_INNER_WBA |
		(32 - __builtin_ctzl((uintptr_t)ADDR_SPACE_SIZE));
	ttbcr |= TTBCR_EPD1_BIT;
	write_ttbcr(ttbcr);

	/* Set TTBR0 bits as well */
	ttbr0 = (uintptr_t) l1_xlation_table;
	write64_ttbr0(ttbr0);
	write64_ttbr1(0);

	/*
	 * Ensure all translation table writes have drained
	 * into memory, the TLB invalidation is complete,
	 * and translation register writes are committed
	 * before enabling the MMU
	 */
	dsb();
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
