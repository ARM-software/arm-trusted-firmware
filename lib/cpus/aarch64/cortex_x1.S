/*
 * Copyright (c) 2022, Google LLC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <asm_macros.S>
#include <cortex_x1.h>
#include <cpu_macros.S>
#include "wa_cve_2022_23960_bhb_vector.S"

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "Cortex-X1 must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

/* 64-bit only core */
#if CTX_INCLUDE_AARCH32_REGS == 1
#error "Cortex-X1 supports only AArch64. Compile with CTX_INCLUDE_AARCH32_REGS=0"
#endif

#if WORKAROUND_CVE_2022_23960
	wa_cve_2022_23960_bhb_vector_table CORTEX_X1_BHB_LOOP_COUNT, cortex_x1
#endif /* WORKAROUND_CVE_2022_23960 */

/* --------------------------------------------------
 * Errata Workaround for X1 Erratum 1821534.
 * This applies to revision r0p0 and r1p0 of X1.
 * Inputs:
 * x0: variant[4:7] and revision[0:3] of current cpu.
 * Shall clobber: x0-x17
 * --------------------------------------------------
 */
func errata_x1_1821534_wa
	/* Compare x0 against revision r1p0 */
	mov	x17, x30
	bl	check_errata_1821534
	cbz	x0, 1f
	mrs	x1, CORTEX_X1_ACTLR2_EL1
	orr	x1, x1, BIT(2)
	msr	CORTEX_X1_ACTLR2_EL1, x1
	isb
1:
	ret	x17
endfunc errata_x1_1821534_wa

func check_errata_1821534
	/* Applies to r0p0 and r1p0 */
	mov	x1, #0x10
	b	cpu_rev_var_ls
endfunc check_errata_1821534

/* --------------------------------------------------
 * Errata Workaround for X1 Erratum 1688305.
 * This applies to revision r0p0 and r1p0 of X1.
 * Inputs:
 * x0: variant[4:7] and revision[0:3] of current cpu.
 * Shall clobber: x0-x17
 * --------------------------------------------------
 */
func errata_x1_1688305_wa
	/* Compare x0 against revision r1p0 */
	mov	x17, x30
	bl	check_errata_1688305
	cbz	x0, 1f
	mrs	x0, CORTEX_X1_ACTLR2_EL1
	orr	x0, x0, BIT(1)
	msr	CORTEX_X1_ACTLR2_EL1, x0
	isb

1:
	ret	x17
endfunc errata_x1_1688305_wa

func check_errata_1688305
	/* Applies to r0p0 and r1p0 */
	mov	x1, #0x10
	b	cpu_rev_var_ls
endfunc check_errata_1688305

/* --------------------------------------------------
 * Errata Workaround for X1 Erratum 1827429.
 * This applies to revision r0p0 and r1p0 of X1.
 * Inputs:
 * x0: variant[4:7] and revision[0:3] of current cpu.
 * Shall clobber: x0-x17
 * --------------------------------------------------
 */
func errata_x1_1827429_wa
	/* Compare x0 against revision r1p0 */
	mov	x17, x30
	bl	check_errata_1827429
	cbz	x0, 1f
	mrs	x0, CORTEX_X1_CPUECTLR_EL1
	orr	x0, x0, BIT(53)
	msr	CORTEX_X1_CPUECTLR_EL1, x0
	isb

1:
	ret	x17
endfunc errata_x1_1827429_wa

func check_errata_1827429
	/* Applies to r0p0 and r1p0 */
	mov	x1, #0x10
	b	cpu_rev_var_ls
endfunc check_errata_1827429

func check_errata_cve_2022_23960
#if WORKAROUND_CVE_2022_23960
	mov	x0, #ERRATA_APPLIES
#else
	mov	x0, #ERRATA_MISSING
#endif
	ret
endfunc check_errata_cve_2022_23960

	/* -------------------------------------------------
	 * The CPU Ops reset function for Cortex-X1.
	 * Shall clobber: x0-x19
	 * -------------------------------------------------
	 */
func cortex_x1_reset_func
	mov	x19, x30
	bl	cpu_get_rev_var
	mov	x18, x0

#if ERRATA_X1_1821534
	mov	x0, x18
	bl	errata_x1_1821534_wa
#endif

#if ERRATA_X1_1688305
	mov	x0, x18
	bl	errata_x1_1688305_wa
#endif

#if ERRATA_X1_1827429
	mov	x0, x18
	bl	errata_x1_1827429_wa
#endif

#if IMAGE_BL31 && WORKAROUND_CVE_2022_23960
	/*
	 * The Cortex-X1 generic vectors are overridden to apply errata
	 * mitigation on exception entry from lower ELs.
	 */
	adr	x0, wa_cve_vbar_cortex_x1
	msr	vbar_el3, x0
#endif /* IMAGE_BL31 && WORKAROUND_CVE_2022_23960 */

	isb
	ret	x19
endfunc cortex_x1_reset_func

	/* ---------------------------------------------
	 * HW will do the cache maintenance while powering down
	 * ---------------------------------------------
	 */
func cortex_x1_core_pwr_dwn
	/* ---------------------------------------------
	 * Enable CPU power down bit in power control register
	 * ---------------------------------------------
	 */
	mrs	x0, CORTEX_X1_CPUPWRCTLR_EL1
	orr	x0, x0, #CORTEX_X1_CORE_PWRDN_EN_MASK
	msr	CORTEX_X1_CPUPWRCTLR_EL1, x0
	isb
	ret
endfunc cortex_x1_core_pwr_dwn

#if REPORT_ERRATA
/*
 * Errata printing function for Cortex X1. Must follow AAPCS.
 */
func cortex_x1_errata_report
	stp	x8, x30, [sp, #-16]!

	bl	cpu_get_rev_var
	mov	x8, x0

	/*
	 * Report all errata. The revision-variant information is passed to
	 * checking functions of each errata.
	 */
	report_errata ERRATA_X1_1821534, cortex_x1, 1821534
	report_errata ERRATA_X1_1688305, cortex_x1, 1688305
	report_errata ERRATA_X1_1827429, cortex_x1, 1827429
	report_errata WORKAROUND_CVE_2022_23960, cortex_x1, cve_2022_23960

	ldp	x8, x30, [sp], #16
	ret
endfunc cortex_x1_errata_report
#endif

       /* ---------------------------------------------
	* This function provides Cortex X1 specific
	* register information for crash reporting.
	* It needs to return with x6 pointing to
	* a list of register names in ascii and
	* x8 - x15 having values of registers to be
	* reported.
	* ---------------------------------------------
	*/
.section .rodata.cortex_x1_regs, "aS"
cortex_x1_regs:  /* The ascii list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func cortex_x1_cpu_reg_dump
	adr	x6, cortex_x1_regs
	mrs	x8, CORTEX_X1_CPUECTLR_EL1
	ret
endfunc cortex_x1_cpu_reg_dump

declare_cpu_ops cortex_x1, CORTEX_X1_MIDR, \
	cortex_x1_reset_func, \
	cortex_x1_core_pwr_dwn
