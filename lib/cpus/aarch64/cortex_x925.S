/*
 * Copyright (c) 2023-2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <cortex_x925.h>
#include <cpu_macros.S>
#include <dsu_macros.S>
#include <plat_macros.S>

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "Cortex-X925 must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

/* 64-bit only core */
#if CTX_INCLUDE_AARCH32_REGS == 1
#error "Cortex-X925 supports only AArch64. Compile with CTX_INCLUDE_AARCH32_REGS=0"
#endif

cpu_reset_prologue cortex_x925

workaround_reset_start cortex_x925, ERRATUM(2900952), ERRATA_DSU_2900952
	errata_dsu_2900952_wa_apply
workaround_reset_end cortex_x925, ERRATUM(2900952)

check_erratum_custom_start cortex_x925, ERRATUM(2900952)
	check_errata_dsu_2900952_applies
	ret
check_erratum_custom_end cortex_x925, ERRATUM(2900952)

add_erratum_entry cortex_x925, ERRATUM(3701747), ERRATA_X925_3701747

.global check_erratum_cortex_x925_3701747
check_erratum_ls cortex_x925, ERRATUM(3701747), CPU_REV(0, 1)

workaround_reset_start cortex_x925, ERRATUM(2963999), ERRATA_X925_2963999
	/* Add ISB before MRS reads of MPIDR_EL1/MIDR_EL1 */
	ldr x0, =0x0
	msr S3_6_c15_c8_0, x0 	/* msr CPUPSELR_EL3, X0 */
	ldr x0, =0xd5380000
	msr S3_6_c15_c8_2, x0 	/* msr CPUPOR_EL3, X0 */
	ldr x0, =0xFFFFFF40
	msr S3_6_c15_c8_3,x0 	/* msr CPUPMR_EL3, X0 */
	ldr x0, =0x000080010033f
	msr S3_6_c15_c8_1, x0	/* msr CPUPCR_EL3, X0 */
	isb
workaround_reset_end cortex_x925, ERRATUM(2963999)

check_erratum_ls cortex_x925, ERRATUM(2963999), CPU_REV(0, 0)

/* Disable hardware page aggregation. Enables mitigation for `CVE-2024-5660` */
workaround_reset_start cortex_x925, CVE(2024, 5660), WORKAROUND_CVE_2024_5660
	sysreg_bit_set CORTEX_X925_CPUECTLR_EL1, BIT(46)
workaround_reset_end cortex_x925, CVE(2024, 5660)

check_erratum_ls cortex_x925, CVE(2024, 5660), CPU_REV(0, 1)

workaround_reset_start cortex_x925, CVE(2024, 7881), WORKAROUND_CVE_2024_7881
	/* ---------------------------------
         * Sets BIT41 of CPUACTLR6_EL1 which
         * disables L1 Data cache prefetcher
         * ---------------------------------
         */
	sysreg_bit_set CORTEX_X925_CPUACTLR6_EL1, BIT(41)
workaround_reset_end cortex_x925, CVE(2024, 7881)

check_erratum_chosen cortex_x925, CVE(2024, 7881), WORKAROUND_CVE_2024_7881

cpu_reset_func_start cortex_x925
	/* Disable speculative loads */
	msr	SSBS, xzr
cpu_reset_func_end cortex_x925

	/* ----------------------------------------------------
	 * HW will do the cache maintenance while powering down
	 * ----------------------------------------------------
	 */
func cortex_x925_core_pwr_dwn
	/* ---------------------------------------------------
	 * Enable CPU power down bit in power control register
	 * ---------------------------------------------------
	 */
	sysreg_bit_set CORTEX_X925_CPUPWRCTLR_EL1, CORTEX_X925_CPUPWRCTLR_EL1_CORE_PWRDN_BIT
	isb
	ret
endfunc cortex_x925_core_pwr_dwn

	/* ---------------------------------------------
	 * This function provides Cortex-X925 specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.cortex_x925_regs, "aS"
cortex_x925_regs:  /* The ascii list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func cortex_x925_cpu_reg_dump
	adr	x6, cortex_x925_regs
	mrs	x8, CORTEX_X925_CPUECTLR_EL1
	ret
endfunc cortex_x925_cpu_reg_dump

declare_cpu_ops cortex_x925, CORTEX_X925_MIDR, \
	cortex_x925_reset_func, \
	cortex_x925_core_pwr_dwn
