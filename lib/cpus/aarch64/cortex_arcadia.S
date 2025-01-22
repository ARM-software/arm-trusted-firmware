/*
 * Copyright (c) 2024-2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <cortex_arcadia.h>
#include <cpu_macros.S>
#include <plat_macros.S>

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "Cortex-ARCADIA must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

/* 64-bit only core */
#if CTX_INCLUDE_AARCH32_REGS == 1
#error "Cortex-ARCADIA supports only AArch64. Compile with CTX_INCLUDE_AARCH32_REGS=0"
#endif

cpu_reset_prologue cortex_arcadia

cpu_reset_func_start cortex_arcadia
	/* Disable speculative loads */
	msr	SSBS, xzr
cpu_reset_func_end cortex_arcadia

	/* ----------------------------------------------------
	 * HW will do the cache maintenance while powering down
	 * ----------------------------------------------------
	 */
func cortex_arcadia_core_pwr_dwn
	/* ---------------------------------------------------
	 * Enable CPU power down bit in power control register
	 * ---------------------------------------------------
	 */
	sysreg_bit_set CORTEX_ARCADIA_CPUPWRCTLR_EL1, CORTEX_ARCADIA_CPUPWRCTLR_EL1_CORE_PWRDN_BIT
	isb
	ret
endfunc cortex_arcadia_core_pwr_dwn

	/* ---------------------------------------------
	 * This function provides Cortex-Arcadia specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.cortex_arcadia_regs, "aS"
cortex_arcadia_regs:  /* The ascii list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func cortex_arcadia_cpu_reg_dump
	adr	x6, cortex_arcadia_regs
	mrs	x8, CORTEX_ARCADIA_CPUECTLR_EL1
	ret
endfunc cortex_arcadia_cpu_reg_dump

declare_cpu_ops cortex_arcadia, CORTEX_ARCADIA_MIDR, \
	cortex_arcadia_reset_func, \
	cortex_arcadia_core_pwr_dwn
