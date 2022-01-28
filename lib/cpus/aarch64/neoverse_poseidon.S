/*
 * Copyright (c) 2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <neoverse_poseidon.h>
#include <cpu_macros.S>
#include <plat_macros.S>

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "Neoverse Poseidon must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

/* 64-bit only core */
#if CTX_INCLUDE_AARCH32_REGS == 1
#error "Neoverse Poseidon supports only AArch64. Compile with CTX_INCLUDE_AARCH32_REGS=0"
#endif

	/* ---------------------------------------------
	 * HW will do the cache maintenance while powering down
	 * ---------------------------------------------
	 */
func neoverse_poseidon_core_pwr_dwn
	/* ---------------------------------------------
	 * Enable CPU power down bit in power control register
	 * ---------------------------------------------
	 */
	mrs	x0, NEOVERSE_POSEIDON_CPUPWRCTLR_EL1
	orr	x0, x0, #NEOVERSE_POSEIDON_CPUPWRCTLR_EL1_CORE_PWRDN_BIT
	msr	NEOVERSE_POSEIDON_CPUPWRCTLR_EL1, x0
	isb
	ret
endfunc neoverse_poseidon_core_pwr_dwn

#if REPORT_ERRATA
	/*
	 * Errata printing function for Neoverse Poseidon. Must follow AAPCS.
	 */
func neoverse_poseidon_errata_report
	ret
endfunc neoverse_poseidon_errata_report
#endif

func neoverse_poseidon_reset_func
	/* Disable speculative loads */
	msr	SSBS, xzr
	isb
	ret
endfunc neoverse_poseidon_reset_func

	/* ---------------------------------------------
	 * This function provides Neoverse-Poseidon specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.neoverse_poseidon_regs, "aS"
neoverse_poseidon_regs:  /* The ascii list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func neoverse_poseidon_cpu_reg_dump
	adr	x6, neoverse_poseidon_regs
	mrs	x8, NEOVERSE_POSEIDON_CPUECTLR_EL1
	ret
endfunc neoverse_poseidon_cpu_reg_dump

declare_cpu_ops neoverse_poseidon, NEOVERSE_POSEIDON_MIDR, \
	neoverse_poseidon_reset_func, \
	neoverse_poseidon_core_pwr_dwn
