/*
 * Copyright (c) 2021, ARM Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <cortex_matterhorn_elp_arm.h>
#include <cpu_macros.S>
#include <plat_macros.S>

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "Cortex Matterhorn ELP ARM must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

/* 64-bit only core */
#if CTX_INCLUDE_AARCH32_REGS == 1
#error "Cortex Matterhorn ELP ARM supports only AArch64. Compile with CTX_INCLUDE_AARCH32_REGS=0"
#endif

	/* ----------------------------------------------------
	 * HW will do the cache maintenance while powering down
	 * ----------------------------------------------------
	 */
func cortex_matterhorn_elp_arm_core_pwr_dwn
	/* ---------------------------------------------------
	 * Enable CPU power down bit in power control register
	 * ---------------------------------------------------
	 */
	mrs	x0, CORTEX_MATTERHORN_ELP_ARM_CPUPWRCTLR_EL1
	orr	x0, x0, #CORTEX_MATTERHORN_ELP_ARM_CPUPWRCTLR_EL1_CORE_PWRDN_BIT
	msr	CORTEX_MATTERHORN_ELP_ARM_CPUPWRCTLR_EL1, x0
	isb
	ret
endfunc cortex_matterhorn_elp_arm_core_pwr_dwn

	/*
	 * Errata printing function for Cortex Matterhorn_elp_arm. Must follow AAPCS.
	 */
#if REPORT_ERRATA
func cortex_matterhorn_elp_arm_errata_report
	ret
endfunc cortex_matterhorn_elp_arm_errata_report
#endif

func cortex_matterhorn_elp_arm_reset_func
	/* Disable speculative loads */
	msr	SSBS, xzr
	isb
	ret
endfunc cortex_matterhorn_elp_arm_reset_func

	/* ---------------------------------------------
	 * This function provides Cortex-Matterhorn_elp_arm specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.cortex_matterhorn_elp_arm_regs, "aS"
cortex_matterhorn_elp_arm_regs:  /* The ascii list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func cortex_matterhorn_elp_arm_cpu_reg_dump
	adr	x6, cortex_matterhorn_elp_arm_regs
	mrs	x8, CORTEX_MATTERHORN_ELP_ARM_CPUECTLR_EL1
	ret
endfunc cortex_matterhorn_elp_arm_cpu_reg_dump

declare_cpu_ops cortex_matterhorn_elp_arm, CORTEX_MATTERHORN_ELP_ARM_MIDR, \
	cortex_matterhorn_elp_arm_reset_func, \
	cortex_matterhorn_elp_arm_core_pwr_dwn
