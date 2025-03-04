/*
 * Copyright (c) 2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <lsc25_e_core.h>
#include <cpu_macros.S>
#include <plat_macros.S>

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "LSC25 E-core must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

/* 64-bit only core */
#if CTX_INCLUDE_AARCH32_REGS == 1
#error "LSC25 E-core supports only AArch64. Compile with " \
       "CTX_INCLUDE_AARCH32_REGS=0"
#endif

#if ERRATA_SME_POWER_DOWN == 0
#error "LSC25 E-core needs ERRATA_SME_POWER_DOWN=1 to powerdown correctly"
#endif

cpu_reset_prologue lsc25_e_core

cpu_reset_func_start lsc25_e_core
	/* ----------------------------------------------------
	 * Disable speculative loads
	 * ----------------------------------------------------
	 */
	msr	SSBS, xzr
	enable_mpmm
cpu_reset_func_end lsc25_e_core

	/* ----------------------------------------------------
	 * HW will do the cache maintenance while powering down
	 * ----------------------------------------------------
	 */
func lsc25_e_core_core_pwr_dwn
	/* ---------------------------------------------------
	 * Flip CPU power down bit in power control register.
	 * It will be set on powerdown and cleared on wakeup
	 * ---------------------------------------------------
	 */
	sysreg_bit_toggle LSC25_E_CORE_IMP_CPUPWRCTLR_EL1, \
		LSC25_E_CORE_IMP_CPUPWRCTLR_EL1_CORE_PWRDN_EN_BIT
	isb
	signal_pabandon_handled
	ret
endfunc lsc25_e_core_core_pwr_dwn

	/* ---------------------------------------------
	 * This function provides LSC25 E-Core specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.lsc25_e_core_regs, "aS"
lsc25_e_core_regs: /* The ASCII list of register names to be reported */
	.asciz	"imp_cpuectlr_el1", ""

func lsc25_e_core_cpu_reg_dump
	adr	x6, lsc25_e_core_regs
	mrs	x8, LSC25_E_CORE_IMP_CPUECTLR_EL1
	ret
endfunc lsc25_e_core_cpu_reg_dump

declare_cpu_ops lsc25_e_core, LSC25_E_CORE_MIDR, \
	lsc25_e_core_reset_func, \
	lsc25_e_core_core_pwr_dwn
