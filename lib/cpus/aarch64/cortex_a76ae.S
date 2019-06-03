/*
 * Copyright (c) 2019, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <asm_macros.S>
#include <cortex_a76ae.h>
#include <cpu_macros.S>

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "Cortex-A76AE must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

/* 64-bit only core */
#if CTX_INCLUDE_AARCH32_REGS == 1
#error "Cortex-A76AE supports only AArch64. Compile with CTX_INCLUDE_AARCH32_REGS=0"
#endif

	/* ---------------------------------------------
	 * HW will do the cache maintenance while powering down
	 * ---------------------------------------------
	 */
func cortex_a76ae_core_pwr_dwn
	/* ---------------------------------------------
	 * Enable CPU power down bit in power control register
	 * ---------------------------------------------
	 */
	mrs	x0, CORTEX_A76AE_CPUPWRCTLR_EL1
	orr	x0, x0, #CORTEX_A76AE_CORE_PWRDN_EN_MASK
	msr	CORTEX_A76AE_CPUPWRCTLR_EL1, x0
	isb
	ret
endfunc cortex_a76ae_core_pwr_dwn

#if REPORT_ERRATA
/*
 * Errata printing function for Cortex-A76AE. Must follow AAPCS.
 */
func cortex_a76ae_errata_report
	ret
endfunc cortex_a76ae_errata_report
#endif	/* REPORT_ERRATA */

	/* ---------------------------------------------
	 * This function provides cortex_a76ae specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.cortex_a76ae_regs, "aS"
cortex_a76ae_regs:  /* The ASCII list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func cortex_a76ae_cpu_reg_dump
	adr	x6, cortex_a76ae_regs
	mrs	x8, CORTEX_A76AE_CPUECTLR_EL1
	ret
endfunc cortex_a76ae_cpu_reg_dump

declare_cpu_ops cortex_a76ae, CORTEX_A76AE_MIDR, CPU_NO_RESET_FUNC, \
	cortex_a76ae_core_pwr_dwn
