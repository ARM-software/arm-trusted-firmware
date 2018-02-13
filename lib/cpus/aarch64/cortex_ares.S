/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <cortex_ares.h>
#include <cpuamu.h>
#include <cpu_macros.S>

func cortex_ares_reset_func
#if ENABLE_AMU
	/* Make sure accesses from EL0/EL1 and EL2 are not trapped to EL3 */
	mrs	x0, actlr_el3
	orr	x0, x0, #CORTEX_ARES_ACTLR_AMEN_BIT
	msr	actlr_el3, x0
	isb

	/* Make sure accesses from EL0/EL1 are not trapped to EL2 */
	mrs	x0, actlr_el2
	orr	x0, x0, #CORTEX_ARES_ACTLR_AMEN_BIT
	msr	actlr_el2, x0
	isb

	/* Enable group0 counters */
	mov	x0, #CORTEX_ARES_AMU_GROUP0_MASK
	msr	CPUAMCNTENSET_EL0, x0
	isb
#endif
	ret
endfunc cortex_ares_reset_func

	/* ---------------------------------------------
	 * HW will do the cache maintenance while powering down
	 * ---------------------------------------------
	 */
func cortex_ares_core_pwr_dwn
	/* ---------------------------------------------
	 * Enable CPU power down bit in power control register
	 * ---------------------------------------------
	 */
	mrs	x0, CORTEX_ARES_CPUPWRCTLR_EL1
	orr	x0, x0, #CORTEX_ARES_CORE_PWRDN_EN_MASK
	msr	CORTEX_ARES_CPUPWRCTLR_EL1, x0
	isb
	ret
endfunc cortex_ares_core_pwr_dwn

	/* ---------------------------------------------
	 * This function provides cortex_ares specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.cortex_ares_regs, "aS"
cortex_ares_regs:  /* The ascii list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func cortex_ares_cpu_reg_dump
	adr	x6, cortex_ares_regs
	mrs	x8, CORTEX_ARES_CPUECTLR_EL1
	ret
endfunc cortex_ares_cpu_reg_dump

declare_cpu_ops cortex_ares, CORTEX_ARES_MIDR, \
	cortex_ares_reset_func, \
	cortex_ares_core_pwr_dwn
