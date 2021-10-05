/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <cortex_a710.h>
#include <cpu_macros.S>
#include <plat_macros.S>

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "Cortex A710 must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

/* 64-bit only core */
#if CTX_INCLUDE_AARCH32_REGS == 1
#error "Cortex A710 supports only AArch64. Compile with CTX_INCLUDE_AARCH32_REGS=0"
#endif

/* --------------------------------------------------
 * Errata Workaround for Cortex-A710 Erratum 1987031.
 * This applies to revision r0p0, r1p0 and r2p0 of Cortex-A710. It is still
 * open.
 * Inputs:
 * x0: variant[4:7] and revision[0:3] of current cpu.
 * Shall clobber: x0-x17
 * --------------------------------------------------
 */
func errata_a710_1987031_wa
	/* Check revision. */
	mov	x17, x30
	bl	check_errata_1987031
	cbz	x0, 1f

	/* Apply instruction patching sequence */
	ldr x0,=0x6
	msr S3_6_c15_c8_0,x0
	ldr x0,=0xF3A08002
	msr S3_6_c15_c8_2,x0
	ldr x0,=0xFFF0F7FE
	msr S3_6_c15_c8_3,x0
	ldr x0,=0x40000001003ff
	msr S3_6_c15_c8_1,x0
	ldr x0,=0x7
	msr S3_6_c15_c8_0,x0
	ldr x0,=0xBF200000
	msr S3_6_c15_c8_2,x0
	ldr x0,=0xFFEF0000
	msr S3_6_c15_c8_3,x0
	ldr x0,=0x40000001003f3
	msr S3_6_c15_c8_1,x0
	isb
1:
	ret	x17
endfunc errata_a710_1987031_wa

func check_errata_1987031
	/* Applies to r0p0, r1p0 and r2p0 */
	mov	x1, #0x20
	b	cpu_rev_var_ls
endfunc check_errata_1987031

/* --------------------------------------------------
 * Errata Workaround for Cortex-A710 Erratum 2081180.
 * This applies to revision r0p0, r1p0 and r2p0 of Cortex-A710.
 * It is still open.
 * Inputs:
 * x0: variant[4:7] and revision[0:3] of current cpu.
 * Shall clobber: x0-x17
 * --------------------------------------------------
 */
func errata_a710_2081180_wa
	/* Check revision. */
	mov	x17, x30
	bl	check_errata_2081180
	cbz	x0, 1f

	/* Apply instruction patching sequence */
	ldr	x0,=0x3
	msr	S3_6_c15_c8_0,x0
	ldr	x0,=0xF3A08002
	msr	S3_6_c15_c8_2,x0
	ldr	x0,=0xFFF0F7FE
	msr	S3_6_c15_c8_3,x0
	ldr	x0,=0x10002001003FF
	msr	S3_6_c15_c8_1,x0
	ldr	x0,=0x4
	msr	S3_6_c15_c8_0,x0
	ldr	x0,=0xBF200000
	msr	S3_6_c15_c8_2,x0
	ldr	x0,=0xFFEF0000
	msr	S3_6_c15_c8_3,x0
	ldr	x0,=0x10002001003F3
	msr	S3_6_c15_c8_1,x0
	isb
1:
	ret	x17
endfunc errata_a710_2081180_wa

func check_errata_2081180
	/* Applies to r0p0, r1p0 and r2p0 */
	mov	x1, #0x20
	b	cpu_rev_var_ls
endfunc check_errata_2081180

/* ---------------------------------------------------------------------
 * Errata Workaround for Cortex-A710 Erratum 2055002.
 * This applies to revision r1p0, r2p0 of Cortex-A710 and is still open.
 * Inputs:
 * x0: variant[4:7] and revision[0:3] of current cpu.
 * Shall clobber: x0-x17
 * ---------------------------------------------------------------------
 */
func errata_a710_2055002_wa
	/* Compare x0 against revision r2p0 */
	mov	x17, x30
	bl	check_errata_2055002
	cbz	x0, 1f
	mrs	x1, CORTEX_A710_CPUACTLR_EL1
	orr	x1, x1, CORTEX_A710_CPUACTLR_EL1_BIT_46
	msr	CORTEX_A710_CPUACTLR_EL1, x1
1:
	ret	x17
endfunc errata_a710_2055002_wa

func check_errata_2055002
	/* Applies to r1p0, r2p0 */
	mov	x1, #0x20
	b	cpu_rev_var_ls
endfunc check_errata_2055002

/* -------------------------------------------------------------
 * Errata Workaround for Cortex-A710 Erratum 2017096.
 * This applies to revisions r0p0, r1p0 and r2p0 of Cortex-A710.
 * Inputs:
 * x0: variant[4:7] and revision[0:3] of current cpu.
 * Shall clobber: x0-x17
 * -------------------------------------------------------------
 */
func errata_a710_2017096_wa
	/* Compare x0 against revision r0p0 to r2p0 */
	mov     x17, x30
	bl      check_errata_2017096
	cbz     x0, 1f
	mrs     x1, CORTEX_A710_CPUECTLR_EL1
	orr     x1, x1, CORTEX_A710_CPUECTLR_EL1_PFSTIDIS_BIT
	msr     CORTEX_A710_CPUECTLR_EL1, x1

1:
	ret     x17
endfunc errata_a710_2017096_wa

func check_errata_2017096
	/* Applies to r0p0, r1p0, r2p0 */
	mov     x1, #0x20
	b       cpu_rev_var_ls
endfunc check_errata_2017096


/* ---------------------------------------------------------------------
 * Errata Workaround for Cortex-A710 Erratum 2083908.
 * This applies to revision r2p0 of Cortex-A710 and is still open.
 * Inputs:
 * x0: variant[4:7] and revision[0:3] of current cpu.
 * Shall clobber: x0-x17
 * ---------------------------------------------------------------------
 */
func errata_a710_2083908_wa
	/* Compare x0 against revision r2p0 */
	mov	x17, x30
	bl	check_errata_2083908
	cbz	x0, 1f
	mrs	x1, CORTEX_A710_CPUACTLR5_EL1
	orr	x1, x1, CORTEX_A710_CPUACTLR5_EL1_BIT_13
	msr	CORTEX_A710_CPUACTLR5_EL1, x1
1:
	ret	x17
endfunc errata_a710_2083908_wa

func check_errata_2083908
	/* Applies to r2p0 */
	mov	x1, #CPU_REV(2, 0)
	mov	x2, #CPU_REV(2, 0)
	b	cpu_rev_var_range
endfunc check_errata_2083908

/* ---------------------------------------------------------------------
 * Errata Workaround for Cortex-A710 Erratum 2058056.
 * This applies to revisions r0p0, r1p0 and r2p0 of Cortex-A710 and is still
 * open.
 * Inputs:
 * x0: variant[4:7] and revision[0:3] of current cpu.
 * Shall clobber: x0-x17
 * ---------------------------------------------------------------------
 */
func errata_a710_2058056_wa
	/* Compare x0 against revision r2p0 */
	mov	x17, x30
	bl	check_errata_2058056
	cbz	x0, 1f
	mrs	x1, CORTEX_A710_CPUECTLR2_EL1
	mov	x0, #CORTEX_A710_CPUECTLR2_EL1_PF_MODE_CNSRV
	bfi	x1, x0, #CPUECTLR2_EL1_PF_MODE_LSB, #CPUECTLR2_EL1_PF_MODE_WIDTH
	msr	CORTEX_A710_CPUECTLR2_EL1, x1
1:
	ret	x17
endfunc errata_a710_2058056_wa

func check_errata_2058056
	/* Applies to r0p0, r1p0 and r2p0 */
	mov	x1, #0x20
	b	cpu_rev_var_ls
endfunc check_errata_2058056

	/* ----------------------------------------------------
	 * HW will do the cache maintenance while powering down
	 * ----------------------------------------------------
	 */
func cortex_a710_core_pwr_dwn
	/* ---------------------------------------------------
	 * Enable CPU power down bit in power control register
	 * ---------------------------------------------------
	 */
	mrs	x0, CORTEX_A710_CPUPWRCTLR_EL1
	orr	x0, x0, #CORTEX_A710_CPUPWRCTLR_EL1_CORE_PWRDN_BIT
	msr	CORTEX_A710_CPUPWRCTLR_EL1, x0
	isb
	ret
endfunc cortex_a710_core_pwr_dwn

#if REPORT_ERRATA
	/*
	 * Errata printing function for Cortex-A710. Must follow AAPCS.
	 */
func cortex_a710_errata_report
	stp	x8, x30, [sp, #-16]!

	bl	cpu_get_rev_var
	mov	x8, x0

	/*
	 * Report all errata. The revision-variant information is passed to
	 * checking functions of each errata.
	 */
	report_errata ERRATA_A710_1987031, cortex_a710, 1987031
	report_errata ERRATA_A710_2081180, cortex_a710, 2081180
	report_errata ERRATA_A710_2055002, cortex_a710, 2055002
	report_errata ERRATA_A710_2017096, cortex_a710, 2017096
	report_errata ERRATA_A710_2083908, cortex_a710, 2083908
	report_errata ERRATA_A710_2058056, cortex_a710, 2058056

	ldp	x8, x30, [sp], #16
	ret
endfunc cortex_a710_errata_report
#endif

func cortex_a710_reset_func
	mov	x19, x30

	/* Disable speculative loads */
	msr	SSBS, xzr

	bl	cpu_get_rev_var
	mov	x18, x0

#if ERRATA_A710_1987031
	mov	x0, x18
	bl	errata_a710_1987031_wa
#endif

#if ERRATA_A710_2081180
	mov	x0, x18
	bl	errata_a710_2081180_wa
#endif

#if ERRATA_A710_2055002
	mov	x0, x18
	bl	errata_a710_2055002_wa
#endif

#if ERRATA_A710_2017096
	mov	x0, x18
	bl	errata_a710_2017096_wa
#endif

#if ERRATA_A710_2083908
	mov	x0, x18
	bl	errata_a710_2083908_wa
#endif

#if ERRATA_A710_2058056
	mov	x0, x18
	bl	errata_a710_2058056_wa
#endif
	isb
	ret	x19
endfunc cortex_a710_reset_func

	/* ---------------------------------------------
	 * This function provides Cortex-A710 specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.cortex_a710_regs, "aS"
cortex_a710_regs:  /* The ascii list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func cortex_a710_cpu_reg_dump
	adr	x6, cortex_a710_regs
	mrs	x8, CORTEX_A710_CPUECTLR_EL1
	ret
endfunc cortex_a710_cpu_reg_dump

declare_cpu_ops cortex_a710, CORTEX_A710_MIDR, \
	cortex_a710_reset_func, \
	cortex_a710_core_pwr_dwn
