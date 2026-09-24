/*
 * Copyright (c) 2026, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <rosillo.h>
#include <cpu_macros.S>
#include <plat_macros.S>

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "Rosillo must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

/* 64-bit only core */
#if CTX_INCLUDE_AARCH32_REGS == 1
#error "Rosillo supports only AArch64. Compile with CTX_INCLUDE_AARCH32_REGS=0"
#endif

cpu_reset_prologue rosillo

cpu_reset_func_start rosillo
	/* ----------------------------------------------------
	 * Disable speculative loads
	 * ----------------------------------------------------
	 */
	msr	SSBS, xzr
	enable_mpmm
cpu_reset_func_end rosillo

func rosillo_core_pwr_dwn
	/*
	 * When software running at lower ELs requests power down without first
	 * disabling SME, the CME connected to it will reject its power down
	 * request. Skip setting the PWRDN_EN bit, downgrading the powerdown
	 * request to a simple WFI wait, to get a minimal amount of power saving
	 * rather than an instant pabandon.
	 */
	mrs	x0, SVCR
	cbnz	x0, rosillo_skip_pwr_dwn

	/* ---------------------------------------------------
	 * Flip CPU power down bit in power control register.
	 * It will be set on powerdown and cleared on wakeup
	 * ---------------------------------------------------
	 */
	sysreg_bit_toggle ROSILLO_IMP_CPUPWRCTLR_EL1, \
		ROSILLO_IMP_CPUPWRCTLR_EL1_CORE_PWRDN_EN_BIT
	isb
rosillo_skip_pwr_dwn:
	signal_pabandon_handled
	ret
endfunc rosillo_core_pwr_dwn


declare_cpu_ops rosillo, ROSILLO_MIDR, \
	rosillo_reset_func, \
	rosillo_core_pwr_dwn
