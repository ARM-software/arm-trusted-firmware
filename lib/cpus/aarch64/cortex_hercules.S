/*
 * Copyright (c) 2019, ARM Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <cortex_hercules.h>
#include <cpu_macros.S>
#include <plat_macros.S>

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "cortex_hercules must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

	/* -------------------------------------------------
	 * The CPU Ops reset function for Cortex-Hercules
	 * -------------------------------------------------
	 */
#if ENABLE_AMU
func cortex_hercules_reset_func
	/* Make sure accesses from EL0/EL1 and EL2 are not trapped to EL3 */
	mrs	x0, actlr_el3
	bic	x0, x0, #CORTEX_HERCULES_ACTLR_TAM_BIT
	msr	actlr_el3, x0

	/* Make sure accesses from non-secure EL0/EL1 are not trapped to EL2 */
	mrs	x0, actlr_el2
	bic	x0, x0, #CORTEX_HERCULES_ACTLR_TAM_BIT
	msr	actlr_el2, x0

	/* Enable group0 counters */
	mov	x0, #CORTEX_HERCULES_AMU_GROUP0_MASK
	msr	CPUAMCNTENSET0_EL0, x0

	/* Enable group1 counters */
	mov	x0, #CORTEX_HERCULES_AMU_GROUP1_MASK
	msr	CPUAMCNTENSET1_EL0, x0
	isb

	ret
endfunc cortex_hercules_reset_func
#endif

	/* ---------------------------------------------
	 * HW will do the cache maintenance while powering down
	 * ---------------------------------------------
	 */
func cortex_hercules_core_pwr_dwn
	/* ---------------------------------------------
	 * Enable CPU power down bit in power control register
	 * ---------------------------------------------
	 */
	mrs	x0, CORTEX_HERCULES_CPUPWRCTLR_EL1
	orr	x0, x0, #CORTEX_HERCULES_CPUPWRCTLR_EL1_CORE_PWRDN_EN_BIT
	msr	CORTEX_HERCULES_CPUPWRCTLR_EL1, x0
	isb
	ret
endfunc cortex_hercules_core_pwr_dwn

	/*
	 * Errata printing function for cortex_hercules. Must follow AAPCS.
	 */
#if REPORT_ERRATA
func cortex_hercules_errata_report
	ret
endfunc cortex_hercules_errata_report
#endif

	/* ---------------------------------------------
	 * This function provides cortex_hercules specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.cortex_hercules_regs, "aS"
cortex_hercules_regs:  /* The ascii list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func cortex_hercules_cpu_reg_dump
	adr	x6, cortex_hercules_regs
	mrs	x8, CORTEX_HERCULES_CPUECTLR_EL1
	ret
endfunc cortex_hercules_cpu_reg_dump

#if ENABLE_AMU
#define HERCULES_RESET_FUNC cortex_hercules_reset_func
#else
#define HERCULES_RESET_FUNC CPU_NO_RESET_FUNC
#endif

declare_cpu_ops cortex_hercules, CORTEX_HERCULES_MIDR, \
	HERCULES_RESET_FUNC, \
	cortex_hercules_core_pwr_dwn
