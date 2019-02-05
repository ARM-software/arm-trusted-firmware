/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <asm_macros.S>
#include <assert_macros.S>
#include <lib/xlat_tables/xlat_tables_v2.h>

	.global	enable_mmu_direct_el1
	.global	enable_mmu_direct_el2
	.global	enable_mmu_direct_el3

	/* Macros to read and write to system register for a given EL. */
	.macro _msr reg_name, el, gp_reg
	msr	\reg_name\()_el\()\el, \gp_reg
	.endm

	.macro _mrs gp_reg, reg_name, el
	mrs	\gp_reg, \reg_name\()_el\()\el
	.endm

	.macro tlbi_invalidate_all el
	.if \el == 1
		TLB_INVALIDATE(vmalle1)
	.elseif \el == 2
		TLB_INVALIDATE(alle2)
	.elseif \el == 3
		TLB_INVALIDATE(alle3)
	.else
		.error "EL must be 1, 2 or 3"
	.endif
	.endm

	/* void enable_mmu_direct_el<x>(unsigned int flags) */
	.macro define_mmu_enable_func el
	func enable_mmu_direct_\()el\el
#if ENABLE_ASSERTIONS
		_mrs	x1, sctlr, \el
		tst	x1, #SCTLR_M_BIT
		ASM_ASSERT(eq)
#endif
		/* Invalidate all TLB entries */
		tlbi_invalidate_all \el

		mov	x7, x0
		adrp	x0, mmu_cfg_params
		add	x0, x0, :lo12:mmu_cfg_params

		/* MAIR */
		ldr	x1, [x0, #(MMU_CFG_MAIR << 3)]
		_msr	mair, \el, x1

		/* TCR */
		ldr	x2, [x0, #(MMU_CFG_TCR << 3)]
		_msr	tcr, \el, x2

		/* TTBR */
		ldr	x3, [x0, #(MMU_CFG_TTBR0 << 3)]
		_msr	ttbr0, \el, x3

		/*
		 * Ensure all translation table writes have drained into memory, the TLB
		 * invalidation is complete, and translation register writes are
		 * committed before enabling the MMU
		 */
		dsb	ish
		isb

		/* Set and clear required fields of SCTLR */
		_mrs	x4, sctlr, \el
		mov_imm	x5, SCTLR_WXN_BIT | SCTLR_C_BIT | SCTLR_M_BIT
		orr	x4, x4, x5

		/* Additionally, amend SCTLR fields based on flags */
		bic	x5, x4, #SCTLR_C_BIT
		tst	x7, #DISABLE_DCACHE
		csel	x4, x5, x4, ne

		_msr	sctlr, \el, x4
		isb

		ret
	endfunc enable_mmu_direct_\()el\el
	.endm

	/*
	 * Define MMU-enabling functions for EL1, EL2 and EL3:
	 *
	 *  enable_mmu_direct_el1
	 *  enable_mmu_direct_el2
	 *  enable_mmu_direct_el3
	 */
	define_mmu_enable_func 1
	define_mmu_enable_func 2
	define_mmu_enable_func 3
