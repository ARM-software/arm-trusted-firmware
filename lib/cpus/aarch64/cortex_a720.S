/*
 * Copyright (c) 2021-2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <cortex_a720.h>
#include <cpu_macros.S>
#include <plat_macros.S>
#include "wa_cve_2022_23960_bhb_vector.S"

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "Cortex A720 must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

/* 64-bit only core */
#if CTX_INCLUDE_AARCH32_REGS == 1
#error "Cortex A720 supports only AArch64. Compile with CTX_INCLUDE_AARCH32_REGS=0"
#endif

#if WORKAROUND_CVE_2022_23960
        wa_cve_2022_23960_bhb_vector_table CORTEX_A720_BHB_LOOP_COUNT, cortex_a720
#endif /* WORKAROUND_CVE_2022_23960 */

workaround_reset_start cortex_a720, CVE(2022, 23960), WORKAROUND_CVE_2022_23960
#if IMAGE_BL31
	/*
	 * The Cortex A720 generic vectors are overridden to apply errata
	 * mitigation on exception entry from lower ELs.
	 */
	override_vector_table wa_cve_vbar_cortex_a720
#endif /* IMAGE_BL31 */
workaround_reset_end cortex_a720, CVE(2022, 23960)

check_erratum_chosen cortex_a720, CVE(2022, 23960), WORKAROUND_CVE_2022_23960

cpu_reset_func_start cortex_a720
	/* Disable speculative loads */
	msr	SSBS, xzr
cpu_reset_func_end cortex_a720

	/* ----------------------------------------------------
	 * HW will do the cache maintenance while powering down
	 * ----------------------------------------------------
	 */
func cortex_a720_core_pwr_dwn
	/* ---------------------------------------------------
	 * Enable CPU power down bit in power control register
	 * ---------------------------------------------------
	 */
	sysreg_bit_set CORTEX_A720_CPUPWRCTLR_EL1, CORTEX_A720_CPUPWRCTLR_EL1_CORE_PWRDN_BIT

	isb
	ret
endfunc cortex_a720_core_pwr_dwn

errata_report_shim cortex_a720

	/* ---------------------------------------------
	 * This function provides Cortex A720-specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.cortex_a720_regs, "aS"
cortex_a720_regs:  /* The ascii list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func cortex_a720_cpu_reg_dump
	adr	x6, cortex_a720_regs
	mrs	x8, CORTEX_A720_CPUECTLR_EL1
	ret
endfunc cortex_a720_cpu_reg_dump

declare_cpu_ops cortex_a720, CORTEX_A720_MIDR, \
	cortex_a720_reset_func, \
	cortex_a720_core_pwr_dwn
