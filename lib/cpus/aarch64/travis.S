/*
 * Copyright (c) 2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <travis.h>
#include <cpu_macros.S>
#include <plat_macros.S>

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "Travis must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

/* 64-bit only core */
#if CTX_INCLUDE_AARCH32_REGS == 1
#error "Travis supports only AArch64. Compile with CTX_INCLUDE_AARCH32_REGS=0"
#endif

cpu_reset_func_start travis
	/* ----------------------------------------------------
	 * Disable speculative loads
	 * ----------------------------------------------------
	 */
	msr	SSBS, xzr
cpu_reset_func_end travis

func travis_core_pwr_dwn
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
	msr	TRAVIS_SVCRSM, xzr
	msr	TRAVIS_SVCRZA, xzr
1:
#endif
	/* ---------------------------------------------------
	 * Enable CPU power down bit in power control register
	 * ---------------------------------------------------
	 */
	sysreg_bit_set TRAVIS_IMP_CPUPWRCTLR_EL1, \
		TRAVIS_IMP_CPUPWRCTLR_EL1_CORE_PWRDN_EN_BIT
	isb
	ret
endfunc travis_core_pwr_dwn

errata_report_shim travis

.section .rodata.travis_regs, "aS"
travis_regs: /* The ASCII list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func travis_cpu_reg_dump
	adr 	x6, travis_regs
	mrs	x8, TRAVIS_IMP_CPUECTLR_EL1
	ret
endfunc travis_cpu_reg_dump

declare_cpu_ops travis, TRAVIS_MIDR, \
	travis_reset_func, \
	travis_core_pwr_dwn
