/*
 * Copyright (c) 2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <canyon.h>
#include <cpu_macros.S>
#include <plat_macros.S>

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "Canyon must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

/* 64-bit only core */
#if CTX_INCLUDE_AARCH32_REGS == 1
#error "Canyon supports only AArch64. Compile with CTX_INCLUDE_AARCH32_REGS=0"
#endif

#if ERRATA_SME_POWER_DOWN == 0
#error "Canyon needs ERRATA_SME_POWER_DOWN=1 to powerdown correctly"
#endif

cpu_reset_prologue canyon

cpu_reset_func_start canyon
	/* ----------------------------------------------------
	 * Disable speculative loads
	 * ----------------------------------------------------
	 */
	msr	SSBS, xzr
cpu_reset_func_end canyon

func canyon_core_pwr_dwn
	/* ---------------------------------------------------
	 * Flip CPU power down bit in power control register.
	 * It will be set on powerdown and cleared on wakeup
	 * ---------------------------------------------------
	 */
	sysreg_bit_toggle CANYON_IMP_CPUPWRCTLR_EL1, \
		CANYON_IMP_CPUPWRCTLR_EL1_CORE_PWRDN_EN_BIT
	isb
	signal_pabandon_handled
	ret
endfunc canyon_core_pwr_dwn

.section .rodata.canyon_regs, "aS"
canyon_regs: /* The ASCII list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func canyon_cpu_reg_dump
	adr 	x6, canyon_regs
	mrs	x8, CANYON_IMP_CPUECTLR_EL1
	ret
endfunc canyon_cpu_reg_dump

declare_cpu_ops canyon, CANYON_MIDR, \
	canyon_reset_func, \
	canyon_core_pwr_dwn
