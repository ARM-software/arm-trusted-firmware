/*
 * Copyright (c) 2019-2020, ARM Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <cortex_a78_ae.h>
#include <cpu_macros.S>
#include <plat_macros.S>

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "cortex_a78_ae must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

	/* -------------------------------------------------
	 * The CPU Ops reset function for Cortex-A78-AE
	 * -------------------------------------------------
	 */
#if ENABLE_AMU
func cortex_a78_ae_reset_func
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
	isb

	ret
endfunc cortex_a78_ae_reset_func
#endif

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

#if ENABLE_AMU
#define A78_AE_RESET_FUNC cortex_a78_ae_reset_func
#else
#define A78_AE_RESET_FUNC CPU_NO_RESET_FUNC
#endif

declare_cpu_ops cortex_a78_ae, CORTEX_A78_AE_MIDR, \
	A78_AE_RESET_FUNC, \
	cortex_a78_ae_core_pwr_dwn
