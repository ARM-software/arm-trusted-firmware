/*
 * Copyright (c) 2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <lsc25_p_core.h>
#include <cpu_macros.S>
#include <plat_macros.S>

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "LSC25 P-core must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

/* 64-bit only core */
#if CTX_INCLUDE_AARCH32_REGS == 1
#error "LSC25 P-core supports only AArch64. Compile with " \
       "CTX_INCLUDE_AARCH32_REGS=0"
#endif

#if ERRATA_SME_POWER_DOWN == 0
#error "LSC25 P-core needs ERRATA_SME_POWER_DOWN=1 to powerdown correctly"
#endif

cpu_reset_prologue lsc25_p_core

cpu_reset_func_start lsc25_p_core
	/* ----------------------------------------------------
	 * Disable speculative loads
	 * ----------------------------------------------------
	 */
	msr	SSBS, xzr
	enable_mpmm
cpu_reset_func_end lsc25_p_core

func lsc25_p_core_core_pwr_dwn
	/* ---------------------------------------------------
	 * Flip CPU power down bit in power control register.
	 * It will be set on powerdown and cleared on wakeup
	 * ---------------------------------------------------
	 */
	sysreg_bit_toggle LSC25_P_CORE_IMP_CPUPWRCTLR_EL1, \
		LSC25_P_CORE_IMP_CPUPWRCTLR_EL1_CORE_PWRDN_EN_BIT
	isb
	signal_pabandon_handled
	ret
endfunc lsc25_p_core_core_pwr_dwn

.section .rodata.lsc25_p_core_regs, "aS"
lsc25_p_core_regs: /* The ASCII list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func lsc25_p_core_cpu_reg_dump
	adr 	x6, lsc25_p_core_regs
	mrs	x8, LSC25_P_CORE_IMP_CPUECTLR_EL1
	ret
endfunc lsc25_p_core_cpu_reg_dump

declare_cpu_ops lsc25_p_core, LSC25_P_CORE_MIDR, \
	lsc25_p_core_reset_func, \
	lsc25_p_core_core_pwr_dwn
