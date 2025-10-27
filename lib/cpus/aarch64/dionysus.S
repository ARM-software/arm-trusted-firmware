/*
 * Copyright (c) 2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <dionysus.h>
#include <cpu_macros.S>
#include <plat_macros.S>

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "Dionysus must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

/* 64-bit only core */
#if CTX_INCLUDE_AARCH32_REGS == 1
#error "Dionysus supports only AArch64. Compile with CTX_INCLUDE_AARCH32_REGS=0"
#endif

cpu_reset_prologue dionysus

cpu_reset_func_start dionysus
	/* ----------------------------------------------------
	 * Disable speculative loads
	 * ----------------------------------------------------
	 */
	msr	SSBS, xzr
	enable_mpmm
cpu_reset_func_end dionysus

	/* ----------------------------------------------------
	 * HW will do the cache maintenance while powering down
	 * ----------------------------------------------------
	 */
func dionysus_core_pwr_dwn
	/* ---------------------------------------------------
	 * Flip CPU power down bit in power control register.
	 * It will be set on powerdown and cleared on wakeup
	 * ---------------------------------------------------
	 */
	sysreg_bit_toggle DIONYSUS_CPUPWRCTLR_EL1, \
		DIONYSUS_CPUPWRCTLR_EL1_CORE_PWRDN_BIT
	isb
	signal_pabandon_handled
	ret
endfunc dionysus_core_pwr_dwn

	/* ---------------------------------------------
	 * This function provides Dionysus specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.dionysus_regs, "aS"
dionysus_regs: /* The ASCII list of register names to be reported */
	.asciz	"imp_cpuectlr_el1", ""

func dionysus_cpu_reg_dump
	adr	x6, dionysus_regs
	mrs	x8, DIONYSUS_IMP_CPUECTLR_EL1
	ret
endfunc dionysus_cpu_reg_dump

declare_cpu_ops dionysus, DIONYSUS_MIDR, \
	dionysus_reset_func, \
	dionysus_core_pwr_dwn
