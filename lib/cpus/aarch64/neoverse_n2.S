/*
 * Copyright (c) 2020-2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <cpu_macros.S>
#include <neoverse_n2.h>

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "Neoverse N2 must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

/* 64-bit only core */
#if CTX_INCLUDE_AARCH32_REGS == 1
#error "Neoverse-N2 supports only AArch64. Compile with CTX_INCLUDE_AARCH32_REGS=0"
#endif

/* --------------------------------------------------
 * Errata Workaround for Neoverse N2 Erratum 2002655.
 * This applies to revision r0p0 of Neoverse N2. it is still open.
 * Inputs:
 * x0: variant[4:7] and revision[0:3] of current cpu.
 * Shall clobber: x0-x17
 * --------------------------------------------------
 */
func errata_n2_2002655_wa
	/* Check revision. */
	mov	x17, x30
	bl	check_errata_2002655
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
endfunc errata_n2_2002655_wa

func check_errata_2002655
	/* Applies to r0p0 */
	mov	x1, #0x00
	b	cpu_rev_var_ls
endfunc check_errata_2002655

/* ---------------------------------------------------------------
 * Errata Workaround for Neoverse N2 Erratum 2067956.
 * This applies to revision r0p0 of Neoverse N2 and is still open.
 * Inputs:
 * x0: variant[4:7] and revision[0:3] of current cpu.
 * Shall clobber: x0-x17
 * ---------------------------------------------------------------
 */
func errata_n2_2067956_wa
	/* Compare x0 against revision r0p0 */
	mov	x17, x30
	bl	check_errata_2067956
	cbz	x0, 1f
	mrs	x1, NEOVERSE_N2_CPUACTLR_EL1
	orr	x1, x1, NEOVERSE_N2_CPUACTLR_EL1_BIT_46
	msr	NEOVERSE_N2_CPUACTLR_EL1, x1
1:
	ret	x17
endfunc errata_n2_2067956_wa

func check_errata_2067956
	/* Applies to r0p0 */
	mov	x1, #0x00
	b	cpu_rev_var_ls
endfunc check_errata_2067956

/* ---------------------------------------------------------------
 * Errata Workaround for Neoverse N2 Erratum 2025414.
 * This applies to revision r0p0 of Neoverse N2 and is still open.
 * Inputs:
 * x0: variant[4:7] and revision[0:3] of current cpu.
 * Shall clobber: x0-x17
 * ---------------------------------------------------------------
 */
func errata_n2_2025414_wa
	/* Compare x0 against revision r0p0 */
	mov     x17, x30
	bl      check_errata_2025414
	cbz     x0, 1f
	mrs     x1, NEOVERSE_N2_CPUECTLR_EL1
	orr     x1, x1, NEOVERSE_N2_CPUECTLR_EL1_PFSTIDIS_BIT
	msr     NEOVERSE_N2_CPUECTLR_EL1, x1

1:
	ret     x17
endfunc errata_n2_2025414_wa

func check_errata_2025414
	/* Applies to r0p0 */
	mov     x1, #0x00
	b       cpu_rev_var_ls
endfunc check_errata_2025414

/* ---------------------------------------------------------------
 * Errata Workaround for Neoverse N2 Erratum 2189731.
 * This applies to revision r0p0 of Neoverse N2 and is still open.
 * Inputs:
 * x0: variant[4:7] and revision[0:3] of current cpu.
 * Shall clobber: x0-x17
 * ---------------------------------------------------------------
 */
func errata_n2_2189731_wa
	/* Compare x0 against revision r0p0 */
	mov     x17, x30
	bl      check_errata_2189731
	cbz     x0, 1f
	mrs     x1, NEOVERSE_N2_CPUACTLR5_EL1
	orr     x1, x1, NEOVERSE_N2_CPUACTLR5_EL1_BIT_44
	msr     NEOVERSE_N2_CPUACTLR5_EL1, x1

1:
	ret     x17
endfunc errata_n2_2189731_wa

func check_errata_2189731
	/* Applies to r0p0 */
	mov     x1, #0x00
	b       cpu_rev_var_ls
endfunc check_errata_2189731

/* --------------------------------------------------
 * Errata Workaround for Neoverse N2 Erratum 2138956.
 * This applies to revision r0p0 of Neoverse N2. it is still open.
 * Inputs:
 * x0: variant[4:7] and revision[0:3] of current cpu.
 * Shall clobber: x0-x17
 * --------------------------------------------------
 */
func errata_n2_2138956_wa
	/* Check revision. */
	mov	x17, x30
	bl	check_errata_2138956
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
endfunc errata_n2_2138956_wa

func check_errata_2138956
	/* Applies to r0p0 */
	mov	x1, #0x00
	b	cpu_rev_var_ls
endfunc check_errata_2138956

/* --------------------------------------------------
 * Errata Workaround for Neoverse N2 Erratum 2242415.
 * This applies to revision r0p0 of Neoverse N2. it is still open.
 * Inputs:
 * x0: variant[4:7] and revision[0:3] of current cpu.
 * Shall clobber: x0-x1, x17
 * --------------------------------------------------
 */
func errata_n2_2242415_wa
	/* Check revision. */
	mov	x17, x30
	bl	check_errata_2242415
	cbz	x0, 1f

	/* Apply instruction patching sequence */
	mrs	x1, NEOVERSE_N2_CPUACTLR_EL1
	orr	x1, x1, NEOVERSE_N2_CPUACTLR_EL1_BIT_22
	msr	NEOVERSE_N2_CPUACTLR_EL1, x1
1:
	ret	x17
endfunc errata_n2_2242415_wa

func check_errata_2242415
	/* Applies to r0p0 */
	mov	x1, #0x00
	b	cpu_rev_var_ls
endfunc check_errata_2242415

/* --------------------------------------------------
 * Errata Workaround for Neoverse N2 Erratum 2138953.
 * This applies to revision r0p0 of Neoverse N2. it is still open.
 * Inputs:
 * x0: variant[4:7] and revision[0:3] of current cpu.
 * Shall clobber: x0-x1, x17
 * --------------------------------------------------
 */
func errata_n2_2138953_wa
	/* Check revision. */
	mov	x17, x30
	bl	check_errata_2138953
	cbz	x0, 1f

	/* Apply instruction patching sequence */
	mrs	x1, NEOVERSE_N2_CPUECTLR2_EL1
	mov	x0, #NEOVERSE_N2_CPUECTLR2_EL1_PF_MODE_CNSRV
	bfi	x1, x0, #CPUECTLR2_EL1_PF_MODE_LSB, #CPUECTLR2_EL1_PF_MODE_WIDTH
	msr	NEOVERSE_N2_CPUECTLR2_EL1, x1
1:
	ret	x17
endfunc errata_n2_2138953_wa

func check_errata_2138953
	/* Applies to r0p0 */
	mov	x1, #0x00
	b	cpu_rev_var_ls
endfunc check_errata_2138953

/* --------------------------------------------------
 * Errata Workaround for Neoverse N2 Erratum 2138958.
 * This applies to revision r0p0 of Neoverse N2. it is still open.
 * Inputs:
 * x0: variant[4:7] and revision[0:3] of current cpu.
 * Shall clobber: x0-x1, x17
 * --------------------------------------------------
 */
func errata_n2_2138958_wa
	/* Check revision. */
	mov	x17, x30
	bl	check_errata_2138958
	cbz	x0, 1f

	/* Apply instruction patching sequence */
	mrs	x1, NEOVERSE_N2_CPUACTLR5_EL1
	orr	x1, x1, NEOVERSE_N2_CPUACTLR5_EL1_BIT_13
	msr	NEOVERSE_N2_CPUACTLR5_EL1, x1
1:
	ret	x17
endfunc errata_n2_2138958_wa

func check_errata_2138958
	/* Applies to r0p0 */
	mov	x1, #0x00
	b	cpu_rev_var_ls
endfunc check_errata_2138958

/* --------------------------------------------------
 * Errata Workaround for Neoverse N2 Erratum 2242400.
 * This applies to revision r0p0 of Neoverse N2. it is still open.
 * Inputs:
 * x0: variant[4:7] and revision[0:3] of current cpu.
 * Shall clobber: x0-x1, x17
 * --------------------------------------------------
 */
func errata_n2_2242400_wa
	/* Check revision. */
	mov	x17, x30
	bl	check_errata_2242400
	cbz	x0, 1f

	/* Apply instruction patching sequence */
	mrs	x1, NEOVERSE_N2_CPUACTLR5_EL1
	orr	x1, x1, NEOVERSE_N2_CPUACTLR5_EL1_BIT_17
	msr	NEOVERSE_N2_CPUACTLR5_EL1, x1
	ldr	x0, =0x2
	msr	S3_6_c15_c8_0, x0
	ldr	x0, =0x10F600E000
	msr	S3_6_c15_c8_2, x0
	ldr	x0, =0x10FF80E000
	msr	S3_6_c15_c8_3, x0
	ldr	x0, =0x80000000003FF
	msr	S3_6_c15_c8_1, x0
	isb
1:
	ret	x17
endfunc errata_n2_2242400_wa

func check_errata_2242400
	/* Applies to r0p0 */
	mov	x1, #0x00
	b	cpu_rev_var_ls
endfunc check_errata_2242400

/* --------------------------------------------------
 * Errata Workaround for Neoverse N2 Erratum 2280757.
 * This applies to revision r0p0 of Neoverse N2. it is still open.
 * Inputs:
 * x0: variant[4:7] and revision[0:3] of current cpu.
 * Shall clobber: x0-x1, x17
 * --------------------------------------------------
 */
func errata_n2_2280757_wa
	/* Check revision. */
	mov	x17, x30
	bl	check_errata_2280757
	cbz	x0, 1f

	/* Apply instruction patching sequence */
	mrs	x1, NEOVERSE_N2_CPUACTLR_EL1
	orr	x1, x1, NEOVERSE_N2_CPUACTLR_EL1_BIT_22
	msr	NEOVERSE_N2_CPUACTLR_EL1, x1
1:
	ret	x17
endfunc errata_n2_2280757_wa

func check_errata_2280757
	/* Applies to r0p0 */
	mov	x1, #0x00
	b	cpu_rev_var_ls
endfunc check_errata_2280757

	/* -------------------------------------------
	 * The CPU Ops reset function for Neoverse N2.
	 * -------------------------------------------
	 */
func neoverse_n2_reset_func
	mov	x19, x30

	/* Check if the PE implements SSBS */
	mrs	x0, id_aa64pfr1_el1
	tst	x0, #(ID_AA64PFR1_EL1_SSBS_MASK << ID_AA64PFR1_EL1_SSBS_SHIFT)
	b.eq	1f

	/* Disable speculative loads */
	msr	SSBS, xzr
1:
	/* Force all cacheable atomic instructions to be near */
	mrs	x0, NEOVERSE_N2_CPUACTLR2_EL1
	orr	x0, x0, #NEOVERSE_N2_CPUACTLR2_EL1_BIT_2
	msr	NEOVERSE_N2_CPUACTLR2_EL1, x0

#if ERRATA_N2_2067956
	mov	x0, x18
	bl	errata_n2_2067956_wa
#endif

#if ERRATA_N2_2025414
	mov	x0, x18
	bl	errata_n2_2025414_wa
#endif

#if ERRATA_N2_2189731
	mov	x0, x18
	bl	errata_n2_2189731_wa
#endif


#if ERRATA_N2_2138956
	mov	x0, x18
	bl	errata_n2_2138956_wa
#endif

#if ERRATA_N2_2138953
	mov	x0, x18
	bl	errata_n2_2138953_wa
#endif

#if ERRATA_N2_2242415
	mov	x0, x18
	bl	errata_n2_2242415_wa
#endif

#if ERRATA_N2_2138958
	mov	x0, x18
	bl	errata_n2_2138958_wa
#endif

#if ERRATA_N2_2242400
	mov	x0, x18
	bl	errata_n2_2242400_wa
#endif

#if ERRATA_N2_2280757
	mov	x0, x18
	bl	errata_n2_2280757_wa
#endif

#if ENABLE_AMU
	/* Make sure accesses from EL0/EL1 and EL2 are not trapped to EL3 */
	mrs	x0, cptr_el3
	orr	x0, x0, #TAM_BIT
	msr	cptr_el3, x0

	/* Make sure accesses from EL0/EL1 are not trapped to EL2 */
	mrs	x0, cptr_el2
	orr	x0, x0, #TAM_BIT
	msr	cptr_el2, x0

	/* No need to enable the counters as this would be done at el3 exit */
#endif

#if NEOVERSE_Nx_EXTERNAL_LLC
	/* Some systems may have External LLC, core needs to be made aware */
	mrs	x0, NEOVERSE_N2_CPUECTLR_EL1
	orr	x0, x0, NEOVERSE_N2_CPUECTLR_EL1_EXTLLC_BIT
	msr	NEOVERSE_N2_CPUECTLR_EL1, x0
#endif

	bl	cpu_get_rev_var
	mov	x18, x0

#if ERRATA_N2_2002655
	mov	x0, x18
	bl	errata_n2_2002655_wa
#endif

	isb
	ret	x19
endfunc neoverse_n2_reset_func

func neoverse_n2_core_pwr_dwn
	/* ---------------------------------------------------
	 * Enable CPU power down bit in power control register
	 * No need to do cache maintenance here.
	 * ---------------------------------------------------
	 */
	mrs	x0, NEOVERSE_N2_CPUPWRCTLR_EL1
	orr	x0, x0, #NEOVERSE_N2_CORE_PWRDN_EN_BIT
	msr	NEOVERSE_N2_CPUPWRCTLR_EL1, x0
	isb
	ret
endfunc neoverse_n2_core_pwr_dwn

#if REPORT_ERRATA
/*
 * Errata printing function for Neoverse N2 cores. Must follow AAPCS.
 */
func neoverse_n2_errata_report
	stp	x8, x30, [sp, #-16]!

	bl	cpu_get_rev_var
	mov	x8, x0

	/*
	 * Report all errata. The revision-variant information is passed to
	 * checking functions of each errata.
	 */
	report_errata ERRATA_N2_2002655, neoverse_n2, 2002655
	report_errata ERRATA_N2_2067956, neoverse_n2, 2067956
	report_errata ERRATA_N2_2025414, neoverse_n2, 2025414
	report_errata ERRATA_N2_2189731, neoverse_n2, 2189731
	report_errata ERRATA_N2_2138956, neoverse_n2, 2138956
	report_errata ERRATA_N2_2138953, neoverse_n2, 2138953
	report_errata ERRATA_N2_2242415, neoverse_n2, 2242415
	report_errata ERRATA_N2_2138958, neoverse_n2, 2138958
	report_errata ERRATA_N2_2242400, neoverse_n2, 2242400
	report_errata ERRATA_N2_2280757, neoverse_n2, 2280757

	ldp	x8, x30, [sp], #16
	ret
endfunc neoverse_n2_errata_report
#endif

	/* ---------------------------------------------
	 * This function provides Neoverse N2 specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ASCII and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.neoverse_n2_regs, "aS"
neoverse_n2_regs:  /* The ASCII list of register names to be reported */
	.asciz	"cpupwrctlr_el1", ""

func neoverse_n2_cpu_reg_dump
	adr	x6, neoverse_n2_regs
	mrs	x8, NEOVERSE_N2_CPUPWRCTLR_EL1
	ret
endfunc neoverse_n2_cpu_reg_dump

declare_cpu_ops neoverse_n2, NEOVERSE_N2_MIDR, \
	neoverse_n2_reset_func, \
	neoverse_n2_core_pwr_dwn
