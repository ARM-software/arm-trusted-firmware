/*
 * Copyright (c) 2023-2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <c1_ultra.h>
#include <common/bl_common.h>
#include <cpu_macros.S>

#include <plat_macros.S>

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "Arm C1-Ultra must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

/* 64-bit only core */
#if CTX_INCLUDE_AARCH32_REGS == 1
#error "Arm C1-Ultra supports only AArch64. Compile with CTX_INCLUDE_AARCH32_REGS=0"
#endif

#if ERRATA_SME_POWER_DOWN == 0
#error "Arm C1-Ultra needs ERRATA_SME_POWER_DOWN=1 to powerdown correctly"
#endif

cpu_reset_prologue c1_ultra

cpu_reset_func_start c1_ultra
	/* ----------------------------------------------------
	 * Disable speculative loads
	 * ----------------------------------------------------
	 */
	msr	SSBS, xzr
	/* model bug: not cleared on reset */
	sysreg_bit_clear C1_ULTRA_IMP_CPUPWRCTLR_EL1, \
		C1_ULTRA_IMP_CPUPWRCTLR_EL1_CORE_PWRDN_EN_BIT
	enable_mpmm
cpu_reset_func_end c1_ultra

func c1_ultra_core_pwr_dwn
	/* ---------------------------------------------------
	 * Flip CPU power down bit in power control register.
	 * It will be set on powerdown and cleared on wakeup
	 * ---------------------------------------------------
	 */
	sysreg_bit_toggle C1_ULTRA_IMP_CPUPWRCTLR_EL1, \
		C1_ULTRA_IMP_CPUPWRCTLR_EL1_CORE_PWRDN_EN_BIT
	isb
	signal_pabandon_handled
	ret
endfunc c1_ultra_core_pwr_dwn

.section .rodata.c1_ultra_regs, "aS"
c1_ultra_regs: /* The ASCII list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func c1_ultra_cpu_reg_dump
	adr 	x6, c1_ultra_regs
	mrs	x8, C1_ULTRA_IMP_CPUECTLR_EL1
	ret
endfunc c1_ultra_cpu_reg_dump

declare_cpu_ops c1_ultra, C1_ULTRA_MIDR, \
	c1_ultra_reset_func, \
	c1_ultra_core_pwr_dwn
