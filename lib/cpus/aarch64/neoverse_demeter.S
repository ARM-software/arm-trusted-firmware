/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <neoverse_demeter.h>
#include <cpu_macros.S>
#include <plat_macros.S>

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "Neoverse Demeter must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

/* 64-bit only core */
#if CTX_INCLUDE_AARCH32_REGS == 1
#error "Neoverse Demeter supports only AArch64. Compile with CTX_INCLUDE_AARCH32_REGS=0"
#endif

	/* ----------------------------------------------------
	 * HW will do the cache maintenance while powering down
	 * ----------------------------------------------------
	 */
func neoverse_demeter_core_pwr_dwn
	/* ---------------------------------------------------
	 * Enable CPU power down bit in power control register
	 * ---------------------------------------------------
	 */
	mrs	x0, NEOVERSE_DEMETER_CPUPWRCTLR_EL1
	orr	x0, x0, #NEOVERSE_DEMETER_CPUPWRCTLR_EL1_CORE_PWRDN_BIT
	msr	NEOVERSE_DEMETER_CPUPWRCTLR_EL1, x0
	isb
	ret
endfunc neoverse_demeter_core_pwr_dwn

#if REPORT_ERRATA
/*
 * Errata printing function for Neoverse Demeter. Must follow AAPCS.
 */
func neoverse_demeter_errata_report
	ret
endfunc neoverse_demeter_errata_report
#endif

func neoverse_demeter_reset_func
	/* Disable speculative loads */
	msr	SSBS, xzr
	isb
	ret
endfunc neoverse_demeter_reset_func

	/* ---------------------------------------------
	 * This function provides Neoverse Demeter-
	 * specific register information for crash
	 * reporting. It needs to return with x6
	 * pointing to a list of register names in ascii
	 * and x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.neoverse_demeter_regs, "aS"
neoverse_demeter_regs:  /* The ascii list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func neoverse_demeter_cpu_reg_dump
	adr	x6, neoverse_demeter_regs
	mrs	x8, NEOVERSE_DEMETER_CPUECTLR_EL1
	ret
endfunc neoverse_demeter_cpu_reg_dump

declare_cpu_ops neoverse_demeter, NEOVERSE_DEMETER_MIDR, \
	neoverse_demeter_reset_func, \
	neoverse_demeter_core_pwr_dwn
