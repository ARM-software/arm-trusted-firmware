/*
 * Copyright (c) 2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <caddo.h>
#include <cpu_macros.S>
#include <plat_macros.S>

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "Caddo must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

/* 64-bit only core */
#if CTX_INCLUDE_AARCH32_REGS == 1
#error "Caddo supports only AArch64. Compile with CTX_INCLUDE_AARCH32_REGS=0"
#endif

#if ERRATA_SME_POWER_DOWN == 0
#error "Caddo needs ERRATA_SME_POWER_DOWN=1 to powerdown correctly"
#endif

cpu_reset_prologue caddo

cpu_reset_func_start caddo
	/* ----------------------------------------------------
	 * Disable speculative loads
	 * ----------------------------------------------------
	 */
	msr	SSBS, xzr
	enable_mpmm
cpu_reset_func_end caddo

func caddo_core_pwr_dwn
	/* ---------------------------------------------------
	 * Flip CPU power down bit in power control register.
	 * It will be set on powerdown and cleared on wakeup
	 * ---------------------------------------------------
	 */
	sysreg_bit_toggle CADDO_IMP_CPUPWRCTLR_EL1, \
		CADDO_IMP_CPUPWRCTLR_EL1_CORE_PWRDN_EN_BIT
	isb
	signal_pabandon_handled
	ret
endfunc caddo_core_pwr_dwn

.section .rodata.caddo_regs, "aS"
caddo_regs: /* The ASCII list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func caddo_cpu_reg_dump
	adr 	x6, caddo_regs
	mrs	x8, CADDO_IMP_CPUECTLR_EL1
	ret
endfunc caddo_cpu_reg_dump

declare_cpu_ops caddo, CADDO_MIDR, \
	caddo_reset_func, \
	caddo_core_pwr_dwn
