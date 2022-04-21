/*
 * Copyright (c) 2022, Google LLC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <asm_macros.S>
#include <cortex_x1.h>
#include <cpu_macros.S>

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "Cortex-X1 must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

/* 64-bit only core */
#if CTX_INCLUDE_AARCH32_REGS == 1
#error "Cortex-X1 supports only AArch64. Compile with CTX_INCLUDE_AARCH32_REGS=0"
#endif

func cortex_x1_reset_func
	ret
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
