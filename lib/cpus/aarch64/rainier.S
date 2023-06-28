/*
 * Copyright (c) 2020-2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <context.h>
#include <cpu_macros.S>
#include <cpuamu.h>
#include <rainier.h>

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "Rainier CPU must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

/* 64-bit only core */
#if CTX_INCLUDE_AARCH32_REGS == 1
#error "Rainier CPU supports only AArch64. Compile with CTX_INCLUDE_AARCH32_REGS=0"
#endif

/* --------------------------------------------------
 * Disable speculative loads if Rainier supports
 * SSBS.
 *
 * Shall clobber: x0.
 * --------------------------------------------------
 */
func rainier_disable_speculative_loads
	/* Check if the PE implements SSBS */
	mrs	x0, id_aa64pfr1_el1
	tst	x0, #(ID_AA64PFR1_EL1_SSBS_MASK << ID_AA64PFR1_EL1_SSBS_SHIFT)
	b.eq	1f

	/* Disable speculative loads */
	msr	SSBS, xzr

1:
	ret
endfunc rainier_disable_speculative_loads

	/* Rainier R0P0 is based on Neoverse N1 R4P0. */
workaround_reset_start rainier, ERRATUM(1868343), ERRATA_N1_1868343
	sysreg_bit_set RAINIER_CPUACTLR_EL1, RAINIER_CPUACTLR_EL1_BIT_13
workaround_reset_end rainier, ERRATUM(1868343)

check_erratum_ls rainier, ERRATUM(1868343), CPU_REV(0, 0)

cpu_reset_func_start rainier
	bl	rainier_disable_speculative_loads
	/* Forces all cacheable atomic instructions to be near */
	sysreg_bit_set RAINIER_CPUACTLR2_EL1, RAINIER_CPUACTLR2_EL1_BIT_2

#if ENABLE_FEAT_AMU
	/* Make sure accesses from EL0/EL1 and EL2 are not trapped to EL3 */
	sysreg_bit_set actlr_el3, RAINIER_ACTLR_AMEN_BIT

	/* Make sure accesses from EL0/EL1 are not trapped to EL2 */
	sysreg_bit_set actlr_el2, RAINIER_ACTLR_AMEN_BIT

	/* Enable group0 counters */
	mov	x0, #RAINIER_AMU_GROUP0_MASK
	msr	CPUAMCNTENSET_EL0, x0
#endif
cpu_reset_func_end rainier

	/* ---------------------------------------------
	 * HW will do the cache maintenance while powering down
	 * ---------------------------------------------
	 */
func rainier_core_pwr_dwn
	/* ---------------------------------------------
	 * Enable CPU power down bit in power control register
	 * ---------------------------------------------
	 */
	 sysreg_bit_set RAINIER_CPUPWRCTLR_EL1, RAINIER_CORE_PWRDN_EN_MASK
	isb
	ret
endfunc rainier_core_pwr_dwn

errata_report_shim rainier

	/* ---------------------------------------------
	 * This function provides Rainier specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.rainier_regs, "aS"
rainier_regs:  /* The ascii list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func rainier_cpu_reg_dump
	adr	x6, rainier_regs
	mrs	x8, RAINIER_CPUECTLR_EL1
	ret
endfunc rainier_cpu_reg_dump

declare_cpu_ops rainier, RAINIER_MIDR, \
	rainier_reset_func, \
	rainier_core_pwr_dwn
