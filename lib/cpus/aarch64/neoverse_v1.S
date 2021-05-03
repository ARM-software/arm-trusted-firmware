/*
 * Copyright (c) 2019-2020, ARM Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <neoverse_v1.h>
#include <cpu_macros.S>
#include <plat_macros.S>

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "Neoverse V1 must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

/* 64-bit only core */
#if CTX_INCLUDE_AARCH32_REGS == 1
#error "Neoverse-V1 supports only AArch64. Compile with CTX_INCLUDE_AARCH32_REGS=0"
#endif

	/* --------------------------------------------------
	 * Errata Workaround for Neoverse V1 Errata #1791573.
	 * This applies to revisions r0p0 and r1p0, fixed in r1p1.
	 * x0: variant[4:7] and revision[0:3] of current cpu.
	 * Shall clobber: x0-x17
	 * --------------------------------------------------
	 */
func errata_neoverse_v1_1791573_wa
	/* Check workaround compatibility. */
	mov	x17, x30
	bl	check_errata_1791573
	cbz	x0, 1f

	/* Set bit 2 in ACTLR2_EL1 */
	mrs     x1, NEOVERSE_V1_ACTLR2_EL1
	orr	x1, x1, #NEOVERSE_V1_ACTLR2_EL1_BIT_2
	msr     NEOVERSE_V1_ACTLR2_EL1, x1
	isb
1:
	ret	x17
endfunc errata_neoverse_v1_1791573_wa

func check_errata_1791573
	/* Applies to r0p0 and r1p0. */
	mov	x1, #0x10
	b	cpu_rev_var_ls
endfunc check_errata_1791573

	/* ---------------------------------------------
	 * HW will do the cache maintenance while powering down
	 * ---------------------------------------------
	 */
func neoverse_v1_core_pwr_dwn
	/* ---------------------------------------------
	 * Enable CPU power down bit in power control register
	 * ---------------------------------------------
	 */
	mrs	x0, NEOVERSE_V1_CPUPWRCTLR_EL1
	orr	x0, x0, #NEOVERSE_V1_CPUPWRCTLR_EL1_CORE_PWRDN_BIT
	msr	NEOVERSE_V1_CPUPWRCTLR_EL1, x0
	isb
	ret
endfunc neoverse_v1_core_pwr_dwn

	/*
	 * Errata printing function for Neoverse V1. Must follow AAPCS.
	 */
#if REPORT_ERRATA
func neoverse_v1_errata_report
	stp	x8, x30, [sp, #-16]!

	bl	cpu_get_rev_var
	mov	x8, x0

	/*
	 * Report all errata. The revision-variant information is passed to
	 * checking functions of each errata.
	 */
	report_errata ERRATA_V1_1791573, neoverse_v1, 1791573

	ldp	x8, x30, [sp], #16
	ret
endfunc neoverse_v1_errata_report
#endif

func neoverse_v1_reset_func
	mov	x19, x30

	/* Disable speculative loads */
	msr	SSBS, xzr
	isb

#if ERRATA_V1_1791573
	mov	x0, x18
	bl	errata_neoverse_v1_1791573_wa
#endif

	ret	x19
endfunc neoverse_v1_reset_func

	/* ---------------------------------------------
	 * This function provides Neoverse-V1 specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.neoverse_v1_regs, "aS"
neoverse_v1_regs:  /* The ascii list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func neoverse_v1_cpu_reg_dump
	adr	x6, neoverse_v1_regs
	mrs	x8, NEOVERSE_V1_CPUECTLR_EL1
	ret
endfunc neoverse_v1_cpu_reg_dump

declare_cpu_ops neoverse_v1, NEOVERSE_V1_MIDR, \
	neoverse_v1_reset_func, \
	neoverse_v1_core_pwr_dwn
