/*
 * Copyright (c) 2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <cortex_gelas.h>
#include <cpu_macros.S>
#include <plat_macros.S>

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "Gelas must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

/* 64-bit only core */
#if CTX_INCLUDE_AARCH32_REGS == 1
#error "Gelas supports only AArch64. Compile with CTX_INCLUDE_AARCH32_REGS=0"
#endif

cpu_reset_func_start cortex_gelas
	/* ----------------------------------------------------
	 * Disable speculative loads
	 * ----------------------------------------------------
	 */
	msr	SSBS, xzr
cpu_reset_func_end cortex_gelas

	/* ----------------------------------------------------
	 * HW will do the cache maintenance while powering down
	 * ----------------------------------------------------
	 */
func cortex_gelas_core_pwr_dwn
#if ENABLE_SME_FOR_NS
        /* ---------------------------------------------------
         * Disable SME if enabled and supported
         * ---------------------------------------------------
         */
	mrs     x0, ID_AA64PFR1_EL1
	ubfx	x0, x0, #ID_AA64PFR1_EL1_SME_SHIFT, \
		#ID_AA64PFR1_EL1_SME_WIDTH
        cmp     x0, #ID_AA64PFR1_EL1_SME_NOT_SUPPORTED
	b.eq	1f
	msr	CORTEX_GELAS_SVCRSM, xzr
	msr	CORTEX_GELAS_SVCRZA, xzr
1:
#endif
	/* ---------------------------------------------------
	 * Enable CPU power down bit in power control register
	 * ---------------------------------------------------
	 */
	sysreg_bit_set 	CORTEX_GELAS_CPUPWRCTLR_EL1, \
		CORTEX_GELAS_CPUPWRCTLR_EL1_CORE_PWRDN_BIT
	isb
	ret
endfunc cortex_gelas_core_pwr_dwn

errata_report_shim cortex_gelas

	/* ---------------------------------------------
	 * This function provides Gelas specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.cortex_gelas_regs, "aS"
cortex_gelas_regs: /* The ASCII list of register names to be reported */
	.asciz	"imp_cpuectlr_el1", ""

func cortex_gelas_cpu_reg_dump
	adr	x6, cortex_gelas_regs
	mrs	x8, CORTEX_GELAS_IMP_CPUECTLR_EL1
	ret
endfunc cortex_gelas_cpu_reg_dump

declare_cpu_ops cortex_gelas, CORTEX_GELAS_MIDR, \
	cortex_gelas_reset_func, \
	cortex_gelas_core_pwr_dwn
