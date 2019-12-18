/*
 * Copyright (c) 2019, ARM Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <cortex_hercules.h>
#include <cpu_macros.S>
#include <plat_macros.S>

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "cortex_hercules must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif


/* --------------------------------------------------
 * Errata Workaround for Hercules Erratum 1688305.
 * This applies to revision r0p0 and r1p0 of Hercules.
 * Inputs:
 * x0: variant[4:7] and revision[0:3] of current cpu.
 * Shall clobber: x0-x17
 * --------------------------------------------------
 */
func errata_hercules_1688305_wa
	/* Compare x0 against revision r1p0 */
	mov	x17, x30
	bl	check_errata_1688305
	cbz	x0, 1f
	mrs     x1, CORTEX_HERCULES_ACTLR2_EL1
	orr	x1, x1, CORTEX_HERCULES_ACTLR2_EL1_BIT_1
	msr     CORTEX_HERCULES_ACTLR2_EL1, x1
	isb
1:
	ret	x17
endfunc errata_hercules_1688305_wa

func check_errata_1688305
	/* Applies to r0p0 and r1p0 */
	mov	x1, #0x10
	b	cpu_rev_var_ls
endfunc check_errata_1688305

	/* -------------------------------------------------
	 * The CPU Ops reset function for Cortex-Hercules
	 * -------------------------------------------------
	 */
func cortex_hercules_reset_func
	mov	x19, x30
	bl	cpu_get_rev_var
	mov	x18, x0

#if ERRATA_HERCULES_1688305
	mov     x0, x18
	bl	errata_hercules_1688305_wa
#endif

#if ENABLE_AMU
	/* Make sure accesses from EL0/EL1 and EL2 are not trapped to EL3 */
	mrs	x0, actlr_el3
	bic	x0, x0, #CORTEX_HERCULES_ACTLR_TAM_BIT
	msr	actlr_el3, x0

	/* Make sure accesses from non-secure EL0/EL1 are not trapped to EL2 */
	mrs	x0, actlr_el2
	bic	x0, x0, #CORTEX_HERCULES_ACTLR_TAM_BIT
	msr	actlr_el2, x0

	/* Enable group0 counters */
	mov	x0, #CORTEX_HERCULES_AMU_GROUP0_MASK
	msr	CPUAMCNTENSET0_EL0, x0

	/* Enable group1 counters */
	mov	x0, #CORTEX_HERCULES_AMU_GROUP1_MASK
	msr	CPUAMCNTENSET1_EL0, x0
#endif

	isb
	ret	x19
endfunc cortex_hercules_reset_func

	/* ---------------------------------------------
	 * HW will do the cache maintenance while powering down
	 * ---------------------------------------------
	 */
func cortex_hercules_core_pwr_dwn
	/* ---------------------------------------------
	 * Enable CPU power down bit in power control register
	 * ---------------------------------------------
	 */
	mrs	x0, CORTEX_HERCULES_CPUPWRCTLR_EL1
	orr	x0, x0, #CORTEX_HERCULES_CPUPWRCTLR_EL1_CORE_PWRDN_EN_BIT
	msr	CORTEX_HERCULES_CPUPWRCTLR_EL1, x0
	isb
	ret
endfunc cortex_hercules_core_pwr_dwn

	/*
	 * Errata printing function for cortex_hercules. Must follow AAPCS.
	 */
#if REPORT_ERRATA
func cortex_hercules_errata_report
	stp	x8, x30, [sp, #-16]!

	bl	cpu_get_rev_var
	mov	x8, x0

	/*
	 * Report all errata. The revision-variant information is passed to
	 * checking functions of each errata.
	 */
	report_errata ERRATA_HERCULES_1688305, cortex_hercules, 1688305

	ldp	x8, x30, [sp], #16
	ret
endfunc cortex_hercules_errata_report
#endif

	/* ---------------------------------------------
	 * This function provides cortex_hercules specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.cortex_hercules_regs, "aS"
cortex_hercules_regs:  /* The ascii list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func cortex_hercules_cpu_reg_dump
	adr	x6, cortex_hercules_regs
	mrs	x8, CORTEX_HERCULES_CPUECTLR_EL1
	ret
endfunc cortex_hercules_cpu_reg_dump

declare_cpu_ops cortex_hercules, CORTEX_HERCULES_MIDR, \
	cortex_hercules_reset_func, \
	cortex_hercules_core_pwr_dwn
