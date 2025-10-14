/*
 * Copyright (c) 2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <venom.h>
#include <cpu_macros.S>
#include <plat_macros.S>

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "Venom must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

/* 64-bit only core */
#if CTX_INCLUDE_AARCH32_REGS == 1
#error "Venom supports only AArch64. Compile with CTX_INCLUDE_AARCH32_REGS=0"
#endif

cpu_reset_prologue venom

cpu_reset_func_start venom
	/* ----------------------------------------------------
	 * Disable speculative loads
	 * ----------------------------------------------------
	 */
	msr	SSBS, xzr
	enable_mpmm
cpu_reset_func_end venom

func venom_core_pwr_dwn
	/* ---------------------------------------------------
	 * Flip CPU power down bit in power control register.
	 * It will be set on powerdown and cleared on wakeup
	 * ---------------------------------------------------
	 */
	sysreg_bit_toggle VENOM_IMP_CPUPWRCTLR_EL1, \
		VENOM_IMP_CPUPWRCTLR_EL1_CORE_PWRDN_EN_BIT
	isb
	signal_pabandon_handled
	ret
endfunc venom_core_pwr_dwn

.section .rodata.venom_regs, "aS"
venom_regs: /* The ASCII list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func venom_cpu_reg_dump
	adr 	x6, venom_regs
	mrs	x8, VENOM_IMP_CPUECTLR_EL1
	ret
endfunc venom_cpu_reg_dump

declare_cpu_ops venom, VENOM_MIDR, \
	venom_reset_func, \
	venom_core_pwr_dwn
