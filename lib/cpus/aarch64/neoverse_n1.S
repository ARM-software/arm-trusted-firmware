/*
 * Copyright (c) 2017-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <neoverse_n1.h>
#include <cpuamu.h>
#include <cpu_macros.S>

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "Neoverse N1 must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

/* --------------------------------------------------
 * Errata Workaround for Neoverse N1 Errata
 * This applies to revision r0p0 and r1p0 of Neoverse N1.
 * Inputs:
 * x0: variant[4:7] and revision[0:3] of current cpu.
 * Shall clobber: x0-x17
 * --------------------------------------------------
 */
func errata_n1_1043202_wa
	/* Compare x0 against revision r1p0 */
	mov	x17, x30
	bl	check_errata_1043202
	cbz	x0, 1f

	/* Apply instruction patching sequence */
	ldr	x0, =0x0
	msr	CPUPSELR_EL3, x0
	ldr	x0, =0xF3BF8F2F
	msr	CPUPOR_EL3, x0
	ldr	x0, =0xFFFFFFFF
	msr	CPUPMR_EL3, x0
	ldr	x0, =0x800200071
	msr	CPUPCR_EL3, x0
	isb
1:
	ret	x17
endfunc errata_n1_1043202_wa

func check_errata_1043202
	/* Applies to r0p0 and r1p0 */
	mov	x1, #0x10
	b	cpu_rev_var_ls
endfunc check_errata_1043202

/* --------------------------------------------------
 * Disable speculative loads if Neoverse N1 supports
 * SSBS.
 *
 * Shall clobber: x0.
 * --------------------------------------------------
 */
func neoverse_n1_disable_speculative_loads
	/* Check if the PE implements SSBS */
	mrs	x0, id_aa64pfr1_el1
	tst	x0, #(ID_AA64PFR1_EL1_SSBS_MASK << ID_AA64PFR1_EL1_SSBS_SHIFT)
	b.eq	1f

	/* Disable speculative loads */
	msr	SSBS, xzr
	isb

1:
	ret
endfunc neoverse_n1_disable_speculative_loads

func neoverse_n1_reset_func
	mov	x19, x30

	bl neoverse_n1_disable_speculative_loads

	/* Forces all cacheable atomic instructions to be near */
	mrs	x0, NEOVERSE_N1_CPUACTLR2_EL1
	orr	x0, x0, #NEOVERSE_N1_CPUACTLR2_EL1_BIT_2
	msr	NEOVERSE_N1_CPUACTLR2_EL1, x0
	isb

	bl	cpu_get_rev_var
	mov	x18, x0

#if ERRATA_N1_1043202
	mov	x0, x18
	bl	errata_n1_1043202_wa
#endif

#if ENABLE_AMU
	/* Make sure accesses from EL0/EL1 and EL2 are not trapped to EL3 */
	mrs	x0, actlr_el3
	orr	x0, x0, #NEOVERSE_N1_ACTLR_AMEN_BIT
	msr	actlr_el3, x0
	isb

	/* Make sure accesses from EL0/EL1 are not trapped to EL2 */
	mrs	x0, actlr_el2
	orr	x0, x0, #NEOVERSE_N1_ACTLR_AMEN_BIT
	msr	actlr_el2, x0
	isb

	/* Enable group0 counters */
	mov	x0, #NEOVERSE_N1_AMU_GROUP0_MASK
	msr	CPUAMCNTENSET_EL0, x0
	isb
#endif
	ret	x19
endfunc neoverse_n1_reset_func

	/* ---------------------------------------------
	 * HW will do the cache maintenance while powering down
	 * ---------------------------------------------
	 */
func neoverse_n1_core_pwr_dwn
	/* ---------------------------------------------
	 * Enable CPU power down bit in power control register
	 * ---------------------------------------------
	 */
	mrs	x0, NEOVERSE_N1_CPUPWRCTLR_EL1
	orr	x0, x0, #NEOVERSE_N1_CORE_PWRDN_EN_MASK
	msr	NEOVERSE_N1_CPUPWRCTLR_EL1, x0
	isb
	ret
endfunc neoverse_n1_core_pwr_dwn

#if REPORT_ERRATA
/*
 * Errata printing function for Neoverse N1. Must follow AAPCS.
 */
func neoverse_n1_errata_report
	stp	x8, x30, [sp, #-16]!

	bl	cpu_get_rev_var
	mov	x8, x0

	/*
	 * Report all errata. The revision-variant information is passed to
	 * checking functions of each errata.
	 */
	report_errata ERRATA_N1_1043202, neoverse_n1, 1043202

	ldp	x8, x30, [sp], #16
	ret
endfunc neoverse_n1_errata_report
#endif

	/* ---------------------------------------------
	 * This function provides neoverse_n1 specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.neoverse_n1_regs, "aS"
neoverse_n1_regs:  /* The ascii list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func neoverse_n1_cpu_reg_dump
	adr	x6, neoverse_n1_regs
	mrs	x8, NEOVERSE_N1_CPUECTLR_EL1
	ret
endfunc neoverse_n1_cpu_reg_dump

declare_cpu_ops neoverse_n1, NEOVERSE_N1_MIDR, \
	neoverse_n1_reset_func, \
	neoverse_n1_core_pwr_dwn
