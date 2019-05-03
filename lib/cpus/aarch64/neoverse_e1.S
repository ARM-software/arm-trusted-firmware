/*
 * Copyright (c) 2018-2019, ARM Limited and Contributors. All rights reserved.
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

func neoverse_e1_cpu_pwr_dwn
	mrs	x0, NEOVERSE_E1_CPUPWRCTLR_EL1
	orr	x0, x0, #NEOVERSE_E1_CPUPWRCTLR_EL1_CORE_PWRDN_BIT
	msr	NEOVERSE_E1_CPUPWRCTLR_EL1, x0
	isb
	ret
endfunc neoverse_e1_cpu_pwr_dwn

#if REPORT_ERRATA
/*
 * Errata printing function for Neoverse N1. Must follow AAPCS.
 */
func neoverse_e1_errata_report
	ret
endfunc neoverse_e1_errata_report
#endif


.section .rodata.neoverse_e1_regs, "aS"
neoverse_e1_regs:  /* The ascii list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func neoverse_e1_cpu_reg_dump
	adr	x6, neoverse_e1_regs
	mrs	x8, NEOVERSE_E1_ECTLR_EL1
	ret
endfunc neoverse_e1_cpu_reg_dump

declare_cpu_ops neoverse_e1, NEOVERSE_E1_MIDR, \
	CPU_NO_RESET_FUNC, \
	neoverse_e1_cpu_pwr_dwn
