/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <bl_common.h>
#include <cortex_a55.h>
#include <cpu_macros.S>
#include <plat_macros.S>

	/* ---------------------------------------------
	 * Disable L1 data cache and unified L2/L3 cache
	 * ---------------------------------------------
	 */
func cortex_a55_disable_dcache
	mrs	x1, sctlr_el3
	bic	x1, x1, #SCTLR_C_BIT
	msr	sctlr_el3, x1
	isb
	ret
endfunc cortex_a55_disable_dcache
	/* ---------------------------------------------
	 * HW will do the cache maintenance while powering down
	 * ---------------------------------------------
	 */
func cortex_a55_core_pwr_dwn
#if !HW_ASSISTED_COHERENCY
	mov x18, x30
	bl  cortex_a55_disable_dcache
	mov	x30, x18
#endif
	/* ---------------------------------------------
	 * Enable CPU power down bit in power control register
	 * ---------------------------------------------
	 */
	mrs	x0, CORTEX_A55_CPUPWRCTLR_EL1
	orr	x0, x0, #CORTEX_A55_CORE_PWRDN_EN_MASK
	msr	CORTEX_A55_CPUPWRCTLR_EL1, x0
	isb
	ret
endfunc cortex_a55_core_pwr_dwn

	/* ---------------------------------------------
	 * This function provides cortex_a55 specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.cortex_a55_regs, "aS"
cortex_a55_regs:  /* The ascii list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func cortex_a55_cpu_reg_dump
	adr	x6, cortex_a55_regs
	mrs	x8, CORTEX_A55_CPUECTLR_EL1
	ret
endfunc cortex_a55_cpu_reg_dump

declare_cpu_ops cortex_a55, CORTEX_A55_MIDR, \
	CPU_NO_RESET_FUNC, \
	cortex_a55_core_pwr_dwn
