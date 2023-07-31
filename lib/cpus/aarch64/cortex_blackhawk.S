/*
 * Copyright (c) 2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <cortex_blackhawk.h>
#include <cpu_macros.S>
#include <plat_macros.S>

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "Cortex blackhawk must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

/* 64-bit only core */
#if CTX_INCLUDE_AARCH32_REGS == 1
#error "Cortex blackhawk supports only AArch64. Compile with CTX_INCLUDE_AARCH32_REGS=0"
#endif

cpu_reset_func_start cortex_blackhawk
	/* Disable speculative loads */
	msr	SSBS, xzr
cpu_reset_func_end cortex_blackhawk

	/* ----------------------------------------------------
	 * HW will do the cache maintenance while powering down
	 * ----------------------------------------------------
	 */
func cortex_blackhawk_core_pwr_dwn
	/* ---------------------------------------------------
	 * Enable CPU power down bit in power control register
	 * ---------------------------------------------------
	 */
	sysreg_bit_set CORTEX_BLACKHAWK_CPUPWRCTLR_EL1, CORTEX_BLACKHAWK_CPUPWRCTLR_EL1_CORE_PWRDN_BIT
	isb
	ret
endfunc cortex_blackhawk_core_pwr_dwn

errata_report_shim cortex_blackhawk

	/* ---------------------------------------------
	 * This function provides Cortex Blackhawk specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.cortex_blackhawk_regs, "aS"
cortex_blackhawk_regs:  /* The ascii list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func cortex_blackhawk_cpu_reg_dump
	adr	x6, cortex_blackhawk_regs
	mrs	x8, CORTEX_BLACKHAWK_CPUECTLR_EL1
	ret
endfunc cortex_blackhawk_cpu_reg_dump

declare_cpu_ops cortex_blackhawk, CORTEX_BLACKHAWK_MIDR, \
	cortex_blackhawk_reset_func, \
	cortex_blackhawk_core_pwr_dwn
