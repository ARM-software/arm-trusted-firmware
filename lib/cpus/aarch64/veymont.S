/*
 * Copyright (c) 2025-2026, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <veymont.h>
#include <cpu_macros.S>
#include <plat_macros.S>

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "Veymont must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

/* 64-bit only core */
#if CTX_INCLUDE_AARCH32_REGS == 1
#error "Veymont supports only AArch64. Compile with CTX_INCLUDE_AARCH32_REGS=0"
#endif

cpu_reset_prologue veymont

cpu_reset_func_start veymont
	/* ----------------------------------------------------
	 * Disable speculative loads
	 * ----------------------------------------------------
	 */
	msr	SSBS, xzr
	enable_mpmm
cpu_reset_func_end veymont

func veymont_core_pwr_dwn
	/*
	 * When software running at lower ELs requests power down without first
	 * disabling SME, the CME connected to it will reject its power down
	 * request. Skip setting the PWRDN_EN bit, downgrading the powerdown
	 * request to a simple WFI wait, to get a minimal amount of power saving
	 * rather than an instant pabandon.
	 */
#if ENABLE_SME_FOR_NS
#if ENABLE_SME_FOR_NS == 2
	is_feat_sme_present_asm x1
	beq 1f
#endif
	mrs	x0, SVCR
	cbnz	x0, veymont_skip_pwr_dwn
1:
#endif

	/* ---------------------------------------------------
	 * Flip CPU power down bit in power control register.
	 * It will be set on powerdown and cleared on wakeup
	 * ---------------------------------------------------
	 */
	sysreg_bit_toggle VEYMONT_IMP_CPUPWRCTLR_EL1, \
		VEYMONT_IMP_CPUPWRCTLR_EL1_CORE_PWRDN_EN_BIT
	isb
veymont_skip_pwr_dwn:
	signal_pabandon_handled
	ret
endfunc veymont_core_pwr_dwn

.section .rodata.veymont_regs, "aS"
veymont_regs: /* The ASCII list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func veymont_cpu_reg_dump
	adr 	x6, veymont_regs
	mrs	x8, VEYMONT_IMP_CPUECTLR_EL1
	ret
endfunc veymont_cpu_reg_dump

declare_cpu_ops veymont, VEYMONT_MIDR, \
	veymont_reset_func, \
	veymont_core_pwr_dwn
