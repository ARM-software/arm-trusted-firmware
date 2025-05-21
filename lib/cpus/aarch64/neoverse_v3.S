/*
 * Copyright (c) 2022-2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <neoverse_v3.h>
#include <cpu_macros.S>
#include <plat_macros.S>
#include "wa_cve_2022_23960_bhb_vector.S"

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "Neoverse V3 must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

/* 64-bit only core */
#if CTX_INCLUDE_AARCH32_REGS == 1
#error "Neoverse V3 supports only AArch64. Compile with CTX_INCLUDE_AARCH32_REGS=0"
#endif

cpu_reset_prologue neoverse_v3

.global check_erratum_neoverse_v3_3701767

workaround_reset_start neoverse_v3, ERRATUM(2970647), ERRATA_V3_2970647
	/* Add ISB before MRS reads of MPIDR_EL1/MIDR_EL1 */
	ldr x0, =0x1
	msr S3_6_c15_c8_0, x0 	/* msr CPUPSELR_EL3, X0 */
	ldr x0, =0xd5380000
	msr S3_6_c15_c8_2, x0 	/* msr CPUPOR_EL3, X0 */
	ldr x0, =0xFFFFFF40
	msr S3_6_c15_c8_3,x0 	/* msr CPUPMR_EL3, X0 */
	ldr x0, =0x000080010033f
	msr S3_6_c15_c8_1, x0	/* msr CPUPCR_EL3, X0 */
	isb
workaround_reset_end neoverse_v3, ERRATUM(2970647)

check_erratum_ls neoverse_v3, ERRATUM(2970647), CPU_REV(0, 0)

add_erratum_entry neoverse_v3, ERRATUM(3701767), ERRATA_V3_3701767

check_erratum_ls neoverse_v3, ERRATUM(3701767), CPU_REV(0, 2)

#if WORKAROUND_CVE_2022_23960
	wa_cve_2022_23960_bhb_vector_table NEOVERSE_V3_BHB_LOOP_COUNT, neoverse_v3
#endif /* WORKAROUND_CVE_2022_23960 */

workaround_reset_start neoverse_v3, CVE(2022,23960), WORKAROUND_CVE_2022_23960
#if IMAGE_BL31
	/*
	 * The Neoverse V3 generic vectors are overridden to apply errata
         * mitigation on exception entry from lower ELs.
	 */
	override_vector_table wa_cve_vbar_neoverse_v3

#endif /* IMAGE_BL31 */
workaround_reset_end neoverse_v3, CVE(2022,23960)

check_erratum_chosen neoverse_v3, CVE(2022, 23960), WORKAROUND_CVE_2022_23960

/* Disable hardware page aggregation. Enables mitigation for `CVE-2024-5660` */
workaround_reset_start neoverse_v3, CVE(2024, 5660), WORKAROUND_CVE_2024_5660
	sysreg_bit_set NEOVERSE_V3_CPUECTLR_EL1, BIT(46)
workaround_reset_end neoverse_v3, CVE(2024, 5660)

check_erratum_ls neoverse_v3, CVE(2024, 5660), CPU_REV(0, 1)

workaround_reset_start neoverse_v3, CVE(2024, 7881), WORKAROUND_CVE_2024_7881
       /* ---------------------------------
        * Sets BIT41 of CPUACTLR6_EL1 which
        * disables L1 Data cache prefetcher
        * ---------------------------------
        */
       sysreg_bit_set NEOVERSE_V3_CPUACTLR6_EL1, BIT(41)
workaround_reset_end neoverse_v3, CVE(2024, 7881)

check_erratum_chosen neoverse_v3, CVE(2024, 7881), WORKAROUND_CVE_2024_7881

	/* ---------------------------------------------
	 * HW will do the cache maintenance while powering down
	 * ---------------------------------------------
	 */
func neoverse_v3_core_pwr_dwn
	/* ---------------------------------------------
	 * Enable CPU power down bit in power control register
	 * ---------------------------------------------
	 */
	sysreg_bit_set NEOVERSE_V3_CPUPWRCTLR_EL1, \
		NEOVERSE_V3_CPUPWRCTLR_EL1_CORE_PWRDN_BIT

	isb
	ret
endfunc neoverse_v3_core_pwr_dwn

cpu_reset_func_start neoverse_v3
	/* Disable speculative loads */
	msr	SSBS, xzr
cpu_reset_func_end neoverse_v3

	/* ---------------------------------------------
	 * This function provides Neoverse V3 specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.neoverse_v3_regs, "aS"
neoverse_v3_regs:  /* The ascii list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func neoverse_v3_cpu_reg_dump
	adr	x6, neoverse_v3_regs
	mrs	x8, NEOVERSE_V3_CPUECTLR_EL1
	ret
endfunc neoverse_v3_cpu_reg_dump

declare_cpu_ops neoverse_v3, NEOVERSE_V3_VNAE_MIDR, \
	neoverse_v3_reset_func, \
	neoverse_v3_core_pwr_dwn

declare_cpu_ops neoverse_v3, NEOVERSE_V3_MIDR, \
	neoverse_v3_reset_func, \
	neoverse_v3_core_pwr_dwn
