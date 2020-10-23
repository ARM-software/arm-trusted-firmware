/*
 * Copyright (c) 2020, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <cpu_macros.S>
#include <neoverse_n2.h>

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "Neoverse N2 must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

/* 64-bit only core */
#if CTX_INCLUDE_AARCH32_REGS == 1
#error "Neoverse-N2 supports only AArch64. Compile with CTX_INCLUDE_AARCH32_REGS=0"
#endif

	/* -------------------------------------------------
	 * The CPU Ops reset function for Neoverse N2.
	 * -------------------------------------------------
	 */
func neoverse_n2_reset_func
	/* Check if the PE implements SSBS */
	mrs	x0, id_aa64pfr1_el1
	tst	x0, #(ID_AA64PFR1_EL1_SSBS_MASK << ID_AA64PFR1_EL1_SSBS_SHIFT)
	b.eq	1f

	/* Disable speculative loads */
	msr	SSBS, xzr
1:
	/* Force all cacheable atomic instructions to be near */
	mrs	x0, NEOVERSE_N2_CPUACTLR2_EL1
	orr	x0, x0, #NEOVERSE_N2_CPUACTLR2_EL1_BIT_2
	msr	NEOVERSE_N2_CPUACTLR2_EL1, x0

#if ENABLE_AMU
	/* Make sure accesses from EL0/EL1 and EL2 are not trapped to EL3 */
	mrs	x0, cptr_el3
	orr	x0, x0, #TAM_BIT
	msr	cptr_el3, x0

	/* Make sure accesses from EL0/EL1 are not trapped to EL2 */
	mrs	x0, cptr_el2
	orr	x0, x0, #TAM_BIT
	msr	cptr_el2, x0

	/* No need to enable the counters as this would be done at el3 exit */
#endif

#if NEOVERSE_Nx_EXTERNAL_LLC
	/* Some systems may have External LLC, core needs to be made aware */
	mrs     x0, NEOVERSE_N2_CPUECTLR_EL1
	orr     x0, x0, NEOVERSE_N2_CPUECTLR_EL1_EXTLLC_BIT
	msr     NEOVERSE_N2_CPUECTLR_EL1, x0
#endif

	isb
	ret
endfunc neoverse_n2_reset_func

func neoverse_n2_core_pwr_dwn
	/* ---------------------------------------------
	 * Enable CPU power down bit in power control register
	 * No need to do cache maintenance here.
	 * ---------------------------------------------
	 */
	mrs	x0, NEOVERSE_N2_CPUPWRCTLR_EL1
	orr	x0, x0, #NEOVERSE_N2_CORE_PWRDN_EN_BIT
	msr	NEOVERSE_N2_CPUPWRCTLR_EL1, x0
	isb
	ret
endfunc neoverse_n2_core_pwr_dwn

#if REPORT_ERRATA
/*
 * Errata printing function for Neoverse N2 cores. Must follow AAPCS.
 */
func neoverse_n2_errata_report
	/* No errata reported for Neoverse N2 cores */
	ret
endfunc neoverse_n2_errata_report
#endif

	/* ---------------------------------------------
	 * This function provides Neoverse N2 specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ASCII and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.neoverse_n2_regs, "aS"
neoverse_n2_regs:  /* The ASCII list of register names to be reported */
	.asciz	"cpupwrctlr_el1", ""

func neoverse_n2_cpu_reg_dump
	adr	x6, neoverse_n2_regs
	mrs	x8, NEOVERSE_N2_CPUPWRCTLR_EL1
	ret
endfunc neoverse_n2_cpu_reg_dump

declare_cpu_ops neoverse_n2, NEOVERSE_N2_MIDR, \
	neoverse_n2_reset_func, \
	neoverse_n2_core_pwr_dwn
