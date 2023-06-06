/*
 * Copyright (c) 2018-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <common/debug.h>
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

/*
 * ERRATA_DSU_936184:
 * The errata is defined in dsu_helpers.S and applies to neoverse_e1.
 * Henceforth creating symbolic names to the already existing errata
 * workaround functions to get them registered under the Errata Framework.
 */
.equ check_erratum_neoverse_e1_936184, check_errata_dsu_936184
.equ erratum_neoverse_e1_936184_wa, errata_dsu_936184_wa
add_erratum_entry neoverse_e1, ERRATUM(936184), ERRATA_DSU_936184, APPLY_AT_RESET

cpu_reset_func_start neoverse_e1
cpu_reset_func_end neoverse_e1

func neoverse_e1_cpu_pwr_dwn
	mrs	x0, NEOVERSE_E1_CPUPWRCTLR_EL1
	orr	x0, x0, #NEOVERSE_E1_CPUPWRCTLR_EL1_CORE_PWRDN_BIT
	msr	NEOVERSE_E1_CPUPWRCTLR_EL1, x0
	isb
	ret
endfunc neoverse_e1_cpu_pwr_dwn

errata_report_shim neoverse_e1

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
