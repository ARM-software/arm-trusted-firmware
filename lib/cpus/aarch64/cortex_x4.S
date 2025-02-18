/*
 * Copyright (c) 2022-2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <cortex_x4.h>
#include <cpu_macros.S>
#include <plat_macros.S>
#include "wa_cve_2022_23960_bhb_vector.S"

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "Cortex X4 must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

/* 64-bit only core */
#if CTX_INCLUDE_AARCH32_REGS == 1
#error "Cortex X4 supports only AArch64. Compile with CTX_INCLUDE_AARCH32_REGS=0"
#endif

.global check_erratum_cortex_x4_2726228
.global check_erratum_cortex_x4_3701758

#if WORKAROUND_CVE_2022_23960
        wa_cve_2022_23960_bhb_vector_table CORTEX_X4_BHB_LOOP_COUNT, cortex_x4
#endif /* WORKAROUND_CVE_2022_23960 */

workaround_runtime_start cortex_x4, ERRATUM(2726228), ERRATA_X4_2726228, CORTEX_X4_MIDR
workaround_runtime_end cortex_x4, ERRATUM(2726228)

check_erratum_custom_start cortex_x4, ERRATUM(2726228)

	/* This erratum needs to be enabled for r0p0 and r0p1.
	 * Check if revision is less than or equal to r0p1.
	 */

#if ERRATA_X4_2726228
	mov	x1, #1
	b	cpu_rev_var_ls
#else
	mov	x0, #ERRATA_MISSING
#endif
	ret
check_erratum_custom_end cortex_x4, ERRATUM(2726228)

/* Disable hardware page aggregation. Enables mitigation for `CVE-2024-5660` */
workaround_reset_start cortex_x4, CVE(2024, 5660), WORKAROUND_CVE_2024_5660
	sysreg_bit_set CORTEX_X4_CPUECTLR_EL1, BIT(46)
workaround_reset_end cortex_x4, CVE(2024, 5660)

check_erratum_ls cortex_x4, CVE(2024, 5660), CPU_REV(0, 2)

workaround_runtime_start cortex_x4, ERRATUM(2740089), ERRATA_X4_2740089
	/* dsb before isb of power down sequence */
	dsb	sy
workaround_runtime_end cortex_x4, ERRATUM(2740089)

check_erratum_ls cortex_x4, ERRATUM(2740089), CPU_REV(0, 1)

workaround_reset_start cortex_x4, ERRATUM(2763018), ERRATA_X4_2763018
	sysreg_bit_set	CORTEX_X4_CPUACTLR3_EL1, BIT(47)
workaround_reset_end cortex_x4, ERRATUM(2763018)

check_erratum_ls cortex_x4, ERRATUM(2763018), CPU_REV(0, 1)

workaround_reset_start cortex_x4, ERRATUM(2816013), ERRATA_X4_2816013
	mrs x1, id_aa64pfr1_el1
	ubfx x2, x1, ID_AA64PFR1_EL1_MTE_SHIFT, #4
	cbz x2, #1f
	sysreg_bit_set CORTEX_X4_CPUACTLR5_EL1, BIT(14)
1:
workaround_reset_end cortex_x4, ERRATUM(2816013)

check_erratum_ls cortex_x4, ERRATUM(2816013), CPU_REV(0, 1)

workaround_reset_start cortex_x4, ERRATUM(2897503), ERRATA_X4_2897503
	sysreg_bit_set	CORTEX_X4_CPUACTLR4_EL1, BIT(8)
workaround_reset_end cortex_x4, ERRATUM(2897503)

check_erratum_ls cortex_x4, ERRATUM(2897503), CPU_REV(0, 1)

workaround_reset_start cortex_x4, ERRATUM(2923985), ERRATA_X4_2923985
	sysreg_bit_set CORTEX_X4_CPUACTLR4_EL1, (BIT(11) | BIT(10))
workaround_reset_end cortex_x4, ERRATUM(2923985)

check_erratum_ls cortex_x4, ERRATUM(2923985), CPU_REV(0, 1)

workaround_reset_start cortex_x4, ERRATUM(3076789), ERRATA_X4_3076789
	sysreg_bit_set CORTEX_X4_CPUACTLR3_EL1, BIT(14)
	sysreg_bit_set CORTEX_X4_CPUACTLR3_EL1, BIT(13)
	sysreg_bit_set CORTEX_X4_CPUACTLR_EL1, BIT(52)
workaround_reset_end cortex_x4, ERRATUM(3076789)

check_erratum_ls cortex_x4, ERRATUM(3076789), CPU_REV(0, 1)

workaround_reset_start cortex_x4, CVE(2022, 23960), WORKAROUND_CVE_2022_23960
#if IMAGE_BL31
	/*
	 * The Cortex X4 generic vectors are overridden to apply errata
	 * mitigation on exception entry from lower ELs.
	 */
	override_vector_table wa_cve_vbar_cortex_x4
#endif /* IMAGE_BL31 */
workaround_reset_end cortex_x4, CVE(2022, 23960)

check_erratum_chosen cortex_x4, CVE(2022, 23960), WORKAROUND_CVE_2022_23960

workaround_reset_start cortex_x4, CVE(2024, 7881), WORKAROUND_CVE_2024_7881
	/* ---------------------------------
	 * Sets BIT41 of CPUACTLR6_EL1 which
	 * disables L1 Data cache prefetcher
	 * ---------------------------------
	 */
	sysreg_bit_set CORTEX_X4_CPUACTLR6_EL1, BIT(41)
workaround_reset_end cortex_x4, CVE(2024, 7881)

check_erratum_chosen cortex_x4, CVE(2024, 7881), WORKAROUND_CVE_2024_7881

add_erratum_entry cortex_x4, ERRATUM(3701758), ERRATA_X4_3701758, NO_APPLY_AT_RESET

check_erratum_ls cortex_x4, ERRATUM(3701758), CPU_REV(0, 3)

cpu_reset_func_start cortex_x4
	/* Disable speculative loads */
	msr	SSBS, xzr
cpu_reset_func_end cortex_x4

	/* ----------------------------------------------------
	 * HW will do the cache maintenance while powering down
	 * ----------------------------------------------------
	 */
func cortex_x4_core_pwr_dwn
	/* ---------------------------------------------------
	 * Enable CPU power down bit in power control register
	 * ---------------------------------------------------
	 */
	sysreg_bit_set CORTEX_X4_CPUPWRCTLR_EL1, CORTEX_X4_CPUPWRCTLR_EL1_CORE_PWRDN_BIT

	apply_erratum cortex_x4, ERRATUM(2740089), ERRATA_X4_2740089, NO_GET_CPU_REV

	isb
	ret
endfunc cortex_x4_core_pwr_dwn

errata_report_shim cortex_x4

	/* ---------------------------------------------
	 * This function provides Cortex X4-specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.cortex_x4_regs, "aS"
cortex_x4_regs:  /* The ascii list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func cortex_x4_cpu_reg_dump
	adr	x6, cortex_x4_regs
	mrs	x8, CORTEX_X4_CPUECTLR_EL1
	ret
endfunc cortex_x4_cpu_reg_dump

declare_cpu_ops_wa_4 cortex_x4, CORTEX_X4_MIDR, \
	cortex_x4_reset_func, \
	CPU_NO_EXTRA1_FUNC, \
	CPU_NO_EXTRA2_FUNC, \
	CPU_NO_EXTRA3_FUNC, \
	check_erratum_cortex_x4_7881, \
	cortex_x4_core_pwr_dwn
