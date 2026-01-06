/*
 * Copyright (c) 2021-2026, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <cortex_x3.h>
#include <cpu_macros.S>
#include <plat_macros.S>
#include "wa_cve_2022_23960_bhb_vector.S"

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "Cortex-X3 must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

/* 64-bit only core */
#if CTX_INCLUDE_AARCH32_REGS == 1
#error "Cortex-X3 supports only AArch64. Compile with CTX_INCLUDE_AARCH32_REGS=0"
#endif

.global check_erratum_cortex_x3_3701769

#if WORKAROUND_CVE_2022_23960
	wa_cve_2022_23960_bhb_vector_table CORTEX_X3_BHB_LOOP_COUNT, cortex_x3
#endif /* WORKAROUND_CVE_2022_23960 */

cpu_reset_prologue cortex_x3

workaround_reset_start cortex_x3, ERRATUM(2266875), ERRATA_X3_2266875
        sysreg_bit_set CORTEX_X3_CPUACTLR_EL1, BIT(22)
workaround_reset_end cortex_x3, ERRATUM(2266875)

check_erratum_ls cortex_x3, ERRATUM(2266875), CPU_REV(1, 0)

workaround_reset_start cortex_x3, ERRATUM(2302506), ERRATA_X3_2302506
	sysreg_bit_set	CORTEX_X3_CPUACTLR2_EL1, BIT(0)
workaround_reset_end cortex_x3, ERRATUM(2302506)

check_erratum_ls cortex_x3, ERRATUM(2302506), CPU_REV(1, 1)

.global erratum_cortex_x3_2313909_wa
workaround_runtime_start cortex_x3, ERRATUM(2313909), ERRATA_X3_2313909
	/* Set/unset bit 36 in ACTLR2_EL1. The first call will set it, applying
	 * the workaround. Second call clears it to undo it. */
	sysreg_bit_toggle CORTEX_X3_CPUACTLR2_EL1, CORTEX_X3_CPUACTLR2_EL1_BIT_36
workaround_runtime_end cortex_x3, ERRATUM(2313909), NO_ISB

check_erratum_ls cortex_x3, ERRATUM(2313909), CPU_REV(1, 0)

workaround_reset_start cortex_x3, ERRATUM(2372204), ERRATA_X3_2372204
	/* Set bit 40 in CPUACTLR2_EL1 */
	sysreg_bit_set CORTEX_X3_CPUACTLR2_EL1, BIT(40)
workaround_reset_end cortex_x3, ERRATUM(2372204)

check_erratum_ls cortex_x3, ERRATUM(2372204), CPU_REV(1, 0)

workaround_reset_start cortex_x3, ERRATUM(2615812), ERRATA_X3_2615812
	/* Disable retention control for WFI and WFE. */
	mrs	x0, CORTEX_X3_CPUPWRCTLR_EL1
	bfi	x0, xzr, #CORTEX_X3_CPUPWRCTLR_EL1_WFI_RET_CTRL_BITS_SHIFT, #3
	bfi	x0, xzr, #CORTEX_X3_CPUPWRCTLR_EL1_WFE_RET_CTRL_BITS_SHIFT, #3
	msr	CORTEX_X3_CPUPWRCTLR_EL1, x0
workaround_reset_end cortex_x3, ERRATUM(2615812)

check_erratum_ls cortex_x3, ERRATUM(2615812), CPU_REV(1, 1)

workaround_reset_start cortex_x3, ERRATUM(2641945), ERRATA_X3_2641945
	sysreg_bit_set	CORTEX_X3_CPUACTLR6_EL1, BIT(41)
workaround_reset_end cortex_x3, ERRATUM(2641945)

check_erratum_ls cortex_x3, ERRATUM(2641945), CPU_REV(1, 0)

workaround_reset_start cortex_x3, ERRATUM(2742421), ERRATA_X3_2742421
	/* Set CPUACTLR5_EL1[56:55] to 2'b01 */
	sysreg_bit_set CORTEX_X3_CPUACTLR5_EL1, CORTEX_X3_CPUACTLR5_EL1_BIT_55
	sysreg_bit_clear CORTEX_X3_CPUACTLR5_EL1, CORTEX_X3_CPUACTLR5_EL1_BIT_56
workaround_reset_end cortex_x3, ERRATUM(2742421)

check_erratum_ls cortex_x3, ERRATUM(2742421), CPU_REV(1, 1)

workaround_runtime_start cortex_x3, ERRATUM(2743088), ERRATA_X3_2743088
	/* dsb before isb of power down sequence */
	dsb sy
workaround_runtime_end cortex_x3, ERRATUM(2743088), NO_ISB

check_erratum_ls cortex_x3, ERRATUM(2743088), CPU_REV(1, 1)

workaround_reset_start cortex_x3, ERRATUM(2779509), ERRATA_X3_2779509
	/* Set CPUACTLR3_EL1 bit 47 */
	sysreg_bit_set CORTEX_X3_CPUACTLR3_EL1, CORTEX_X3_CPUACTLR3_EL1_BIT_47
workaround_reset_end cortex_x3, ERRATUM(2779509)

check_erratum_ls cortex_x3, ERRATUM(2779509), CPU_REV(1, 1)

workaround_reset_start cortex_x3, ERRATUM(3213672), ERRATA_X3_3213672
	sysreg_bit_set	CORTEX_X3_CPUACTLR_EL1, BIT(36)
workaround_reset_end cortex_x3, ERRATUM(3213672)

check_erratum_ls cortex_x3, ERRATUM(3213672), CPU_REV(1, 2)

workaround_reset_start cortex_x3, ERRATUM(3692984), ERRATA_X3_3692984
	sysreg_bit_set	CORTEX_X3_CPUACTLR6_EL1, BIT(41)
workaround_reset_end cortex_x3, ERRATUM(3692984)

check_erratum_ls cortex_x3, ERRATUM(3692984), CPU_REV(1, 2)

add_erratum_entry cortex_x3, ERRATUM(3701769), ERRATA_X3_3701769

check_erratum_ls cortex_x3, ERRATUM(3701769), CPU_REV(1, 2)

workaround_reset_start cortex_x3, ERRATUM(3827463), ERRATA_X3_3827463
	sysreg_bit_set	CORTEX_X3_CPUACTLR_EL1, BIT(1)
workaround_reset_end cortex_x3, ERRATUM(3827463)

check_erratum_ls cortex_x3, ERRATUM(3827463), CPU_REV(1, 1)

workaround_reset_start cortex_x3, ERRATUM(3888125), ERRATA_X3_3888125
	sysreg_bit_set	CORTEX_X3_CPUACTLR2_EL1, BIT(22)
workaround_reset_end cortex_x3, ERRATUM(3888125)

check_erratum_ls cortex_x3, ERRATUM(3888125), CPU_REV(1, 2)

workaround_reset_start cortex_x3, ERRATUM(4302966), ERRATA_X3_4302966
	sysreg_bit_set	CORTEX_X3_CPUACTLR5_EL1, BIT(50)
workaround_reset_end cortex_x3, ERRATUM(4302966)

check_erratum_ls cortex_x3, ERRATUM(4302966), CPU_REV(1, 2)

workaround_reset_start cortex_x3, CVE(2022, 23960), WORKAROUND_CVE_2022_23960
#if IMAGE_BL31
	override_vector_table wa_cve_vbar_cortex_x3
#endif /* IMAGE_BL31 */
workaround_reset_end cortex_x3, CVE(2022, 23960)

check_erratum_ls cortex_x3, CVE(2022, 23960), CPU_REV(1, 0)

/* Disable hardware page aggregation. Enables mitigation for `CVE-2024-5660` */
workaround_reset_start cortex_x3, CVE(2024, 5660), WORKAROUND_CVE_2024_5660
	sysreg_bit_set CORTEX_X3_CPUECTLR_EL1, BIT(46)
workaround_reset_end cortex_x3, CVE(2024, 5660)

check_erratum_ls cortex_x3, CVE(2024, 5660), CPU_REV(1, 2)

	/* --------------------------------------------------------------
	 * CVE-2024-7881 is mitigated for Cortex-X3 using erratum 3692984
	 * workaround by disabling the affected prefetcher setting
	 * CPUACTLR6_EL1[41].
	 * --------------------------------------------------------------
	 */
workaround_reset_start cortex_x3, CVE(2024, 7881), WORKAROUND_CVE_2024_7881
	sysreg_bit_set CORTEX_X3_CPUACTLR6_EL1, BIT(41)
workaround_reset_end cortex_x3, CVE(2024, 7881)

check_erratum_ls cortex_x3, CVE(2024, 7881), CPU_REV(1, 2)

cpu_reset_func_start cortex_x3
	/* Disable speculative loads */
	msr	SSBS, xzr
	enable_mpmm
cpu_reset_func_end cortex_x3

	/* ----------------------------------------------------
	 * HW will do the cache maintenance while powering down
	 * ----------------------------------------------------
	 */
func cortex_x3_core_pwr_dwn
	apply_erratum cortex_x3, ERRATUM(2313909), ERRATA_X3_2313909
	/* ---------------------------------------------------
	 * Enable CPU power down bit in power control register
	 * ---------------------------------------------------
	 */
	sysreg_bit_set CORTEX_X3_CPUPWRCTLR_EL1, CORTEX_X3_CPUPWRCTLR_EL1_CORE_PWRDN_BIT
	apply_erratum cortex_x3, ERRATUM(2743088), ERRATA_X3_2743088, NO_GET_CPU_REV
	isb
	ret
endfunc cortex_x3_core_pwr_dwn

	/* ---------------------------------------------
	 * This function provides Cortex-X3-
	 * specific register information for crash
	 * reporting. It needs to return with x6
	 * pointing to a list of register names in ascii
	 * and x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.cortex_x3_regs, "aS"
cortex_x3_regs:  /* The ascii list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func cortex_x3_cpu_reg_dump
	adr	x6, cortex_x3_regs
	mrs	x8, CORTEX_X3_CPUECTLR_EL1
	ret
endfunc cortex_x3_cpu_reg_dump

declare_cpu_ops cortex_x3, CORTEX_X3_MIDR, \
	cortex_x3_reset_func, \
	cortex_x3_core_pwr_dwn
