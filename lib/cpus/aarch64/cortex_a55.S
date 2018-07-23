/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <bl_common.h>
#include <cortex_a55.h>
#include <cpu_macros.S>
#include <plat_macros.S>

func cortex_a55_reset_func
	mov	x19, x30
#if ERRATA_DSU_936184
	bl	errata_dsu_936184_wa
#endif
	ret	x19
endfunc cortex_a55_reset_func

	/* ---------------------------------------------
	 * HW will do the cache maintenance while powering down
	 * ---------------------------------------------
	 */
func cortex_a55_core_pwr_dwn
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

#if REPORT_ERRATA
/*
 * Errata printing function for Cortex A55. Must follow AAPCS & can use stack.
 */
func cortex_a55_errata_report
	stp	x8, x30, [sp, #-16]!
	bl	cpu_get_rev_var
	mov	x8, x0

	/*
	 * Report all errata. The revision variant information is at x8, where
	 * "report_errata" is expecting it and it doesn't corrupt it.
	 */
	report_errata ERRATA_DSU_936184, cortex_a55, dsu_936184

	ldp	x8, x30, [sp], #16
	ret
endfunc cortex_a55_errata_report
#endif

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
	cortex_a55_reset_func, \
	cortex_a55_core_pwr_dwn
