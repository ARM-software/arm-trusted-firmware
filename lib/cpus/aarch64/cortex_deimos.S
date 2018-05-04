/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <bl_common.h>
#include <cortex_deimos.h>
#include <cpu_macros.S>
#include <plat_macros.S>

	/* ---------------------------------------------
	 * HW will do the cache maintenance while powering down
	 * ---------------------------------------------
	 */
func cortex_deimos_core_pwr_dwn
	/* ---------------------------------------------
	 * Enable CPU power down bit in power control register
	 * ---------------------------------------------
	 */
	mrs	x0, CORTEX_DEIMOS_CPUPWRCTLR_EL1
	orr	x0, x0, #CORTEX_DEIMOS_CPUPWRCTLR_EL1_CORE_PWRDN_BIT
	msr	CORTEX_DEIMOS_CPUPWRCTLR_EL1, x0
	isb
	ret
endfunc cortex_deimos_core_pwr_dwn

	/* ---------------------------------------------
	 * This function provides Cortex-Deimos specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.cortex_deimos_regs, "aS"
cortex_deimos_regs:  /* The ascii list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func cortex_deimos_cpu_reg_dump
	adr	x6, cortex_deimos_regs
	mrs	x8, CORTEX_DEIMOS_CPUECTLR_EL1
	ret
endfunc cortex_deimos_cpu_reg_dump

declare_cpu_ops cortex_deimos, CORTEX_DEIMOS_MIDR, \
	CPU_NO_RESET_FUNC, \
	cortex_deimos_core_pwr_dwn
