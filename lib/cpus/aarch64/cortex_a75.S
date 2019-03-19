/*
 * Copyright (c) 2017-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <cortex_a75.h>
#include <cpuamu.h>
#include <cpu_macros.S>

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "Cortex-A75 must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

	/* --------------------------------------------------
	 * Errata Workaround for Cortex A75 Errata #764081.
	 * This applies only to revision r0p0 of Cortex A75.
	 * Inputs:
	 * x0: variant[4:7] and revision[0:3] of current cpu.
	 * Shall clobber: x0-x17
	 * --------------------------------------------------
	 */
func errata_a75_764081_wa
	/*
	 * Compare x0 against revision r0p0
	 */
	mov	x17, x30
	bl	check_errata_764081
	cbz	x0, 1f
	mrs	x1, sctlr_el3
	orr	x1, x1 ,#SCTLR_IESB_BIT
	msr	sctlr_el3, x1
	isb
1:
	ret	x17
endfunc errata_a75_764081_wa

func check_errata_764081
	mov	x1, #0x00
	b	cpu_rev_var_ls
endfunc check_errata_764081

	/* --------------------------------------------------
	 * Errata Workaround for Cortex A75 Errata #790748.
	 * This applies only to revision r0p0 of Cortex A75.
	 * Inputs:
	 * x0: variant[4:7] and revision[0:3] of current cpu.
	 * Shall clobber: x0-x17
	 * --------------------------------------------------
	 */
func errata_a75_790748_wa
	/*
	 * Compare x0 against revision r0p0
	 */
	mov	x17, x30
	bl	check_errata_790748
	cbz	x0, 1f
	mrs	x1, CORTEX_A75_CPUACTLR_EL1
	orr	x1, x1 ,#(1 << 13)
	msr	CORTEX_A75_CPUACTLR_EL1, x1
	isb
1:
	ret	x17
endfunc errata_a75_790748_wa

func check_errata_790748
	mov	x1, #0x00
	b	cpu_rev_var_ls
endfunc check_errata_790748

	/* -------------------------------------------------
	 * The CPU Ops reset function for Cortex-A75.
	 * -------------------------------------------------
	 */
func cortex_a75_reset_func
	mov	x19, x30
	bl	cpu_get_rev_var
	mov	x18, x0

#if ERRATA_A75_764081
	mov	x0, x18
	bl	errata_a75_764081_wa
#endif

#if ERRATA_A75_790748
	mov	x0, x18
	bl	errata_a75_790748_wa
#endif

#if IMAGE_BL31 && WORKAROUND_CVE_2017_5715
	cpu_check_csv2	x0, 1f
	adr	x0, wa_cve_2017_5715_bpiall_vbar
	msr	vbar_el3, x0
	isb
1:
#endif

#if WORKAROUND_CVE_2018_3639
	mrs	x0, CORTEX_A75_CPUACTLR_EL1
	orr	x0, x0, #CORTEX_A75_CPUACTLR_EL1_DISABLE_LOAD_PASS_STORE
	msr	CORTEX_A75_CPUACTLR_EL1, x0
	isb
#endif

#if ERRATA_DSU_798953
	bl	errata_dsu_798953_wa
#endif

#if ERRATA_DSU_936184
	bl	errata_dsu_936184_wa
#endif

#if ENABLE_AMU
	/* Make sure accesses from EL0/EL1 and EL2 are not trapped to EL3 */
	mrs	x0, actlr_el3
	orr	x0, x0, #CORTEX_A75_ACTLR_AMEN_BIT
	msr	actlr_el3, x0
	isb

	/* Make sure accesses from EL0/EL1 are not trapped to EL2 */
	mrs	x0, actlr_el2
	orr	x0, x0, #CORTEX_A75_ACTLR_AMEN_BIT
	msr	actlr_el2, x0
	isb

	/* Enable group0 counters */
	mov	x0, #CORTEX_A75_AMU_GROUP0_MASK
	msr	CPUAMCNTENSET_EL0, x0
	isb

	/* Enable group1 counters */
	mov	x0, #CORTEX_A75_AMU_GROUP1_MASK
	msr	CPUAMCNTENSET_EL0, x0
	isb
#endif
	ret	x19
endfunc cortex_a75_reset_func

func check_errata_cve_2017_5715
	cpu_check_csv2	x0, 1f
#if WORKAROUND_CVE_2017_5715
	mov	x0, #ERRATA_APPLIES
#else
	mov	x0, #ERRATA_MISSING
#endif
	ret
1:
	mov	x0, #ERRATA_NOT_APPLIES
	ret
endfunc check_errata_cve_2017_5715

func check_errata_cve_2018_3639
#if WORKAROUND_CVE_2018_3639
	mov	x0, #ERRATA_APPLIES
#else
	mov	x0, #ERRATA_MISSING
#endif
	ret
endfunc check_errata_cve_2018_3639

	/* ---------------------------------------------
	 * HW will do the cache maintenance while powering down
	 * ---------------------------------------------
	 */
func cortex_a75_core_pwr_dwn
	/* ---------------------------------------------
	 * Enable CPU power down bit in power control register
	 * ---------------------------------------------
	 */
	mrs	x0, CORTEX_A75_CPUPWRCTLR_EL1
	orr	x0, x0, #CORTEX_A75_CORE_PWRDN_EN_MASK
	msr	CORTEX_A75_CPUPWRCTLR_EL1, x0
	isb
	ret
endfunc cortex_a75_core_pwr_dwn

#if REPORT_ERRATA
/*
 * Errata printing function for Cortex A75. Must follow AAPCS.
 */
func cortex_a75_errata_report
	stp	x8, x30, [sp, #-16]!

	bl	cpu_get_rev_var
	mov	x8, x0

	/*
	 * Report all errata. The revision-variant information is passed to
	 * checking functions of each errata.
	 */
	report_errata ERRATA_A75_764081, cortex_a75, 764081
	report_errata ERRATA_A75_790748, cortex_a75, 790748
	report_errata WORKAROUND_CVE_2017_5715, cortex_a75, cve_2017_5715
	report_errata WORKAROUND_CVE_2018_3639, cortex_a75, cve_2018_3639
	report_errata ERRATA_DSU_798953, cortex_a75, dsu_798953
	report_errata ERRATA_DSU_936184, cortex_a75, dsu_936184

	ldp	x8, x30, [sp], #16
	ret
endfunc cortex_a75_errata_report
#endif

	/* ---------------------------------------------
	 * This function provides cortex_a75 specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.cortex_a75_regs, "aS"
cortex_a75_regs:  /* The ascii list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func cortex_a75_cpu_reg_dump
	adr	x6, cortex_a75_regs
	mrs	x8, CORTEX_A75_CPUECTLR_EL1
	ret
endfunc cortex_a75_cpu_reg_dump

declare_cpu_ops_wa cortex_a75, CORTEX_A75_MIDR, \
	cortex_a75_reset_func, \
	check_errata_cve_2017_5715, \
	CPU_NO_EXTRA2_FUNC, \
	cortex_a75_core_pwr_dwn
