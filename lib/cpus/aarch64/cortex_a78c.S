/*
 * Copyright (c) 2021-2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <cortex_a78c.h>
#include <cpu_macros.S>
#include <plat_macros.S>
#include "wa_cve_2022_23960_bhb_vector.S"

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "cortex_a78c must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

/* --------------------------------------------------
 * Errata Workaround for Cortex A78C Erratum 2376749.
 * This applies to revision r0p1 and r0p2 of the A78C
 * and is currently open. It is a Cat B erratum.
 * Inputs:
 * x0: variant[4:7] and revision[0:3] of current cpu.
 * Shall clobber: x0-x4, x17
 * --------------------------------------------------
 */
func errata_a78c_2376749_wa
	/* Check revision */
	mov	x17, x30
	bl	check_errata_2376749
	cbz	x0, 1f
	/* Set CPUACTLR2_EL1[0] to 1. */
	mrs	x1, CORTEX_A78C_CPUACTLR2_EL1
	orr	x1, x1, #CORTEX_A78C_CPUACTLR2_EL1_BIT_0
	msr	CORTEX_A78C_CPUACTLR2_EL1, x1
1:
	ret	x17
endfunc errata_a78c_2376749_wa

func check_errata_2376749
	/* Applies to r0p1 and r0p2*/
	mov	x1, #0x01
	mov	x2, #0x02
	b	cpu_rev_var_range
endfunc check_errata_2376749

/* --------------------------------------------------
 * Errata Workaround for Cortex A78C Erratum 2395411.
 * This applies to revision r0p1 and r0p2 of the A78C
 * and is currently open. It is a Cat B erratum.
 * Inputs:
 * x0: variant[4:7] and revision[0:3] of current cpu.
 * Shall clobber: x0-x4, x17
 * --------------------------------------------------
 */
func errata_a78c_2395411_wa
	/* Check revision. */
	mov 	x17, x30
	bl 	check_errata_2395411
	cbz 	x0, 1f

	/* Set CPUACTRL2_EL1[40] to 1. */
	mrs 	x1, CORTEX_A78C_CPUACTLR2_EL1
	orr 	x1, x1, #CORTEX_A78C_CPUACTLR2_EL1_BIT_40
	msr 	CORTEX_A78C_CPUACTLR2_EL1, x1
1:
	ret 	x17
endfunc errata_a78c_2395411_wa

func check_errata_2395411
	/* Applies to r0p1 and r0p2 */
	mov 	x1, #0x01
	mov 	x2, #0x02
	b 	cpu_rev_var_range
endfunc check_errata_2395411

#if WORKAROUND_CVE_2022_23960
	wa_cve_2022_23960_bhb_vector_table CORTEX_A78C_BHB_LOOP_COUNT, cortex_a78c
#endif /* WORKAROUND_CVE_2022_23960 */

/* --------------------------------------------------
 * Errata Workaround for A78C Erratum 2132064.
 * This applies to revisions r0p1 and r0p2 of A78C
 * and is still open.
 * Inputs:
 * x0: variant[4:7] and revision[0:3] of current cpu.
 * Shall clobber: x0-x17
 * --------------------------------------------------
 */
func errata_a78c_2132064_wa
	/* Compare x0 against revisions r0p0 - r0p1 */
	mov	x17, x30
	bl	check_errata_2132064
	cbz	x0, 1f

	/* --------------------------------------------------------
	 * Place the data prefetcher in the most conservative mode
	 * to reduce prefetches by writing the following bits to
	 * the value indicated: ecltr[7:6], PF_MODE = 2'b11
	 * --------------------------------------------------------
	 */
	mrs	x0, CORTEX_A78C_CPUECTLR_EL1
	orr	x0, x0, #CORTEX_A78C_CPUECTLR_EL1_BIT_6
	orr	x0, x0, #CORTEX_A78C_CPUECTLR_EL1_BIT_7
	msr	CORTEX_A78C_CPUECTLR_EL1, x0
	isb
1:
	ret	x17
endfunc errata_a78c_2132064_wa

func check_errata_2132064
	/* Applies to revisions r0p1 and r0p2. */
	mov	x1, #CPU_REV(0, 1)
	mov	x2, #CPU_REV(0, 2)
	b	cpu_rev_var_range
endfunc check_errata_2132064

/* --------------------------------------------------------------------
 * Errata Workaround for A78C Erratum 2242638.
 * This applies to revisions r0p1 and r0p2 of the Cortex A78C
 * processor and is still open.
 * x0: variant[4:7] and revision[0:3] of current cpu.
 * Shall clobber: x0-x17
 * --------------------------------------------------------------------
 */
func errata_a78c_2242638_wa
	/* Compare x0 against revisions r0p1 - r0p2 */
	mov	x17, x30
	bl	check_errata_2242638
	cbz	x0, 1f

	ldr	x0, =0x5
	msr	CORTEX_A78C_IMP_CPUPSELR_EL3, x0
	ldr	x0, =0x10F600E000
	msr	CORTEX_A78C_IMP_CPUPOR_EL3, x0
	ldr	x0, =0x10FF80E000
	msr	CORTEX_A78C_IMP_CPUPMR_EL3, x0
	ldr	x0, =0x80000000003FF
	msr	CORTEX_A78C_IMP_CPUPCR_EL3, x0

	isb
1:
	ret	x17
endfunc errata_a78c_2242638_wa

func check_errata_2242638
	/* Applies to revisions r0p1-r0p2. */
	mov	x1, #CPU_REV(0, 1)
	mov	x2, #CPU_REV(0, 2)
	b	cpu_rev_var_range
endfunc check_errata_2242638

func check_errata_cve_2022_23960
#if WORKAROUND_CVE_2022_23960
	mov	x0, #ERRATA_APPLIES
#else
	mov	x0, #ERRATA_MISSING
#endif
	ret
endfunc check_errata_cve_2022_23960

	/* -------------------------------------------------
	 * The CPU Ops reset function for Cortex-A78C
	 * -------------------------------------------------
	 */
func cortex_a78c_reset_func
	mov	x19, x30
	bl	cpu_get_rev_var
	mov	x18, x0

#if ERRATA_A78C_2132064
	mov	x0, x18
	bl	errata_a78c_2132064_wa
#endif

#if ERRATA_A78C_2242638
	mov	x0, x18
	bl	errata_a78c_2242638_wa
#endif

#if ERRATA_A78C_2376749
	mov	x0, x18
	bl	errata_a78c_2376749_wa
#endif

#if ERRATA_A78C_2395411
	mov 	x0, x18
	bl	errata_a78c_2395411_wa
#endif

#if IMAGE_BL31 && WORKAROUND_CVE_2022_23960
	/*
	 * The Cortex-A78c generic vectors are overridden to apply errata
	 * mitigation on exception entry from lower ELs.
	 */
	adr	x0, wa_cve_vbar_cortex_a78c
	msr	vbar_el3, x0
#endif /* IMAGE_BL31 && WORKAROUND_CVE_2022_23960 */

	isb
	ret	x19
endfunc cortex_a78c_reset_func

	/* ----------------------------------------------------
	 * HW will do the cache maintenance while powering down
	 * ----------------------------------------------------
	 */
func cortex_a78c_core_pwr_dwn
	/* ---------------------------------------------------
	 * Enable CPU power down bit in power control register
	 * ---------------------------------------------------
	 */
	mrs	x0, CORTEX_A78C_CPUPWRCTLR_EL1
	orr	x0, x0, #CORTEX_A78C_CPUPWRCTLR_EL1_CORE_PWRDN_EN_BIT
	msr	CORTEX_A78C_CPUPWRCTLR_EL1, x0
	isb
	ret
endfunc cortex_a78c_core_pwr_dwn

#if REPORT_ERRATA
/*
 * Errata printing function for Cortex A78C. Must follow AAPCS.
 */
func cortex_a78c_errata_report
	stp	x8, x30, [sp, #-16]!

	bl	cpu_get_rev_var
	mov	x8, x0

	/*
	 * Report all errata. The revision-variant information is passed to
	 * checking functions of each errata.
	 */
	report_errata ERRATA_A78C_2132064, cortex_a78c, 2132064
	report_errata ERRATA_A78C_2242638, cortex_a78c, 2242638
	report_errata ERRATA_A78C_2376749, cortex_a78c, 2376749
	report_errata ERRATA_A78C_2395411, cortex_a78c, 2395411
	report_errata WORKAROUND_CVE_2022_23960, cortex_a78c, cve_2022_23960

	ldp	x8, x30, [sp], #16
        ret
endfunc cortex_a78c_errata_report
#endif

	/* ---------------------------------------------
	 * This function provides cortex_a78c specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.cortex_a78c_regs, "aS"
cortex_a78c_regs:  /* The ascii list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func cortex_a78c_cpu_reg_dump
	adr	x6, cortex_a78c_regs
	mrs	x8, CORTEX_A78C_CPUECTLR_EL1
	ret
endfunc cortex_a78c_cpu_reg_dump

declare_cpu_ops cortex_a78c, CORTEX_A78C_MIDR, \
	cortex_a78c_reset_func, \
	cortex_a78c_core_pwr_dwn
