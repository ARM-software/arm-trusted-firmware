/*
 * Copyright (c) 2023-2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <c1_pro.h>
#include <common/bl_common.h>
#include <cpu_macros.S>

#include <plat_macros.S>

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "Arm C1-Pro must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

/* 64-bit only core */
#if CTX_INCLUDE_AARCH32_REGS == 1
#error "Arm C1-Pro supports only AArch64. Compile with CTX_INCLUDE_AARCH32_REGS=0"
#endif

#if ERRATA_SME_POWER_DOWN == 0
#error "Arm C1-Pro needs ERRATA_SME_POWER_DOWN=1 to powerdown correctly"
#endif

cpu_reset_prologue c1_pro

	/* -----------------------------------------------------------
	 * CVE-2024-7881 is mitigated for C1-Pro using erratum 3684268
	 * workaround by disabling the affected prefetcher
	 * via IMP_CPUECTLR_EL1[49].
	 * -----------------------------------------------------------
	 */
workaround_reset_start c1_pro, CVE(2024, 7881), WORKAROUND_CVE_2024_7881
	sysreg_bit_set C1_PRO_IMP_CPUECTLR_EL1, BIT(49)
	dsb	sy
workaround_reset_end c1_pro, CVE(2024, 7881)

check_erratum_ls c1_pro, CVE(2024, 7881), CPU_REV(1, 0)

cpu_reset_func_start c1_pro
	/* ----------------------------------------------------
	 * Disable speculative loads
	 * ----------------------------------------------------
	 */
	msr	SSBS, xzr
	/* model bug: not cleared on reset */
	sysreg_bit_clear 	C1_PRO_IMP_CPUPWRCTLR_EL1, \
		C1_PRO_IMP_CPUPWRCTLR_EL1_CORE_PWRDN_BIT
	enable_mpmm
cpu_reset_func_end c1_pro

	/* ----------------------------------------------------
	 * HW will do the cache maintenance while powering down
	 * ----------------------------------------------------
	 */
func c1_pro_core_pwr_dwn
	/* ---------------------------------------------------
	 * Flip CPU power down bit in power control register.
	 * It will be set on powerdown and cleared on wakeup
	 * ---------------------------------------------------
	 */
	sysreg_bit_set 	C1_PRO_IMP_CPUPWRCTLR_EL1, \
		C1_PRO_IMP_CPUPWRCTLR_EL1_CORE_PWRDN_BIT
	isb
	signal_pabandon_handled
	ret
endfunc c1_pro_core_pwr_dwn

	/* ---------------------------------------------
	 * This function provides Arm C1-Pro specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.c1_pro_regs, "aS"
c1_pro_regs: /* The ASCII list of register names to be reported */
	.asciz	"imp_cpuectlr_el1", ""

func c1_pro_cpu_reg_dump
	adr	x6, c1_pro_regs
	mrs	x8, C1_PRO_IMP_CPUECTLR_EL1
	ret
endfunc c1_pro_cpu_reg_dump

declare_cpu_ops c1_pro, C1_PRO_MIDR, \
	c1_pro_reset_func, \
	c1_pro_core_pwr_dwn
