/*
 * Copyright (c) 2024-2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <cortex_a320.h>
#include <cpu_macros.S>
#include <plat_macros.S>

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "Cortex-A320 must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

/* 64-bit only core */
#if CTX_INCLUDE_AARCH32_REGS == 1
#error "Cortex-A320 supports only AArch64. Compile with CTX_INCLUDE_AARCH32_REGS=0"
#endif

cpu_reset_prologue cortex_a320

cpu_reset_func_start cortex_a320
	/* Disable speculative loads */
	msr	SSBS, xzr
	enable_mpmm
cpu_reset_func_end cortex_a320

	/* ----------------------------------------------------
	 * HW will do the cache maintenance while powering down
	 * ----------------------------------------------------
	 */
func cortex_a320_core_pwr_dwn
	/* ---------------------------------------------------
	 * Enable CPU power down bit in power control register
	 * ---------------------------------------------------
	 */
	sysreg_bit_set CORTEX_A320_CPUPWRCTLR_EL1, CORTEX_A320_CPUPWRCTLR_EL1_CORE_PWRDN_BIT
	isb
	ret
endfunc cortex_a320_core_pwr_dwn

	/* ---------------------------------------------
	 * This function provides Cortex-A320 specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.cortex_a320_regs, "aS"
cortex_a320_regs:  /* The ascii list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func cortex_a320_cpu_reg_dump
	adr	x6, cortex_a320_regs
	mrs	x8, CORTEX_A320_CPUECTLR_EL1
	ret
endfunc cortex_a320_cpu_reg_dump

declare_cpu_ops cortex_a320, CORTEX_A320_MIDR, \
	cortex_a320_reset_func, \
	cortex_a320_core_pwr_dwn
