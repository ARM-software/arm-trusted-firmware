/*
 * Copyright (c) 2021-2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <cortex_a520.h>
#include <cpu_macros.S>
#include <dsu_macros.S>
#include <plat_macros.S>

.global check_erratum_cortex_a520_2938996

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "Cortex A520 must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

/* 64-bit only core */
#if CTX_INCLUDE_AARCH32_REGS == 1
#error "Cortex A520 supports only AArch64. Compile with CTX_INCLUDE_AARCH32_REGS=0"
#endif

cpu_reset_prologue cortex_a520

workaround_reset_start cortex_a520, ERRATUM(2630792), ERRATA_A520_2630792
	sysreg_bit_set CORTEX_A520_CPUACTLR_EL1, BIT(38)
workaround_reset_end cortex_a520, ERRATUM(2630792)

check_erratum_ls cortex_a520, ERRATUM(2630792), CPU_REV(0, 1)

workaround_reset_start cortex_a520, ERRATUM(2858100), ERRATA_A520_2858100
	sysreg_bit_set CORTEX_A520_CPUACTLR_EL1, BIT(29)
workaround_reset_end cortex_a520, ERRATUM(2858100)

check_erratum_ls cortex_a520, ERRATUM(2858100), CPU_REV(0, 1)

workaround_reset_start cortex_a520, ERRATUM(2900952), ERRATA_DSU_2900952
	errata_dsu_2900952_wa_apply
workaround_reset_end cortex_a520, ERRATUM(2900952)

check_erratum_custom_start cortex_a520, ERRATUM(2900952)
	check_errata_dsu_2900952_applies
	ret
check_erratum_custom_end cortex_a520, ERRATUM(2900952)

add_erratum_entry cortex_a520, ERRATUM(2938996), ERRATA_A520_2938996

check_erratum_ls cortex_a520, ERRATUM(2938996), CPU_REV(0, 1)

	/* ----------------------------------------------------
	 * HW will do the cache maintenance while powering down
	 * ----------------------------------------------------
	 */
func cortex_a520_core_pwr_dwn
	/* ---------------------------------------------------
	 * Enable CPU power down bit in power control register
	 * ---------------------------------------------------
	 */
	sysreg_bit_set CORTEX_A520_CPUPWRCTLR_EL1, CORTEX_A520_CPUPWRCTLR_EL1_CORE_PWRDN_BIT
	isb
	ret
endfunc cortex_a520_core_pwr_dwn

cpu_reset_func_start cortex_a520
	/* Disable speculative loads */
	msr	SSBS, xzr
	enable_mpmm
cpu_reset_func_end cortex_a520

	/* ---------------------------------------------
	 * This function provides Cortex A520 specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.cortex_a520_regs, "aS"
cortex_a520_regs:  /* The ascii list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func cortex_a520_cpu_reg_dump
	adr	x6, cortex_a520_regs
	mrs	x8, CORTEX_A520_CPUECTLR_EL1
	ret
endfunc cortex_a520_cpu_reg_dump

declare_cpu_ops cortex_a520, CORTEX_A520_MIDR, \
	cortex_a520_reset_func, \
	cortex_a520_core_pwr_dwn
