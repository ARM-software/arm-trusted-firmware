/*
 * Copyright (c) 2022-2023, Google LLC. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <asm_macros.S>
#include <cortex_x1.h>
#include <cpu_macros.S>
#include "wa_cve_2022_23960_bhb_vector.S"

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "Cortex-X1 must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

/* 64-bit only core */
#if CTX_INCLUDE_AARCH32_REGS == 1
#error "Cortex-X1 supports only AArch64. Compile with CTX_INCLUDE_AARCH32_REGS=0"
#endif

#if WORKAROUND_CVE_2022_23960
	wa_cve_2022_23960_bhb_vector_table CORTEX_X1_BHB_LOOP_COUNT, cortex_x1
#endif /* WORKAROUND_CVE_2022_23960 */

workaround_reset_start cortex_x1, ERRATUM(1688305), ERRATA_X1_1688305
	mrs	x0, CORTEX_X1_ACTLR2_EL1
	orr	x0, x0, #BIT(1)
	msr	CORTEX_X1_ACTLR2_EL1, x0
workaround_reset_end cortex_x1, ERRATUM(1688305)

check_erratum_ls cortex_x1, ERRATUM(1688305), CPU_REV(1, 0)

workaround_reset_start cortex_x1, ERRATUM(1821534), ERRATA_X1_1821534
	mrs	x1, CORTEX_X1_ACTLR2_EL1
	orr	x1, x1, #BIT(2)
	msr	CORTEX_X1_ACTLR2_EL1, x1
workaround_reset_end cortex_x1, ERRATUM(1821534)

check_erratum_ls cortex_x1, ERRATUM(1821534), CPU_REV(1, 0)

workaround_reset_start cortex_x1, ERRATUM(1827429), ERRATA_X1_1827429
	mrs	x0, CORTEX_X1_CPUECTLR_EL1
	orr	x0, x0, #BIT(53)
	msr	CORTEX_X1_CPUECTLR_EL1, x0
workaround_reset_end cortex_x1, ERRATUM(1827429)

check_erratum_ls cortex_x1, ERRATUM(1827429), CPU_REV(1, 0)

check_erratum_chosen cortex_x1, CVE(2022, 23960), WORKAROUND_CVE_2022_23960

workaround_reset_start cortex_x1, CVE(2022, 23960), WORKAROUND_CVE_2022_23960
#if IMAGE_BL31
	/*
	 * The Cortex-X1 generic vectors are overridden to apply errata
	 * mitigation on exception entry from lower ELs.
	 */
	adr	x0, wa_cve_vbar_cortex_x1
	msr	vbar_el3, x0
#endif /* IMAGE_BL31 */
workaround_reset_end cortex_x1, CVE(2022, 23960)

cpu_reset_func_start cortex_x1
cpu_reset_func_end cortex_x1

	/* ---------------------------------------------
	 * HW will do the cache maintenance while powering down
	 * ---------------------------------------------
	 */
func cortex_x1_core_pwr_dwn
	/* ---------------------------------------------
	 * Enable CPU power down bit in power control register
	 * ---------------------------------------------
	 */
	mrs	x0, CORTEX_X1_CPUPWRCTLR_EL1
	orr	x0, x0, #CORTEX_X1_CORE_PWRDN_EN_MASK
	msr	CORTEX_X1_CPUPWRCTLR_EL1, x0
	isb
	ret
endfunc cortex_x1_core_pwr_dwn

errata_report_shim cortex_x1

       /* ---------------------------------------------
	* This function provides Cortex X1 specific
	* register information for crash reporting.
	* It needs to return with x6 pointing to
	* a list of register names in ascii and
	* x8 - x15 having values of registers to be
	* reported.
	* ---------------------------------------------
	*/
.section .rodata.cortex_x1_regs, "aS"
cortex_x1_regs:  /* The ascii list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func cortex_x1_cpu_reg_dump
	adr	x6, cortex_x1_regs
	mrs	x8, CORTEX_X1_CPUECTLR_EL1
	ret
endfunc cortex_x1_cpu_reg_dump

declare_cpu_ops cortex_x1, CORTEX_X1_MIDR, \
	cortex_x1_reset_func, \
	cortex_x1_core_pwr_dwn
