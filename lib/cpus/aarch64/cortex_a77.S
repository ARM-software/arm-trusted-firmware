/*
 * Copyright (c) 2018-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <cortex_a77.h>
#include <cpu_macros.S>
#include <plat_macros.S>

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "Cortex-A77 must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

/* 64-bit only core */
#if CTX_INCLUDE_AARCH32_REGS == 1
#error "Cortex-A77 supports only AArch64. Compile with CTX_INCLUDE_AARCH32_REGS=0"
#endif

	/* --------------------------------------------------
	 * Errata Workaround for Cortex A77 Errata #1508412.
	 * This applies only to revision <= r1p0 of Cortex A77.
	 * Inputs:
	 * x0: variant[4:7] and revision[0:3] of current cpu.
	 * Shall clobber: x0-x17
	 * --------------------------------------------------
	 */
func errata_a77_1508412_wa
	/*
	 * Compare x0 against revision r1p0
	 */
	mov	x17, x30
	bl	check_errata_1508412
	cbz	x0, 3f
	/*
	 * Compare x0 against revision r0p0
	 */
	bl	check_errata_1508412_0
	cbz	x0, 1f
	ldr	x0, =0x0
	msr	CORTEX_A77_CPUPSELR_EL3, x0
	ldr 	x0, =0x00E8400000
	msr	CORTEX_A77_CPUPOR_EL3, x0
	ldr	x0, =0x00FFE00000
	msr	CORTEX_A77_CPUPMR_EL3, x0
	ldr	x0, =0x4004003FF
	msr	CORTEX_A77_CPUPCR_EL3, x0
	ldr	x0, =0x1
	msr	CORTEX_A77_CPUPSELR_EL3, x0
	ldr	x0, =0x00E8C00040
	msr	CORTEX_A77_CPUPOR_EL3, x0
	ldr	x0, =0x00FFE00040
	msr	CORTEX_A77_CPUPMR_EL3, x0
	b	2f
1:
	ldr	x0, =0x0
	msr	CORTEX_A77_CPUPSELR_EL3, x0
	ldr	x0, =0x00E8400000
	msr	CORTEX_A77_CPUPOR_EL3, x0
	ldr	x0, =0x00FF600000
	msr	CORTEX_A77_CPUPMR_EL3, x0
	ldr	x0, =0x00E8E00080
	msr	CORTEX_A77_CPUPOR2_EL3, x0
	ldr	x0, =0x00FFE000C0
	msr	CORTEX_A77_CPUPMR2_EL3, x0
2:
	ldr	x0, =0x04004003FF
	msr	CORTEX_A77_CPUPCR_EL3, x0
	isb
3:
	ret	x17
endfunc errata_a77_1508412_wa

func check_errata_1508412
	mov	x1, #0x10
	b	cpu_rev_var_ls
endfunc check_errata_1508412

func check_errata_1508412_0
	mov	x1, #0x0
	b	cpu_rev_var_ls
endfunc check_errata_1508412_0

	/* --------------------------------------------------
	 * Errata Workaround for Cortex A77 Errata #1800714.
	 * This applies to revision <= r1p1 of Cortex A77.
	 * Inputs:
	 * x0: variant[4:7] and revision[0:3] of current cpu.
	 * Shall clobber: x0-x17
	 * --------------------------------------------------
	 */
func errata_a77_1800714_wa
	/* Compare x0 against revision <= r1p1 */
	mov	x17, x30
	bl	check_errata_1800714
	cbz	x0, 1f

	/* Disable allocation of splintered pages in the L2 TLB */
	mrs	x1, CORTEX_A77_CPUECTLR_EL1
	orr	x1, x1, CORTEX_A77_CPUECTLR_EL1_BIT_53
	msr	CORTEX_A77_CPUECTLR_EL1, x1
	isb
1:
	ret	x17
endfunc errata_a77_1800714_wa

func check_errata_1800714
	/* Applies to everything <= r1p1 */
	mov	x1, #0x11
	b	cpu_rev_var_ls
endfunc check_errata_1800714

	/* --------------------------------------------------
	 * Errata Workaround for Cortex A77 Errata #1925769.
	 * This applies to revision <= r1p1 of Cortex A77.
	 * Inputs:
	 * x0: variant[4:7] and revision[0:3] of current cpu.
	 * Shall clobber: x0-x17
	 * --------------------------------------------------
	 */
func errata_a77_1925769_wa
	/* Compare x0 against revision <= r1p1 */
	mov	x17, x30
	bl	check_errata_1925769
	cbz	x0, 1f

	/* Set bit 8 in ECTLR_EL1 */
	mrs	x1, CORTEX_A77_CPUECTLR_EL1
	orr	x1, x1, #CORTEX_A77_CPUECTLR_EL1_BIT_8
	msr	CORTEX_A77_CPUECTLR_EL1, x1
	isb
1:
	ret	x17
endfunc errata_a77_1925769_wa

func check_errata_1925769
	/* Applies to everything <= r1p1 */
	mov	x1, #0x11
	b	cpu_rev_var_ls
endfunc check_errata_1925769

	/* -------------------------------------------------
	 * The CPU Ops reset function for Cortex-A77.
	 * Shall clobber: x0-x19
	 * -------------------------------------------------
	 */
func cortex_a77_reset_func
	mov	x19, x30
	bl	cpu_get_rev_var
	mov	x18, x0

#if ERRATA_A77_1508412
	mov	x0, x18
	bl	errata_a77_1508412_wa
#endif

#if ERRATA_A77_1800714
	mov	x0, x18
	bl	errata_a77_1800714_wa
#endif

#if ERRATA_A77_1925769
	mov	x0, x18
	bl	errata_a77_1925769_wa
#endif

	ret	x19
endfunc cortex_a77_reset_func

	/* ---------------------------------------------
	 * HW will do the cache maintenance while powering down
	 * ---------------------------------------------
	 */
func cortex_a77_core_pwr_dwn
	/* ---------------------------------------------
	 * Enable CPU power down bit in power control register
	 * ---------------------------------------------
	 */
	mrs	x0, CORTEX_A77_CPUPWRCTLR_EL1
	orr	x0, x0, #CORTEX_A77_CPUPWRCTLR_EL1_CORE_PWRDN_BIT
	msr	CORTEX_A77_CPUPWRCTLR_EL1, x0
	isb
	ret
endfunc cortex_a77_core_pwr_dwn

#if REPORT_ERRATA
/*
 * Errata printing function for Cortex-A77. Must follow AAPCS.
 */
func cortex_a77_errata_report
	stp	x8, x30, [sp, #-16]!

	bl	cpu_get_rev_var
	mov	x8, x0

	/*
	 * Report all errata. The revision-variant information is passed to
	 * checking functions of each errata.
	 */
	report_errata ERRATA_A77_1508412, cortex_a77, 1508412
	report_errata ERRATA_A77_1800714, cortex_a77, 1800714
	report_errata ERRATA_A77_1925769, cortex_a77, 1925769

	ldp	x8, x30, [sp], #16
	ret
endfunc cortex_a77_errata_report
#endif


	/* ---------------------------------------------
	 * This function provides Cortex-A77 specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.cortex_a77_regs, "aS"
cortex_a77_regs:  /* The ascii list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func cortex_a77_cpu_reg_dump
	adr	x6, cortex_a77_regs
	mrs	x8, CORTEX_A77_CPUECTLR_EL1
	ret
endfunc cortex_a77_cpu_reg_dump

declare_cpu_ops cortex_a77, CORTEX_A77_MIDR, \
	cortex_a77_reset_func, \
	cortex_a77_core_pwr_dwn
