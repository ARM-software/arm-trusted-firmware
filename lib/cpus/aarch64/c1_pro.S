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

workaround_runtime_start c1_pro, ERRATUM(3338470), ERRATA_C1PRO_3338470
	speculation_barrier
workaround_runtime_end c1_pro, ERRATUM(3338470)

check_erratum_ls c1_pro, ERRATUM(3338470), CPU_REV(0, 0)

workaround_reset_start c1_pro, ERRATUM(3362007), ERRATA_C1PRO_3362007
	sysreg_bit_set C1_PRO_IMP_CPUACTLR2_EL1, BIT(27)
workaround_reset_end c1_pro, ERRATUM(3362007)

check_erratum_ls c1_pro, ERRATUM(3362007), CPU_REV(0, 0)

workaround_reset_start c1_pro, ERRATUM(3619847), ERRATA_C1PRO_3619847
	sysreg_bit_set C1_PRO_IMP_CPUACTLR2_EL1, BIT(42)
workaround_reset_end c1_pro, ERRATUM(3619847)

check_erratum_ls c1_pro, ERRATUM(3619847), CPU_REV(0, 0)

workaround_reset_start c1_pro, ERRATUM(3684268), ERRATA_C1PRO_3684268
	sysreg_bit_set C1_PRO_IMP_CPUECTLR2_EL1, BIT(49)
	dsb sy
workaround_reset_end c1_pro, ERRATUM(3684268)

check_erratum_ls c1_pro, ERRATUM(3684268), CPU_REV(1, 0)

workaround_runtime_start c1_pro, ERRATUM(3686597), ERRATA_C1PRO_3686597
#if ENABLE_SME_FOR_NS
#if ENABLE_SME_FOR_NS == 2
	is_feat_sme_present_asm x1
	beq 1f
#endif

	sysreg_bit_set C1_PRO_IMP_CPUECTLR_EL1, BIT(57)
	dsb sy

1:
#endif
workaround_runtime_end c1_pro, ERRATUM(3686597)

check_erratum_ls c1_pro, ERRATUM(3686597), CPU_REV(1, 0)

workaround_reset_start c1_pro, ERRATUM(3694158), ERRATA_C1PRO_3694158
	mov x0, #5
	msr C1_PRO_IMP_CPUPSELR_EL3, x0
	isb
	ldr x0, =0xd503329f
	msr C1_PRO_IMP_CPUPOR_EL3, x0
	ldr x0, =0xfffff3ff
	msr C1_PRO_IMP_CPUPMR_EL3, x0
	mov x1, #0
	orr x1, x1, #1<<0
	orr x1, x1, #3<<4
	orr x1, x1, #0xf<<6
	orr x1, x1, #1<<22
	orr x1, x1, #1<<32
	msr C1_PRO_IMP_CPUPCR_EL3, x1
workaround_reset_end c1_pro, ERRATUM(3694158)

check_erratum_ls c1_pro, ERRATUM(3694158), CPU_REV(1, 1)

workaround_reset_start c1_pro, ERRATUM(3706576), ERRATA_C1PRO_3706576
	sysreg_bit_set C1_PRO_IMP_CPUACTLR2_EL1, BIT(37)
workaround_reset_end c1_pro, ERRATUM(3706576)

check_erratum_ls c1_pro, ERRATUM(3706576), CPU_REV(1, 0)

add_erratum_entry c1_pro, ERRATUM(3300099), ERRATA_C1PRO_3300099
.global check_erratum_c1_pro_3300099
check_erratum_ls c1_pro, ERRATUM(3300099), CPU_REV(1, 0)

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
	apply_erratum c1_pro, ERRATUM(3338470), ERRATA_C1PRO_3338470
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
	apply_erratum c1_pro, ERRATUM(3686597), ERRATA_C1PRO_3686597
	sysreg_bit_toggle C1_PRO_IMP_CPUPWRCTLR_EL1, \
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
