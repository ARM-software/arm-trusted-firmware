/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <cortex_x2.h>
#include <cpu_macros.S>
#include <plat_macros.S>

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "Cortex X2 must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

/* 64-bit only core */
#if CTX_INCLUDE_AARCH32_REGS == 1
#error "Cortex X2 supports only AArch64. Compile with CTX_INCLUDE_AARCH32_REGS=0"
#endif

	/* ----------------------------------------------------
	 * HW will do the cache maintenance while powering down
	 * ----------------------------------------------------
	 */
func cortex_x2_core_pwr_dwn
	/* ---------------------------------------------------
	 * Enable CPU power down bit in power control register
	 * ---------------------------------------------------
	 */
	mrs	x0, CORTEX_X2_CPUPWRCTLR_EL1
	orr	x0, x0, #CORTEX_X2_CPUPWRCTLR_EL1_CORE_PWRDN_BIT
	msr	CORTEX_X2_CPUPWRCTLR_EL1, x0
	isb
	ret
endfunc cortex_x2_core_pwr_dwn

	/*
	 * Errata printing function for Cortex X2. Must follow AAPCS.
	 */
#if REPORT_ERRATA
func cortex_x2_errata_report
	ret
endfunc cortex_x2_errata_report
#endif

func cortex_x2_reset_func
	/* Disable speculative loads */
	msr	SSBS, xzr
	isb
	ret
endfunc cortex_x2_reset_func

	/* ---------------------------------------------
	 * This function provides Cortex X2 specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.cortex_x2_regs, "aS"
cortex_x2_regs:  /* The ascii list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func cortex_x2_cpu_reg_dump
	adr	x6, cortex_x2_regs
	mrs	x8, CORTEX_X2_CPUECTLR_EL1
	ret
endfunc cortex_x2_cpu_reg_dump

declare_cpu_ops cortex_x2, CORTEX_X2_MIDR, \
	cortex_x2_reset_func, \
	cortex_x2_core_pwr_dwn
