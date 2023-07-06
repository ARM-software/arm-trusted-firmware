/*
 * Copyright (c) 2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <nevis.h>
#include <cpu_macros.S>
#include <plat_macros.S>

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "Nevis must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

/* 64-bit only core */
#if CTX_INCLUDE_AARCH32_REGS == 1
#error "Nevis supports only AArch64. Compile with CTX_INCLUDE_AARCH32_REGS=0"
#endif

cpu_reset_func_start nevis
	/* ----------------------------------------------------
	 * Disable speculative loads
	 * ----------------------------------------------------
	 */
	msr	SSBS, xzr
cpu_reset_func_end nevis

func nevis_core_pwr_dwn
	/* ---------------------------------------------------
	 * Enable CPU power down bit in power control register
	 * ---------------------------------------------------
	 */
	sysreg_bit_set NEVIS_IMP_CPUPWRCTLR_EL1, \
		NEVIS_IMP_CPUPWRCTLR_EL1_CORE_PWRDN_EN_BIT
	isb
	ret
endfunc nevis_core_pwr_dwn

errata_report_shim nevis

.section .rodata.nevis_regs, "aS"
nevis_regs: /* The ASCII list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func nevis_cpu_reg_dump
	adr 	x6, nevis_regs
	mrs	x8, NEVIS_CPUECTLR_EL1
	ret
endfunc nevis_cpu_reg_dump

declare_cpu_ops nevis, NEVIS_MIDR, \
	nevis_reset_func, \
	nevis_core_pwr_dwn
