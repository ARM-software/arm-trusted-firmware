/*
 * Copyright (c) 2023-2026, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * TRM: https://support.arm.com/documentation/107997/latest
 * SDEN: https://support.arm.com/documentation/SDEN-3050973/latest
 * MinRevision: r0p0
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

workaround_reset_start neoverse_n3, ERRATUM(3711915), ERRATA_N3_3711915
	mov x0, #5
	msr S3_6_C15_C8_0, x0
	isb
	ldr x0, =0xd503329f
	msr S3_6_C15_C8_2, x0
	ldr x0, =0xfffff3ff
	msr S3_6_C15_C8_3, x0
	mov x1, #0
	orr x1, x1, #1<<0
	orr x1, x1, #3<<4
	orr x1, x1, #0xf<<6
	orr x1, x1, #1<<22
	orr x1, x1, #1<<32
	msr S3_6_C15_C8_1, x1
workaround_reset_end neoverse_n3, ERRATUM(3711915)

check_erratum_ls neoverse_n3, ERRATUM(3711915), CPU_REV(0, 0)

workaround_reset_start neoverse_n3, ERRATUM(3866095), ERRATA_N3_3866095
	sysreg_bit_set NEOVERSE_N3_CPUECTLR2_EL1, BIT(0)
workaround_reset_end neoverse_n3, ERRATUM(3866095)

check_erratum_ls neoverse_n3, ERRATUM(3866095), CPU_REV(0, 0)

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


declare_cpu_ops neoverse_n3, NEOVERSE_N3_MIDR, \
	neoverse_n3_reset_func, \
	neoverse_n3_core_pwr_dwn
