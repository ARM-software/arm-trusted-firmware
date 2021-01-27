/*
 * Copyright (c) 2020, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <context.h>
#include <cpu_macros.S>
#include <cpuamu.h>
#include <rainier.h>

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "Rainier CPU must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

/* 64-bit only core */
#if CTX_INCLUDE_AARCH32_REGS == 1
#error "Rainier CPU supports only AArch64. Compile with CTX_INCLUDE_AARCH32_REGS=0"
#endif

/* --------------------------------------------------
 * Disable speculative loads if Rainier supports
 * SSBS.
 *
 * Shall clobber: x0.
 * --------------------------------------------------
 */
func rainier_disable_speculative_loads
	/* Check if the PE implements SSBS */
	mrs	x0, id_aa64pfr1_el1
	tst	x0, #(ID_AA64PFR1_EL1_SSBS_MASK << ID_AA64PFR1_EL1_SSBS_SHIFT)
	b.eq	1f

	/* Disable speculative loads */
	msr	SSBS, xzr

1:
	ret
endfunc rainier_disable_speculative_loads

func rainier_reset_func
	mov	x19, x30

	bl	rainier_disable_speculative_loads

	/* Forces all cacheable atomic instructions to be near */
	mrs	x0, RAINIER_CPUACTLR2_EL1
	orr	x0, x0, #RAINIER_CPUACTLR2_EL1_BIT_2
	msr	RAINIER_CPUACTLR2_EL1, x0
	isb

	bl	cpu_get_rev_var
	mov	x18, x0

#if ENABLE_AMU
	/* Make sure accesses from EL0/EL1 and EL2 are not trapped to EL3 */
	mrs	x0, actlr_el3
	orr	x0, x0, #RAINIER_ACTLR_AMEN_BIT
	msr	actlr_el3, x0

	/* Make sure accesses from EL0/EL1 are not trapped to EL2 */
	mrs	x0, actlr_el2
	orr	x0, x0, #RAINIER_ACTLR_AMEN_BIT
	msr	actlr_el2, x0

	/* Enable group0 counters */
	mov	x0, #RAINIER_AMU_GROUP0_MASK
	msr	CPUAMCNTENSET_EL0, x0
#endif

	isb
	ret	x19
endfunc rainier_reset_func

	/* ---------------------------------------------
	 * HW will do the cache maintenance while powering down
	 * ---------------------------------------------
	 */
func rainier_core_pwr_dwn
	/* ---------------------------------------------
	 * Enable CPU power down bit in power control register
	 * ---------------------------------------------
	 */
	mrs	x0, RAINIER_CPUPWRCTLR_EL1
	orr	x0, x0, #RAINIER_CORE_PWRDN_EN_MASK
	msr	RAINIER_CPUPWRCTLR_EL1, x0
	isb
	ret
endfunc rainier_core_pwr_dwn

#if REPORT_ERRATA
/*
 * Errata printing function for Rainier. Must follow AAPCS.
 */
func rainier_errata_report
	stp	x8, x30, [sp, #-16]!

	bl	cpu_get_rev_var
	mov	x8, x0

	ldp	x8, x30, [sp], #16
	ret
endfunc rainier_errata_report
#endif

	/* ---------------------------------------------
	 * This function provides Rainier specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.rainier_regs, "aS"
rainier_regs:  /* The ascii list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func rainier_cpu_reg_dump
	adr	x6, rainier_regs
	mrs	x8, RAINIER_CPUECTLR_EL1
	ret
endfunc rainier_cpu_reg_dump

declare_cpu_ops rainier, RAINIER_MIDR, \
	rainier_reset_func, \
	rainier_core_pwr_dwn
