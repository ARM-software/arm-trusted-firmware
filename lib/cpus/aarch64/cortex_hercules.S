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

declare_cpu_ops cortex_hercules, CORTEX_HERCULES_MIDR, \
	CPU_NO_RESET_FUNC, \
	cortex_hercules_core_pwr_dwn
