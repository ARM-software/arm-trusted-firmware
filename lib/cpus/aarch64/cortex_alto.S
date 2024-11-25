/*
 * Copyright (c) 2024-2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <cortex_alto.h>
#include <cpu_macros.S>
#include <plat_macros.S>

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "Alto must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

/* 64-bit only core */
#if CTX_INCLUDE_AARCH32_REGS == 1
#error "Alto supports only AArch64. Compile with CTX_INCLUDE_AARCH32_REGS=0"
#endif

#if ERRATA_SME_POWER_DOWN == 0
#error "Travis needs ERRATA_SME_POWER_DOWN=1 to powerdown correctly"
#endif

cpu_reset_prologue cortex_alto

cpu_reset_func_start cortex_alto
	/* Disable speculative loads */
	msr	SSBS, xzr
	enable_mpmm
cpu_reset_func_end cortex_alto

func cortex_alto_core_pwr_dwn
	/* ---------------------------------------------------
	 * Enable CPU power down bit in power control register
	 * ---------------------------------------------------
	 */
	sysreg_bit_set CORTEX_ALTO_IMP_CPUPWRCTLR_EL1, \
		CORTEX_ALTO_IMP_CPUPWRCTLR_EL1_CORE_PWRDN_EN_BIT
	isb
	ret
endfunc cortex_alto_core_pwr_dwn

.section .rodata.cortex_alto_regs, "aS"
cortex_alto_regs: /* The ASCII list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func cortex_alto_cpu_reg_dump
	adr 	x6, cortex_alto_regs
	mrs	x8, CORTEX_ALTO_IMP_CPUECTLR_EL1
	ret
endfunc cortex_alto_cpu_reg_dump

declare_cpu_ops cortex_alto, CORTEX_ALTO_MIDR, \
	cortex_alto_reset_func, \
	cortex_alto_core_pwr_dwn
