/*
 * Copyright (c) 2025-2026, Arm Limited. All rights reserved.
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
	/*
	 * When software running at lower ELs requests power down without first
	 * disabling SME, the CME connected to it will reject its power down
	 * request. Skip setting the PWRDN_EN bit, downgrading the powerdown
	 * request to a simple WFI wait, to get a minimal amount of power saving
	 * rather than an instant pabandon.
	 */
	mrs	x0, SVCR
	cbnz	x0, lsc25_e_core_skip_pwr_dwn

	/* ---------------------------------------------------
	 * Flip CPU power down bit in power control register.
	 * It will be set on powerdown and cleared on wakeup
	 * ---------------------------------------------------
	 */
	sysreg_bit_toggle LSC25_E_CORE_IMP_CPUPWRCTLR_EL1, \
		LSC25_E_CORE_IMP_CPUPWRCTLR_EL1_CORE_PWRDN_EN_BIT
	isb
lsc25_e_core_skip_pwr_dwn:
	signal_pabandon_handled
	ret
endfunc lsc25_e_core_core_pwr_dwn


declare_cpu_ops lsc25_e_core, LSC25_E_CORE_MIDR, \
	lsc25_e_core_reset_func, \
	lsc25_e_core_core_pwr_dwn
