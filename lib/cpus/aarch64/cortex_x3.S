/*
 * Copyright (c) 2021-2023, Arm Limited. All rights reserved.
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

#if WORKAROUND_CVE_2022_23960
	wa_cve_2022_23960_bhb_vector_table CORTEX_X3_BHB_LOOP_COUNT, cortex_x3
#endif /* WORKAROUND_CVE_2022_23960 */

workaround_reset_start cortex_x3, ERRATUM(2070301), ERRATA_X3_2070301
	sysreg_bitfield_insert CORTEX_X3_CPUECTLR2_EL1, CORTEX_X3_CPUECTLR2_EL1_PF_MODE_CNSRV, \
	CORTEX_X3_CPUECTLR2_EL1_PF_MODE_LSB, CORTEX_X3_CPUECTLR2_EL1_PF_MODE_WIDTH
workaround_reset_end cortex_x3, ERRATUM(2070301)

check_erratum_ls cortex_x3, ERRATUM(2070301), CPU_REV(1, 2)

workaround_runtime_start cortex_x3, ERRATUM(2313909), ERRATA_X3_2313909
	sysreg_bit_set	CORTEX_X3_CPUACTLR2_EL1, CORTEX_X3_CPUACTLR2_EL1_BIT_36
workaround_runtime_end cortex_x3, ERRATUM(2313909), NO_ISB

check_erratum_ls cortex_x3, ERRATUM(2313909), CPU_REV(1, 0)

workaround_reset_start cortex_x3, ERRATUM(2615812), ERRATA_X3_2615812
	/* Disable retention control for WFI and WFE. */
	mrs	x0, CORTEX_X3_CPUPWRCTLR_EL1
	bfi	x0, xzr, #CORTEX_X3_CPUPWRCTLR_EL1_WFI_RET_CTRL_BITS_SHIFT, #3
	bfi	x0, xzr, #CORTEX_X3_CPUPWRCTLR_EL1_WFE_RET_CTRL_BITS_SHIFT, #3
	msr	CORTEX_X3_CPUPWRCTLR_EL1, x0
workaround_reset_end cortex_x3, ERRATUM(2615812)

check_erratum_ls cortex_x3, ERRATUM(2615812), CPU_REV(1, 1)

workaround_reset_start cortex_x3, ERRATUM(2742421), ERRATA_X3_2742421
	/* Set CPUACTLR5_EL1[56:55] to 2'b01 */
	sysreg_bit_set CORTEX_X3_CPUACTLR5_EL1, CORTEX_X3_CPUACTLR5_EL1_BIT_55
	sysreg_bit_clear CORTEX_X3_CPUACTLR5_EL1, CORTEX_X3_CPUACTLR5_EL1_BIT_56
workaround_reset_end cortex_x3, ERRATUM(2742421)

check_erratum_ls cortex_x3, ERRATUM(2742421), CPU_REV(1, 1)

workaround_reset_start cortex_x3, CVE(2022, 23960), WORKAROUND_CVE_2022_23960
#if IMAGE_BL31
	override_vector_table wa_cve_vbar_cortex_x3
#endif /* IMAGE_BL31 */
workaround_reset_end cortex_x3, CVE(2022, 23960)

check_erratum_chosen cortex_x3, CVE(2022, 23960), WORKAROUND_CVE_2022_23960

cpu_reset_func_start cortex_x3
	/* Disable speculative loads */
	msr	SSBS, xzr
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
	isb
	ret
endfunc cortex_x3_core_pwr_dwn

errata_report_shim cortex_x3

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
