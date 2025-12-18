/*
 * Copyright (c) 2024-2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <c1_premium.h>
#include <common/bl_common.h>
#include <cpu_macros.S>

#include <plat_macros.S>

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "Arm C1-Premium must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

/* 64-bit only core */
#if CTX_INCLUDE_AARCH32_REGS == 1
#error "Arm C1-Premium supports only AArch64. Compile with CTX_INCLUDE_AARCH32_REGS=0"
#endif

#if ERRATA_SME_POWER_DOWN == 0
#error "Arm C1-Premium needs ERRATA_SME_POWER_DOWN=1 to powerdown correctly"
#endif

cpu_reset_prologue c1_premium

	/* ---------------------------------------------------------------
	 * CVE-2024-7881 is mitigated for C1-Premium using erratum 3651221
	 * workaround by disabling the affected prefetcher setting
	 * CPUACTLR6_EL1[41].
	 * ---------------------------------------------------------------
	 */
workaround_reset_start c1_premium, CVE(2024, 7881), WORKAROUND_CVE_2024_7881
	sysreg_bit_set C1_PREMIUM_CPUACTLR6_EL1, BIT(41)
workaround_reset_end c1_premium, CVE(2024, 7881)

check_erratum_ls c1_premium, CVE(2024, 7881), CPU_REV(0, 0)

workaround_runtime_start c1_premium, ERRATUM(3324333), ERRATA_C1PREMIUM_3324333
	speculation_barrier
workaround_runtime_end c1_premium, ERRATUM(3324333)

check_erratum_ls c1_premium, ERRATUM(3324333), CPU_REV(0, 0)

workaround_reset_start c1_premium, ERRATUM(3502731), ERRATA_C1PREMIUM_3502731
	sysreg_bit_set C1_PREMIUM_IMP_CPUACTLR4_EL1, BIT(23)
workaround_reset_end c1_premium, ERRATUM(3502731)

check_erratum_ls c1_premium, ERRATUM(3502731), CPU_REV(0, 0)

workaround_reset_start c1_premium, ERRATUM(3651221), ERRATA_C1PREMIUM_3651221
	sysreg_bit_set C1_PREMIUM_CPUACTLR6_EL1, BIT(41)
workaround_reset_end c1_premium, ERRATUM(3651221)

check_erratum_ls c1_premium, ERRATUM(3651221), CPU_REV(0, 0)

workaround_reset_start c1_premium, ERRATUM(3684152), ERRATA_C1PREMIUM_3684152
	sysreg_bitfield_insert C1_PREMIUM_IMP_CPUACTLR_EL1, C1_PREMIUM_IMP_CPUACTLR_EL1_LOAD_BIT, \
	C1_PREMIUM_IMP_CPUACTLR_EL1_LOAD_SHIFT, C1_PREMIUM_IMP_CPUACTLR_EL1_LOAD_WIDTH
workaround_reset_end c1_premium, ERRATUM(3684152)

check_erratum_ls c1_premium, ERRATUM(3684152), CPU_REV(0, 0)

workaround_reset_start c1_premium, ERRATUM(3705939), ERRATA_C1PREMIUM_3705939
	sysreg_bit_set C1_PREMIUM_IMP_CPUACTLR_EL1, BIT(48)
workaround_reset_end c1_premium, ERRATUM(3705939)

check_erratum_ls c1_premium, ERRATUM(3705939), CPU_REV(1, 0)

workaround_reset_start c1_premium, ERRATUM(3815514), ERRATA_C1PREMIUM_3815514
	sysreg_bit_set C1_PREMIUM_IMP_CPUACTLR5_EL1, BIT(13)
workaround_reset_end c1_premium, ERRATUM(3815514)

check_erratum_ls c1_premium, ERRATUM(3815514), CPU_REV(1, 0)

workaround_reset_start c1_premium, ERRATUM(3865171), ERRATA_C1PREMIUM_3865171
	sysreg_bit_set C1_PREMIUM_IMP_CPUACTLR2_EL1, BIT(22)
workaround_reset_end c1_premium, ERRATUM(3865171)

check_erratum_ls c1_premium, ERRATUM(3865171), CPU_REV(1, 0)

workaround_reset_start c1_premium, ERRATUM(3926381), ERRATA_C1PREMIUM_3926381
	/* Convert WFx to NOP */
	ldr x0,=0x0
	msr C1_PREMIUM_IMP_CPUPSELR_EL3, x0
	ldr x0,=0xD503205f
	msr C1_PREMIUM_IMP_CPUPOR_EL3, x0
	ldr x0,=0xFFFFFFDF
	msr C1_PREMIUM_IMP_CPUPMR_EL3, x0
	ldr x0,=0x1000002043ff
	msr C1_PREMIUM_IMP_CPUPCR_EL3, x0

	/* Convert WFxT to NOP */
	ldr x0,=0x1
	msr C1_PREMIUM_IMP_CPUPSELR_EL3, x0
	ldr x0,=0xD5031000
	msr C1_PREMIUM_IMP_CPUPOR_EL3, x0
	ldr x0,=0xFFFFFFC0
	msr C1_PREMIUM_IMP_CPUPMR_EL3, x0
	ldr x0,=0x1000002043ff
	msr C1_PREMIUM_IMP_CPUPCR_EL3, x0
	isb
workaround_reset_end c1_premium, ERRATUM(3926381)

check_erratum_range c1_premium, ERRATUM(3926381), CPU_REV(1, 0), CPU_REV(1, 0)

workaround_reset_start c1_premium, ERRATUM(4102704), ERRATA_C1PREMIUM_4102704
	sysreg_bit_set C1_PREMIUM_IMP_CPUACTLR4_EL1, BIT(23)
workaround_reset_end c1_premium, ERRATUM(4102704)

check_erratum_ls c1_premium, ERRATUM(4102704), CPU_REV(1, 0)

cpu_reset_func_start c1_premium
	/* Disable speculative loads */
	msr	SSBS, xzr
	apply_erratum c1_premium, ERRATUM(3324333), ERRATA_C1PREMIUM_3324333
	enable_mpmm
cpu_reset_func_end c1_premium

func c1_premium_core_pwr_dwn
	/* ---------------------------------------------------
	 * Flip CPU power down bit in power control register.
	 * It will be set on powerdown and cleared on wakeup.
	 * ---------------------------------------------------
	 */
	sysreg_bit_toggle C1_PREMIUM_IMP_CPUPWRCTLR_EL1, \
		C1_PREMIUM_IMP_CPUPWRCTLR_EL1_CORE_PWRDN_EN_BIT
	isb
	signal_pabandon_handled
	ret
endfunc c1_premium_core_pwr_dwn

.section .rodata.c1_premium_regs, "aS"
c1_premium_regs: /* The ASCII list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func c1_premium_cpu_reg_dump
	adr 	x6, c1_premium_regs
	mrs	x8, C1_PREMIUM_IMP_CPUECTLR_EL1
	ret
endfunc c1_premium_cpu_reg_dump

declare_cpu_ops c1_premium, C1_PREMIUM_MIDR, \
	c1_premium_reset_func, \
	c1_premium_core_pwr_dwn
