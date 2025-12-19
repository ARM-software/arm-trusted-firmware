/*
 * Copyright (c) 2023-2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <c1_nano.h>
#include <common/bl_common.h>
#include <cpu_macros.S>

#include <plat_macros.S>

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "Arm C1-Nano must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

/* 64-bit only core */
#if CTX_INCLUDE_AARCH32_REGS == 1
#error "Arm C1-Nano supports only AArch64. Compile with CTX_INCLUDE_AARCH32_REGS=0"
#endif

#if ERRATA_SME_POWER_DOWN == 0
#error "Arm C1-Nano needs ERRATA_SME_POWER_DOWN=1 to powerdown correctly"
#endif

cpu_reset_prologue c1_nano

workaround_reset_start c1_nano, ERRATUM(3392149), ERRATA_C1NANO_3392149
	sysreg_bit_set C1_NANO_IMP_CPUACTLR3_EL1, BIT(39)
workaround_reset_end c1_nano, ERRATUM(3392149)

check_erratum_ls c1_nano, ERRATUM(3392149), CPU_REV(0, 0)

workaround_reset_start c1_nano, ERRATUM(3419531), ERRATA_C1NANO_3419531
	sysreg_bit_set C1_NANO_IMP_CPUACTLR_EL1, BIT(27)
workaround_reset_end c1_nano, ERRATUM(3419531)

check_erratum_ls c1_nano, ERRATUM(3419531), CPU_REV(0, 0)

workaround_reset_start c1_nano, ERRATUM(3437202), ERRATA_C1NANO_3437202
	sysreg_bit_set C1_NANO_IMP_CPUACTLR_EL1, BIT(26)
workaround_reset_end c1_nano, ERRATUM(3437202)

check_erratum_ls c1_nano, ERRATUM(3437202), CPU_REV(0, 0)

workaround_reset_start c1_nano, ERRATUM(3516455), ERRATA_C1NANO_3516455
#if ENABLE_SME_FOR_NS
#if ENABLE_SME_FOR_NS == 2
	is_feat_sme_present_asm x1
	beq 1f
#endif

	mov x0, #0
	msr C1_NANO_IMP_CPUPSELR_EL3, x0
	isb
	ldr x0, =0xA0008000
	msr C1_NANO_IMP_CPUPOR_EL3, x0
	ldr x0, =0xFE808000
	msr C1_NANO_IMP_CPUPMR_EL3, x0
	ldr x0, =0x7F9
	movk x0, #0x20, LSL #32
	msr C1_NANO_IMP_CPUPCR_EL3, x0
	isb
	mov x0, #1
	msr C1_NANO_IMP_CPUPSELR_EL3, x0
	isb
	ldr x0, =0xA4604000
	msr C1_NANO_IMP_CPUPOR_EL3, x0
	ldr x0, =0xBE604000
	msr C1_NANO_IMP_CPUPMR_EL3, x0
	ldr x0, =0x7F9
	movk x0, #0x20, LSL #32
	msr C1_NANO_IMP_CPUPCR_EL3, x0

	1:
#endif
workaround_reset_end c1_nano, ERRATUM(3516455)

check_erratum_ls c1_nano, ERRATUM(3516455), CPU_REV(0, 0)

workaround_reset_start c1_nano, ERRATUM(3616450), ERRATA_C1NANO_3616450
#if ENABLE_SME_FOR_NS
#if ENABLE_SME_FOR_NS == 2
	is_feat_sme_present_asm x1
	beq 1f
#endif

	sysreg_bit_set C1_NANO_IMP_CPUACTLR_EL1, BIT(29)

	1:
#endif
workaround_reset_end c1_nano, ERRATUM(3616450)

check_erratum_ls c1_nano, ERRATUM(3616450), CPU_REV(0, 0)

workaround_reset_start c1_nano, ERRATUM(3630925), ERRATA_C1NANO_3630925
	sysreg_bitfield_insert C1_NANO_IMP_CPUPWRCTLR_EL1, \
	C1_NANO_IMP_CPUPWRCTLR_EL1_WFE_RET_CTRL_BIT, \
	C1_NANO_IMP_CPUPWRCTLR_EL1_WFE_RET_CTRL_SHIFT, \
	C1_NANO_IMP_CPUPWRCTLR_EL1_WFE_RET_CTRL_WIDTH

	sysreg_bitfield_insert C1_NANO_IMP_CPUPWRCTLR_EL1, \
	C1_NANO_IMP_CPUPWRCTLR_EL1_WFI_RET_CTRL_BIT, \
	C1_NANO_IMP_CPUPWRCTLR_EL1_WFI_RET_CTRL_SHIFT, \
	C1_NANO_IMP_CPUPWRCTLR_EL1_WFI_RET_CTRL_WIDTH
workaround_reset_end c1_nano, ERRATUM(3630925)

check_erratum_ls c1_nano, ERRATUM(3630925), CPU_REV(0, 0)

workaround_runtime_start c1_nano, ERRATUM(3754876), ERRATA_C1NANO_3754876
	tsb csync
workaround_runtime_end c1_nano, ERRATUM(3754876)

check_erratum_ls c1_nano, ERRATUM(3754876), CPU_REV(0, 1)

cpu_reset_func_start c1_nano
	/* ----------------------------------------------------
	 * Disable speculative loads
	 * ----------------------------------------------------
	 */
	msr	SSBS, xzr
	/* model bug: not cleared on reset */
	sysreg_bit_clear 	C1_NANO_IMP_CPUPWRCTLR_EL1, \
		C1_NANO_IMP_CPUPWRCTLR_EL1_CORE_PWRDN_EN_BIT
	enable_mpmm
cpu_reset_func_end c1_nano

func c1_nano_core_pwr_dwn
	/* ---------------------------------------------------
	 * Enable CPU power down bit in power control register
	 * ---------------------------------------------------
	 */
	sysreg_bit_toggle C1_NANO_IMP_CPUPWRCTLR_EL1, \
		C1_NANO_IMP_CPUPWRCTLR_EL1_CORE_PWRDN_EN_BIT
	isb
	apply_erratum c1_nano, ERRATUM(3754876), ERRATA_C1NANO_3754876
	signal_pabandon_handled
	ret
endfunc c1_nano_core_pwr_dwn

.section .rodata.c1_nano_regs, "aS"
c1_nano_regs: /* The ASCII list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func c1_nano_cpu_reg_dump
	adr 	x6, c1_nano_regs
	mrs	x8, C1_NANO_IMP_CPUECTLR_EL1
	ret
endfunc c1_nano_cpu_reg_dump

declare_cpu_ops c1_nano, C1_NANO_MIDR, \
	c1_nano_reset_func, \
	c1_nano_core_pwr_dwn
