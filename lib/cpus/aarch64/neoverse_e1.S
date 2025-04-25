/*
 * Copyright (c) 2018-2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <common/debug.h>
#include <dsu_macros.S>
#include <neoverse_e1.h>
#include <cpu_macros.S>
#include <plat_macros.S>

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "Neoverse E1 must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

/* 64-bit only core */
#if CTX_INCLUDE_AARCH32_REGS == 1
#error "Neoverse-E1 supports only AArch64. Compile with CTX_INCLUDE_AARCH32_REGS=0"
#endif

cpu_reset_prologue neoverse_e1

workaround_reset_start neoverse_e1, ERRATUM(936184), ERRATA_DSU_936184
	errata_dsu_936184_wa_impl
workaround_reset_end neoverse_e1, ERRATUM(936184)

check_erratum_custom_start neoverse_e1, ERRATUM(936184)
	branch_if_scu_not_present 2f /* label 1 is used in the macro */
	check_errata_dsu_936184_impl
	2:
	ret
check_erratum_custom_end neoverse_e1, ERRATUM(936184)

cpu_reset_func_start neoverse_e1
cpu_reset_func_end neoverse_e1

func neoverse_e1_cpu_pwr_dwn
	mrs	x0, NEOVERSE_E1_CPUPWRCTLR_EL1
	orr	x0, x0, #NEOVERSE_E1_CPUPWRCTLR_EL1_CORE_PWRDN_BIT
	msr	NEOVERSE_E1_CPUPWRCTLR_EL1, x0
	isb
	ret
endfunc neoverse_e1_cpu_pwr_dwn

.section .rodata.neoverse_e1_regs, "aS"
neoverse_e1_regs:  /* The ascii list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func neoverse_e1_cpu_reg_dump
	adr	x6, neoverse_e1_regs
	mrs	x8, NEOVERSE_E1_ECTLR_EL1
	ret
endfunc neoverse_e1_cpu_reg_dump

declare_cpu_ops neoverse_e1, NEOVERSE_E1_MIDR, \
	neoverse_e1_reset_func, \
	neoverse_e1_cpu_pwr_dwn
