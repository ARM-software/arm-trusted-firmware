/*
 * Copyright (c) 2025-2026, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * TRM: https://support.arm.com/documentation/109736/latest
 * SDEN: https://support.arm.com/documentation/112420/latest
 */

#include <arch.h>
#include <asm_macros.S>
#include <c2_ultra.h>
#include <common/bl_common.h>
#include <cpu_macros.S>
#include <plat_macros.S>

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "Arm C2-Ultra must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

/* 64-bit only core */
#if CTX_INCLUDE_AARCH32_REGS == 1
#error "Arm C2-Ultra supports only AArch64. Compile with CTX_INCLUDE_AARCH32_REGS=0"
#endif

cpu_reset_prologue c2_ultra

cpu_reset_func_start c2_ultra
	/* ----------------------------------------------------
	 * Disable speculative loads
	 * ----------------------------------------------------
	 */
	msr	SSBS, xzr
cpu_reset_func_end c2_ultra

func c2_ultra_core_pwr_dwn
	/*
	 * When software running at lower ELs requests power down without first
	 * disabling SME, the CME connected to it will reject its power down
	 * request. Skip setting the PWRDN_EN bit, downgrading the powerdown
	 * request to a simple WFI wait, to get a minimal amount of power saving
	 * rather than an instant pabandon.
	 */
	mrs	x0, SVCR
	cbnz	x0, c2_ultra_skip_pwr_dwn

	/* ---------------------------------------------------
	 * Flip CPU power down bit in power control register.
	 * It will be set on powerdown and cleared on wakeup
	 * ---------------------------------------------------
	 */
	sysreg_bit_toggle C2_ULTRA_CPUPWRCTLR_EL1, \
		C2_ULTRA_CPUPWRCTLR_EL1_CORE_PWRDN_EN_BIT
	isb
c2_ultra_skip_pwr_dwn:
	signal_pabandon_handled
	ret
endfunc c2_ultra_core_pwr_dwn

.section .rodata.c2_ultra_regs, "aS"
c2_ultra_regs: /* The ASCII list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func c2_ultra_cpu_reg_dump
	adr 	x6, c2_ultra_regs
	mrs	x8, C2_ULTRA_CPUECTLR_EL1
	ret
endfunc c2_ultra_cpu_reg_dump

declare_cpu_ops c2_ultra, C2_ULTRA_MIDR, \
	c2_ultra_reset_func, \
	c2_ultra_core_pwr_dwn
