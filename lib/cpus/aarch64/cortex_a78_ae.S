/*
 * Copyright (c) 2019-2022, ARM Limited. All rights reserved.
 * Copyright (c) 2021-2022, NVIDIA Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <cortex_a78_ae.h>
#include <cpu_macros.S>
#include <plat_macros.S>
#include "wa_cve_2022_23960_bhb_vector.S"

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "cortex_a78_ae must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

#if WORKAROUND_CVE_2022_23960
	wa_cve_2022_23960_bhb_vector_table CORTEX_A78_AE_BHB_LOOP_COUNT, cortex_a78_ae
#endif /* WORKAROUND_CVE_2022_23960 */

/* --------------------------------------------------
 * Errata Workaround for A78 AE Erratum 1941500.
 * This applies to revisions r0p0 and r0p1 of A78 AE.
 * Inputs:
 * x0: variant[4:7] and revision[0:3] of current cpu.
 * Shall clobber: x0-x17
 * --------------------------------------------------
 */
func errata_a78_ae_1941500_wa
	/* Compare x0 against revisions r0p0 - r0p1 */
	mov	x17, x30
	bl	check_errata_1941500
	cbz	x0, 1f

	/* Set bit 8 in ECTLR_EL1 */
	mrs	x0, CORTEX_A78_AE_CPUECTLR_EL1
	bic	x0, x0, #CORTEX_A78_AE_CPUECTLR_EL1_BIT_8
	msr	CORTEX_A78_AE_CPUECTLR_EL1, x0
	isb
1:
	ret	x17
endfunc errata_a78_ae_1941500_wa

func check_errata_1941500
	/* Applies to revisions r0p0 and r0p1. */
	mov	x1, #CPU_REV(0, 0)
	mov	x2, #CPU_REV(0, 1)
	b	cpu_rev_var_range
endfunc check_errata_1941500

/* --------------------------------------------------
 * Errata Workaround for A78 AE Erratum 1951502.
 * This applies to revisions r0p0 and r0p1 of A78 AE.
 * Inputs:
 * x0: variant[4:7] and revision[0:3] of current cpu.
 * Shall clobber: x0-x17
 * --------------------------------------------------
 */
func errata_a78_ae_1951502_wa
	/* Compare x0 against revisions r0p0 - r0p1 */
	mov	x17, x30
	bl	check_errata_1951502
	cbz	x0, 1f

	msr	S3_6_c15_c8_0, xzr
	ldr	x0, =0x10E3900002
	msr	S3_6_c15_c8_2, x0
	ldr	x0, =0x10FFF00083
	msr	S3_6_c15_c8_3, x0
	ldr	x0, =0x2001003FF
	msr	S3_6_c15_c8_1, x0

	mov	x0, #1
	msr	S3_6_c15_c8_0, x0
	ldr	x0, =0x10E3800082
	msr	S3_6_c15_c8_2, x0
	ldr	x0, =0x10FFF00083
	msr	S3_6_c15_c8_3, x0
	ldr	x0, =0x2001003FF
	msr	S3_6_c15_c8_1, x0

	mov	x0, #2
	msr	S3_6_c15_c8_0, x0
	ldr	x0, =0x10E3800200
	msr	S3_6_c15_c8_2, x0
	ldr	x0, =0x10FFF003E0
	msr	S3_6_c15_c8_3, x0
	ldr	x0, =0x2001003FF
	msr	S3_6_c15_c8_1, x0

	isb
1:
	ret	x17
endfunc errata_a78_ae_1951502_wa

func check_errata_1951502
	/* Applies to revisions r0p0 and r0p1. */
	mov	x1, #CPU_REV(0, 0)
	mov	x2, #CPU_REV(0, 1)
	b	cpu_rev_var_range
endfunc check_errata_1951502

/* --------------------------------------------------
 * Errata Workaround for A78 AE Erratum 2376748.
 * This applies to revisions r0p0 and r0p1 of A78 AE.
 * Inputs:
 * x0: variant[4:7] and revision[0:3] of current cpu.
 * Shall clobber: x0-x17
 * --------------------------------------------------
 */
func errata_a78_ae_2376748_wa
	/* Compare x0 against revisions r0p0 - r0p1 */
	mov	x17, x30
	bl	check_errata_2376748
	cbz	x0, 1f

	/* -------------------------------------------------------
	 * Set CPUACTLR2_EL1[0] to 1 to force PLDW/PFRM ST to
	 * behave like PLD/PRFM LD and not cause invalidations to
	 * other PE caches. There might be a small performance
	 * degradation to this workaround for certain workloads
	 * that share data.
	 * -------------------------------------------------------
	 */
	mrs	x0, CORTEX_A78_AE_ACTLR2_EL1
	orr	x0, x0, #CORTEX_A78_AE_ACTLR2_EL1_BIT_0
	msr	CORTEX_A78_AE_ACTLR2_EL1, x0
	isb
1:
	ret	x17
endfunc errata_a78_ae_2376748_wa

func check_errata_2376748
	/* Applies to revisions r0p0 and r0p1. */
	mov	x1, #CPU_REV(0, 0)
	mov	x2, #CPU_REV(0, 1)
	b	cpu_rev_var_range
endfunc check_errata_2376748

/* --------------------------------------------------
 * Errata Workaround for A78 AE Erratum 2395408.
 * This applies to revisions r0p0 and r0p1 of A78 AE.
 * Inputs:
 * x0: variant[4:7] and revision[0:3] of current cpu.
 * Shall clobber: x0-x17
 * --------------------------------------------------
 */
func errata_a78_ae_2395408_wa
	/* Compare x0 against revisions r0p0 - r0p1 */
	mov	x17, x30
	bl	check_errata_2395408
	cbz	x0, 1f

	/* --------------------------------------------------------
	 * Disable folding of demand requests into older prefetches
	 * with L2 miss requests outstanding by setting the
	 * CPUACTLR2_EL1[40] to 1.
	 * --------------------------------------------------------
	 */
	mrs	x0, CORTEX_A78_AE_ACTLR2_EL1
	orr	x0, x0, #CORTEX_A78_AE_ACTLR2_EL1_BIT_40
	msr	CORTEX_A78_AE_ACTLR2_EL1, x0
	isb
1:
	ret	x17
endfunc errata_a78_ae_2395408_wa

func check_errata_2395408
	/* Applies to revisions r0p0 and r0p1. */
	mov	x1, #CPU_REV(0, 0)
	mov	x2, #CPU_REV(0, 1)
	b	cpu_rev_var_range
endfunc check_errata_2395408

func check_errata_cve_2022_23960
#if WORKAROUND_CVE_2022_23960
	mov	x0, #ERRATA_APPLIES
#else
	mov	x0, #ERRATA_MISSING
#endif
	ret
endfunc check_errata_cve_2022_23960

	/* -------------------------------------------------
	 * The CPU Ops reset function for Cortex-A78-AE
	 * -------------------------------------------------
	 */
func cortex_a78_ae_reset_func
	mov	x19, x30
	bl	cpu_get_rev_var
	mov	x18, x0

#if ERRATA_A78_AE_1941500
	mov	x0, x18
	bl	errata_a78_ae_1941500_wa
#endif

#if ERRATA_A78_AE_1951502
	mov	x0, x18
	bl	errata_a78_ae_1951502_wa
#endif

#if ERRATA_A78_AE_2376748
	mov	x0, x18
	bl	errata_a78_ae_2376748_wa
#endif

#if ERRATA_A78_AE_2395408
	mov	x0, x18
	bl	errata_a78_ae_2395408_wa
#endif

#if ENABLE_AMU
	/* Make sure accesses from EL0/EL1 and EL2 are not trapped to EL3 */
	mrs	x0, actlr_el3
	bic	x0, x0, #CORTEX_A78_ACTLR_TAM_BIT
	msr	actlr_el3, x0

	/* Make sure accesses from non-secure EL0/EL1 are not trapped to EL2 */
	mrs	x0, actlr_el2
	bic	x0, x0, #CORTEX_A78_ACTLR_TAM_BIT
	msr	actlr_el2, x0

	/* Enable group0 counters */
	mov	x0, #CORTEX_A78_AMU_GROUP0_MASK
	msr	CPUAMCNTENSET0_EL0, x0

	/* Enable group1 counters */
	mov	x0, #CORTEX_A78_AMU_GROUP1_MASK
	msr	CPUAMCNTENSET1_EL0, x0
#endif

#if IMAGE_BL31 && WORKAROUND_CVE_2022_23960
	/*
	 * The Cortex-A78AE generic vectors are overridden to apply errata
	 * mitigation on exception entry from lower ELs.
	 */
	adr	x0, wa_cve_vbar_cortex_a78_ae
	msr	vbar_el3, x0
#endif /* IMAGE_BL31 && WORKAROUND_CVE_2022_23960 */

	isb
	ret	x19
endfunc cortex_a78_ae_reset_func

	/* -------------------------------------------------------
	 * HW will do the cache maintenance while powering down
	 * -------------------------------------------------------
	 */
func cortex_a78_ae_core_pwr_dwn
	/* -------------------------------------------------------
	 * Enable CPU power down bit in power control register
	 * -------------------------------------------------------
	 */
	mrs	x0, CORTEX_A78_CPUPWRCTLR_EL1
	orr	x0, x0, #CORTEX_A78_CPUPWRCTLR_EL1_CORE_PWRDN_EN_BIT
	msr	CORTEX_A78_CPUPWRCTLR_EL1, x0
	isb
	ret
endfunc cortex_a78_ae_core_pwr_dwn

	/*
	 * Errata printing function for cortex_a78_ae. Must follow AAPCS.
	 */
#if REPORT_ERRATA
func cortex_a78_ae_errata_report
	stp	x8, x30, [sp, #-16]!

	bl	cpu_get_rev_var
	mov	x8, x0

	/*
	 * Report all errata. The revision-variant information is passed to
	 * checking functions of each errata.
	 */
	report_errata ERRATA_A78_AE_1941500, cortex_a78_ae, 1941500
	report_errata ERRATA_A78_AE_1951502, cortex_a78_ae, 1951502
	report_errata ERRATA_A78_AE_2376748, cortex_a78_ae, 2376748
	report_errata ERRATA_A78_AE_2395408, cortex_a78_ae, 2395408
	report_errata WORKAROUND_CVE_2022_23960, cortex_a78_ae, cve_2022_23960

	ldp	x8, x30, [sp], #16
	ret
endfunc cortex_a78_ae_errata_report
#endif

	/* -------------------------------------------------------
	 * This function provides cortex_a78_ae specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * -------------------------------------------------------
	 */
.section .rodata.cortex_a78_ae_regs, "aS"
cortex_a78_ae_regs:  /* The ascii list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func cortex_a78_ae_cpu_reg_dump
	adr	x6, cortex_a78_ae_regs
	mrs	x8, CORTEX_A78_CPUECTLR_EL1
	ret
endfunc cortex_a78_ae_cpu_reg_dump

declare_cpu_ops cortex_a78_ae, CORTEX_A78_AE_MIDR, \
	cortex_a78_ae_reset_func, \
	cortex_a78_ae_core_pwr_dwn
