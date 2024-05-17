/*
 * Copyright (c) 2023-2024, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <cortex_x925.h>
#include <cpu_macros.S>
#include <plat_macros.S>

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "Cortex-X925 must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

/* 64-bit only core */
#if CTX_INCLUDE_AARCH32_REGS == 1
#error "Cortex-X925 supports only AArch64. Compile with CTX_INCLUDE_AARCH32_REGS=0"
#endif

cpu_reset_func_start cortex_x925
	/* Disable speculative loads */
	msr	SSBS, xzr
cpu_reset_func_end cortex_x925

	/* ----------------------------------------------------
	 * HW will do the cache maintenance while powering down
	 * ----------------------------------------------------
	 */
func cortex_x925_core_pwr_dwn
	/* ---------------------------------------------------
	 * Enable CPU power down bit in power control register
	 * ---------------------------------------------------
	 */
	sysreg_bit_set CORTEX_X925_CPUPWRCTLR_EL1, CORTEX_X925_CPUPWRCTLR_EL1_CORE_PWRDN_BIT
	isb
	ret
endfunc cortex_x925_core_pwr_dwn

errata_report_shim cortex_x925

	/* ---------------------------------------------
	 * This function provides Cortex-X925 specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.cortex_x925_regs, "aS"
cortex_x925_regs:  /* The ascii list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func cortex_x925_cpu_reg_dump
	adr	x6, cortex_x925_regs
	mrs	x8, CORTEX_X925_CPUECTLR_EL1
	ret
endfunc cortex_x925_cpu_reg_dump

declare_cpu_ops cortex_x925, CORTEX_X925_MIDR, \
	cortex_x925_reset_func, \
	cortex_x925_core_pwr_dwn
