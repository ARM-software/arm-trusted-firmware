/*
 * Copyright (c) 2024-2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <cortex_a720_ae.h>
#include <cpu_macros.S>
#include <plat_macros.S>

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "Cortex-A720AE must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

/* 64-bit only core */
#if CTX_INCLUDE_AARCH32_REGS == 1
#error "Cortex-A720AE supports only AArch64. Compile with CTX_INCLUDE_AARCH32_REGS=0"
#endif

cpu_reset_prologue cortex_a720_ae

.global check_erratum_cortex_a720_ae_3699562

add_erratum_entry cortex_a720_ae, ERRATUM(3699562), ERRATA_A720_AE_3699562

check_erratum_ls cortex_a720_ae, ERRATUM(3699562), CPU_REV(0, 0)

cpu_reset_func_start cortex_a720_ae
	/* Disable speculative loads */
	msr	SSBS, xzr
cpu_reset_func_end cortex_a720_ae

	/* ----------------------------------------------------
	 * HW will do the cache maintenance while powering down
	 * ----------------------------------------------------
	 */
func cortex_a720_ae_core_pwr_dwn
	/* ---------------------------------------------------
	 * Enable CPU power down bit in power control register
	 * ---------------------------------------------------
	 */
	sysreg_bit_set CORTEX_A720_AE_CPUPWRCTLR_EL1, CORTEX_A720_AE_CPUPWRCTLR_EL1_CORE_PWRDN_BIT

	isb
	ret
endfunc cortex_a720_ae_core_pwr_dwn

	/* ---------------------------------------------
	 * This function provides Cortex-A720AE specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.cortex_a720_ae_regs, "aS"
cortex_a720_ae_regs:  /* The ascii list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func cortex_a720_ae_cpu_reg_dump
	adr	x6, cortex_a720_ae_regs
	mrs	x8, CORTEX_A720_AE_CPUECTLR_EL1
	ret
endfunc cortex_a720_ae_cpu_reg_dump

declare_cpu_ops cortex_a720_ae, CORTEX_A720_AE_MIDR, \
	cortex_a720_ae_reset_func, \
	cortex_a720_ae_core_pwr_dwn
