/*
 * Copyright (c) 2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <asm_macros.S>
#include <assert_macros.S>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <platform_def.h>

	.global	enable_mpu_direct_el2

	/* void enable_mmu_direct_el2(unsigned int flags) */
func enable_mpu_direct_el2
#if ENABLE_ASSERTIONS
	mrs	x1, sctlr_el2
	tst	x1, #SCTLR_M_BIT
	ASM_ASSERT(eq)
#endif
	mov	x7, x0
	adrp	x0, mmu_cfg_params
	add	x0, x0, :lo12:mmu_cfg_params

	/* (MAIRs are already set up) */

	/* TCR */
	ldr	x2, [x0, #(MMU_CFG_TCR << 3)]
	msr	tcr_el2, x2

	/*
	 * Ensure all translation table writes have drained into memory, the TLB
	 * invalidation is complete, and translation register writes are
	 * committed before enabling the MMU
	 */
	dsb	ish
	isb

	/* Set and clear required fields of SCTLR */
	mrs	x4, sctlr_el2
	mov_imm	x5, SCTLR_WXN_BIT | SCTLR_C_BIT | SCTLR_M_BIT
	orr	x4, x4, x5

	/* Additionally, amend SCTLR fields based on flags */
	bic	x5, x4, #SCTLR_C_BIT
	tst	x7, #DISABLE_DCACHE
	csel	x4, x5, x4, ne

	msr	sctlr_el2, x4
	isb

	ret
endfunc enable_mpu_direct_el2
