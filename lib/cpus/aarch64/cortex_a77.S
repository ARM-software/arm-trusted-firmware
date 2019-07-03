/*
 * Copyright (c) 2018-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <cortex_a77.h>
#include <cpu_macros.S>
#include <plat_macros.S>

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "Cortex-A77 must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

/* 64-bit only core */
#if CTX_INCLUDE_AARCH32_REGS == 1
#error "Cortex-A77 supports only AArch64. Compile with CTX_INCLUDE_AARCH32_REGS=0"
#endif

	/* ---------------------------------------------
	 * HW will do the cache maintenance while powering down
	 * ---------------------------------------------
	 */
func cortex_a77_core_pwr_dwn
	/* ---------------------------------------------
	 * Enable CPU power down bit in power control register
	 * ---------------------------------------------
	 */
	mrs	x0, CORTEX_A77_CPUPWRCTLR_EL1
	orr	x0, x0, #CORTEX_A77_CPUPWRCTLR_EL1_CORE_PWRDN_BIT
	msr	CORTEX_A77_CPUPWRCTLR_EL1, x0
	isb
	ret
endfunc cortex_a77_core_pwr_dwn

#if REPORT_ERRATA
/*
 * Errata printing function for Cortex-A77. Must follow AAPCS.
 */
func cortex_a77_errata_report
	ret
endfunc cortex_a77_errata_report
#endif


	/* ---------------------------------------------
	 * This function provides Cortex-A77 specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.cortex_a77_regs, "aS"
cortex_a77_regs:  /* The ascii list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func cortex_a77_cpu_reg_dump
	adr	x6, cortex_a77_regs
	mrs	x8, CORTEX_A77_CPUECTLR_EL1
	ret
endfunc cortex_a77_cpu_reg_dump

declare_cpu_ops cortex_a77, CORTEX_A77_MIDR, \
	CPU_NO_RESET_FUNC, \
	cortex_a77_core_pwr_dwn
