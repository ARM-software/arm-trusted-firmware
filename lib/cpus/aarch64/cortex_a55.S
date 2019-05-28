/*
 * Copyright (c) 2017-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <cortex_a55.h>
#include <cpu_macros.S>
#include <plat_macros.S>

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "Cortex-A55 must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

	/* --------------------------------------------------
	 * Errata Workaround for Cortex A55 Errata #768277.
	 * This applies only to revision r0p0 of Cortex A55.
	 * Inputs:
	 * x0: variant[4:7] and revision[0:3] of current cpu.
	 * Shall clobber: x0-x17
	 * --------------------------------------------------
	 */
func errata_a55_768277_wa
	/*
	 * Compare x0 against revision r0p0
	 */
	mov	x17, x30
	bl	check_errata_768277
	cbz	x0, 1f
	mrs	x1, CORTEX_A55_CPUACTLR_EL1
	orr	x1, x1, #CORTEX_A55_CPUACTLR_EL1_DISABLE_DUAL_ISSUE
	msr	CORTEX_A55_CPUACTLR_EL1, x1
	isb
1:
	ret	x17
endfunc errata_a55_768277_wa

func check_errata_768277
	mov	x1, #0x00
	b	cpu_rev_var_ls
endfunc check_errata_768277

	/* ------------------------------------------------------------------
	 * Errata Workaround for Cortex A55 Errata #778703.
	 * This applies only to revision r0p0 of Cortex A55 where L2 cache is
	 * not configured.
	 * Inputs:
	 * x0: variant[4:7] and revision[0:3] of current cpu.
	 * Shall clobber: x0-x17
	 * ------------------------------------------------------------------
	 */
func errata_a55_778703_wa
	/*
	 * Compare x0 against revision r0p0 and check that no private L2 cache
	 * is configured
	 */
	mov	x17, x30
	bl	check_errata_778703
	cbz	x0, 1f
	mrs	x1, CORTEX_A55_CPUECTLR_EL1
	orr	x1, x1, #CORTEX_A55_CPUECTLR_EL1_L1WSCTL
	msr	CORTEX_A55_CPUECTLR_EL1, x1
	mrs	x1, CORTEX_A55_CPUACTLR_EL1
	orr	x1, x1, #CORTEX_A55_CPUACTLR_EL1_DISABLE_WRITE_STREAMING
	msr	CORTEX_A55_CPUACTLR_EL1, x1
	isb
1:
	ret	x17
endfunc errata_a55_778703_wa

func check_errata_778703
	mov	x16, x30
	mov	x1, #0x00
	bl	cpu_rev_var_ls
	/*
	 * Check that no private L2 cache is configured
	 */
	mrs	x1, CORTEX_A55_CLIDR_EL1
	and	x1, x1, CORTEX_A55_CLIDR_EL1_CTYPE3
	cmp	x1, #0
	mov	x2, #ERRATA_NOT_APPLIES
	csel	x0, x0, x2, eq
	ret	x16
endfunc check_errata_778703

	/* --------------------------------------------------
	 * Errata Workaround for Cortex A55 Errata #798797.
	 * This applies only to revision r0p0 of Cortex A55.
	 * Inputs:
	 * x0: variant[4:7] and revision[0:3] of current cpu.
	 * Shall clobber: x0-x17
	 * --------------------------------------------------
	 */
func errata_a55_798797_wa
	/*
	 * Compare x0 against revision r0p0
	 */
	mov	x17, x30
	bl	check_errata_798797
	cbz	x0, 1f
	mrs	x1, CORTEX_A55_CPUACTLR_EL1
	orr	x1, x1, #CORTEX_A55_CPUACTLR_EL1_DISABLE_L1_PAGEWALKS
	msr	CORTEX_A55_CPUACTLR_EL1, x1
	isb
1:
	ret	x17
endfunc errata_a55_798797_wa

func check_errata_798797
	mov	x1, #0x00
	b	cpu_rev_var_ls
endfunc check_errata_798797

	/* --------------------------------------------------------------------
	 * Errata Workaround for Cortex A55 Errata #846532.
	 * This applies only to revisions <= r0p1 of Cortex A55.
	 * Disabling dual-issue has a small impact on performance. Disabling a
	 * power optimization feature is an alternate workaround with no impact
	 * on performance but with an increase in power consumption (see errata
	 * notice).
	 * Inputs:
	 * x0: variant[4:7] and revision[0:3] of current cpu.
	 * Shall clobber: x0-x17
	 * --------------------------------------------------------------------
	 */
func errata_a55_846532_wa
	/*
	 * Compare x0 against revision r0p1
	 */
	mov	x17, x30
	bl	check_errata_846532
	cbz	x0, 1f
	mrs	x1, CORTEX_A55_CPUACTLR_EL1
	orr	x1, x1, #CORTEX_A55_CPUACTLR_EL1_DISABLE_DUAL_ISSUE
	msr	CORTEX_A55_CPUACTLR_EL1, x1
	isb
1:
	ret	x17
endfunc errata_a55_846532_wa

func check_errata_846532
	mov	x1, #0x01
	b	cpu_rev_var_ls
endfunc check_errata_846532

	/* -----------------------------------------------------
	 * Errata Workaround for Cortex A55 Errata #903758.
	 * This applies only to revisions <= r0p1 of Cortex A55.
	 * Inputs:
	 * x0: variant[4:7] and revision[0:3] of current cpu.
	 * Shall clobber: x0-x17
	 * -----------------------------------------------------
	 */
func errata_a55_903758_wa
	/*
	 * Compare x0 against revision r0p1
	 */
	mov	x17, x30
	bl	check_errata_903758
	cbz	x0, 1f
	mrs	x1, CORTEX_A55_CPUACTLR_EL1
	orr	x1, x1, #CORTEX_A55_CPUACTLR_EL1_DISABLE_L1_PAGEWALKS
	msr	CORTEX_A55_CPUACTLR_EL1, x1
	isb
1:
	ret	x17
endfunc errata_a55_903758_wa

func check_errata_903758
	mov	x1, #0x01
	b	cpu_rev_var_ls
endfunc check_errata_903758

	/* -----------------------------------------------------
	 * Errata Workaround for Cortex A55 Errata #1221012.
	 * This applies only to revisions <= r1p0 of Cortex A55.
	 * Inputs:
	 * x0: variant[4:7] and revision[0:3] of current cpu.
	 * Shall clobber: x0-x17
	 * -----------------------------------------------------
	 */
func errata_a55_1221012_wa
	/*
	 * Compare x0 against revision r1p0
	 */
	mov	x17, x30
	bl	check_errata_1221012
	cbz	x0, 1f
	mov	x0, #0x0020
	movk	x0, #0x0850, lsl #16
	msr	CPUPOR_EL3, x0
	mov	x0, #0x0000
	movk	x0, #0x1FF0, lsl #16
	movk	x0, #0x2, lsl #32
	msr	CPUPMR_EL3, x0
	mov	x0, #0x03fd
	movk	x0, #0x0110, lsl #16
	msr	CPUPCR_EL3, x0
	mov	x0, #0x1
	msr	CPUPSELR_EL3, x0
	mov	x0, #0x0040
	movk	x0, #0x08D0, lsl #16
	msr	CPUPOR_EL3, x0
	mov	x0, #0x0040
	movk	x0, #0x1FF0, lsl #16
	movk	x0, #0x2, lsl #32
	msr	CPUPMR_EL3, x0
	mov	x0, #0x03fd
	movk	x0, #0x0110, lsl #16
	msr	CPUPCR_EL3, x0
	isb
1:
	ret	x17
endfunc errata_a55_1221012_wa

func check_errata_1221012
	mov	x1, #0x10
	b	cpu_rev_var_ls
endfunc check_errata_1221012

func cortex_a55_reset_func
	mov	x19, x30

#if ERRATA_DSU_798953
	bl	errata_dsu_798953_wa
#endif

#if ERRATA_DSU_936184
	bl	errata_dsu_936184_wa
#endif

	bl	cpu_get_rev_var
	mov	x18, x0

#if ERRATA_A55_768277
	mov	x0, x18
	bl	errata_a55_768277_wa
#endif

#if ERRATA_A55_778703
	mov	x0, x18
	bl	errata_a55_778703_wa
#endif

#if ERRATA_A55_798797
	mov	x0, x18
	bl	errata_a55_798797_wa
#endif

#if ERRATA_A55_846532
	mov	x0, x18
	bl	errata_a55_846532_wa
#endif

#if ERRATA_A55_903758
	mov	x0, x18
	bl	errata_a55_903758_wa
#endif

#if ERRATA_A55_1221012
	mov	x0, x18
	bl	errata_a55_1221012_wa
#endif

	ret	x19
endfunc cortex_a55_reset_func

	/* ---------------------------------------------
	 * HW will do the cache maintenance while powering down
	 * ---------------------------------------------
	 */
func cortex_a55_core_pwr_dwn
	/* ---------------------------------------------
	 * Enable CPU power down bit in power control register
	 * ---------------------------------------------
	 */
	mrs	x0, CORTEX_A55_CPUPWRCTLR_EL1
	orr	x0, x0, #CORTEX_A55_CORE_PWRDN_EN_MASK
	msr	CORTEX_A55_CPUPWRCTLR_EL1, x0
	isb
	ret
endfunc cortex_a55_core_pwr_dwn

#if REPORT_ERRATA
/*
 * Errata printing function for Cortex A55. Must follow AAPCS & can use stack.
 */
func cortex_a55_errata_report
	stp	x8, x30, [sp, #-16]!
	bl	cpu_get_rev_var
	mov	x8, x0

	/*
	 * Report all errata. The revision variant information is at x8, where
	 * "report_errata" is expecting it and it doesn't corrupt it.
	 */
	report_errata ERRATA_DSU_798953, cortex_a55, dsu_798953
	report_errata ERRATA_DSU_936184, cortex_a55, dsu_936184
	report_errata ERRATA_A55_768277, cortex_a55, 768277
	report_errata ERRATA_A55_778703, cortex_a55, 778703
	report_errata ERRATA_A55_798797, cortex_a55, 798797
	report_errata ERRATA_A55_846532, cortex_a55, 846532
	report_errata ERRATA_A55_903758, cortex_a55, 903758
	report_errata ERRATA_A55_1221012, cortex_a55, 1221012

	ldp	x8, x30, [sp], #16
	ret
endfunc cortex_a55_errata_report
#endif

	/* ---------------------------------------------
	 * This function provides cortex_a55 specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.cortex_a55_regs, "aS"
cortex_a55_regs:  /* The ascii list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func cortex_a55_cpu_reg_dump
	adr	x6, cortex_a55_regs
	mrs	x8, CORTEX_A55_CPUECTLR_EL1
	ret
endfunc cortex_a55_cpu_reg_dump

declare_cpu_ops cortex_a55, CORTEX_A55_MIDR, \
	cortex_a55_reset_func, \
	cortex_a55_core_pwr_dwn
