/*
 * Copyright (c) 2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <veymont.h>
#include <cpu_macros.S>
#include <plat_macros.S>

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "Veymont must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

/* 64-bit only core */
#if CTX_INCLUDE_AARCH32_REGS == 1
#error "Veymont supports only AArch64. Compile with CTX_INCLUDE_AARCH32_REGS=0"
#endif

#if ERRATA_SME_POWER_DOWN == 0
#error "Veymont needs ERRATA_SME_POWER_DOWN=1 to powerdown correctly"
#endif

cpu_reset_prologue veymont

cpu_reset_func_start veymont
	/* ----------------------------------------------------
	 * Disable speculative loads
	 * ----------------------------------------------------
	 */
	msr	SSBS, xzr
	enable_mpmm
cpu_reset_func_end veymont

func veymont_core_pwr_dwn
	/* ---------------------------------------------------
	 * Flip CPU power down bit in power control register.
	 * It will be set on powerdown and cleared on wakeup
	 * ---------------------------------------------------
	 */
	sysreg_bit_toggle VEYMONT_IMP_CPUPWRCTLR_EL1, \
		VEYMONT_IMP_CPUPWRCTLR_EL1_CORE_PWRDN_EN_BIT
	isb
	signal_pabandon_handled
	ret
endfunc veymont_core_pwr_dwn

.section .rodata.veymont_regs, "aS"
veymont_regs: /* The ASCII list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func veymont_cpu_reg_dump
	adr 	x6, veymont_regs
	mrs	x8, VEYMONT_IMP_CPUECTLR_EL1
	ret
endfunc veymont_cpu_reg_dump

declare_cpu_ops veymont, VEYMONT_MIDR, \
	veymont_reset_func, \
	veymont_core_pwr_dwn
