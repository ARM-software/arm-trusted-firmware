/*
 * Copyright (c) 2023-2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <neoverse_n3.h>
#include <cpu_macros.S>
#include <plat_macros.S>

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "Neoverse-N3 must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

/* 64-bit only core */
#if CTX_INCLUDE_AARCH32_REGS == 1
#error "Neoverse-N3 supports only AArch64. Compile with CTX_INCLUDE_AARCH32_REGS=0"
#endif

cpu_reset_prologue neoverse_n3

.global check_erratum_neoverse_n3_3699563

workaround_runtime_start neoverse_n3, ERRATUM(3456111), ERRATA_N3_3456111
	speculation_barrier
workaround_runtime_end neoverse_n3, ERRATUM(3456111)

check_erratum_ls neoverse_n3, ERRATUM(3456111), CPU_REV(0, 1)

add_erratum_entry neoverse_n3, ERRATUM(3699563), ERRATA_N3_3699563

check_erratum_ls neoverse_n3, ERRATUM(3699563), CPU_REV(0, 0)

cpu_reset_func_start neoverse_n3
	/* Disable speculative loads */
	msr	SSBS, xzr
	apply_erratum neoverse_n3, ERRATUM(3456111), ERRATA_N3_3456111

#if !NEOVERSE_Nx_EXTERNAL_LLC
	/* -------------------------------------------------------------
	 * Neoverse n3 has that last level cache is external by default.
	 * Clear the bit when NEOVERSE_Nx_EXTERNAL_LLC is not enabled.
	 * -------------------------------------------------------------
	 */
	sysreg_bit_clear NEOVERSE_N3_CPUECTLR2_EL1, NEOVERSE_N3_CPUECTLR2_EL1_SW_EXT_LLC_BIT
#endif
cpu_reset_func_end neoverse_n3

	/* ----------------------------------------------------
	 * HW will do the cache maintenance while powering down
	 * ----------------------------------------------------
	 */
func neoverse_n3_core_pwr_dwn
	/* ---------------------------------------------------
	 * Enable CPU power down bit in power control register
	 * ---------------------------------------------------
	 */
	sysreg_bit_set NEOVERSE_N3_CPUPWRCTLR_EL1, NEOVERSE_N3_CPUPWRCTLR_EL1_CORE_PWRDN_BIT
	isb
	ret
endfunc neoverse_n3_core_pwr_dwn

	/* ---------------------------------------------
	 * This function provides Neoverse-N3 specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.neoverse_n3_regs, "aS"
neoverse_n3_regs:  /* The ascii list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func neoverse_n3_cpu_reg_dump
	adr	x6, neoverse_n3_regs
	mrs	x8, NEOVERSE_N3_CPUECTLR_EL1
	ret
endfunc neoverse_n3_cpu_reg_dump

declare_cpu_ops neoverse_n3, NEOVERSE_N3_MIDR, \
	neoverse_n3_reset_func, \
	neoverse_n3_core_pwr_dwn
