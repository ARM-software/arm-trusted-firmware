/*
 * Copyright (c) 2023-2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <c1_nano.h>
#include <common/bl_common.h>
#include <cpu_macros.S>

#include <plat_macros.S>

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "Arm C1-Nano must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

/* 64-bit only core */
#if CTX_INCLUDE_AARCH32_REGS == 1
#error "Arm C1-Nano supports only AArch64. Compile with CTX_INCLUDE_AARCH32_REGS=0"
#endif

#if ERRATA_SME_POWER_DOWN == 0
#error "Arm C1-Nano needs ERRATA_SME_POWER_DOWN=1 to powerdown correctly"
#endif

cpu_reset_prologue c1_nano

cpu_reset_func_start c1_nano
	/* ----------------------------------------------------
	 * Disable speculative loads
	 * ----------------------------------------------------
	 */
	msr	SSBS, xzr
	enable_mpmm
cpu_reset_func_end c1_nano

func c1_nano_core_pwr_dwn
	/* ---------------------------------------------------
	 * Enable CPU power down bit in power control register
	 * ---------------------------------------------------
	 */
	sysreg_bit_set C1_NANO_IMP_CPUPWRCTLR_EL1, \
		C1_NANO_IMP_CPUPWRCTLR_EL1_CORE_PWRDN_EN_BIT
	isb
	ret
endfunc c1_nano_core_pwr_dwn

.section .rodata.c1_nano_regs, "aS"
c1_nano_regs: /* The ASCII list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func c1_nano_cpu_reg_dump
	adr 	x6, c1_nano_regs
	mrs	x8, C1_NANO_IMP_CPUECTLR_EL1
	ret
endfunc c1_nano_cpu_reg_dump

declare_cpu_ops c1_nano, C1_NANO_MIDR, \
	c1_nano_reset_func, \
	c1_nano_core_pwr_dwn
