/*
 * Copyright (c) 2021-2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <cortex_a78c.h>
#include <cpu_macros.S>
#include <plat_macros.S>
#include "wa_cve_2022_23960_bhb_vector.S"

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "cortex_a78c must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

#if WORKAROUND_CVE_2022_23960
	wa_cve_2022_23960_bhb_vector_table CORTEX_A78C_BHB_LOOP_COUNT, cortex_a78c
#endif /* WORKAROUND_CVE_2022_23960 */

workaround_reset_start cortex_a78c, ERRATUM(1827430), ERRATA_A78C_1827430
	/* Disable allocation of splintered pages in the L2 TLB */
	sysreg_bit_set CORTEX_A78C_CPUECTLR_EL1, CORTEX_A78C_CPUECTLR_EL1_MM_ASP_EN
workaround_reset_end cortex_a78c, ERRATUM(1827430)

check_erratum_ls cortex_a78c, ERRATUM(1827430), CPU_REV(0, 0)

workaround_reset_start cortex_a78c, ERRATUM(1827440), ERRATA_A78C_1827440
	/* Force Atomic Store to WB memory be done in L1 data cache */
	sysreg_bit_set CORTEX_A78C_CPUACTLR2_EL1, BIT(2)
workaround_reset_end cortex_a78c, ERRATUM(1827440)

check_erratum_ls cortex_a78c, ERRATUM(1827440), CPU_REV(0, 0)

workaround_reset_start cortex_a78c, ERRATUM(2132064), ERRATA_A78C_2132064
	/* --------------------------------------------------------
	 * Place the data prefetcher in the most conservative mode
	 * to reduce prefetches by writing the following bits to
	 * the value indicated: ecltr[7:6], PF_MODE = 2'b11
	 * --------------------------------------------------------
	 */
	sysreg_bit_set CORTEX_A78C_CPUECTLR_EL1, (CORTEX_A78C_CPUECTLR_EL1_BIT_6 | CORTEX_A78C_CPUECTLR_EL1_BIT_7)
workaround_reset_end cortex_a78c, ERRATUM(2132064)

check_erratum_range cortex_a78c, ERRATUM(2132064), CPU_REV(0, 1), CPU_REV(0, 2)

workaround_reset_start cortex_a78c, ERRATUM(2242638), ERRATA_A78C_2242638
	ldr	x0, =0x5
	msr	CORTEX_A78C_IMP_CPUPSELR_EL3, x0
	ldr	x0, =0x10F600E000
	msr	CORTEX_A78C_IMP_CPUPOR_EL3, x0
	ldr	x0, =0x10FF80E000
	msr	CORTEX_A78C_IMP_CPUPMR_EL3, x0
	ldr	x0, =0x80000000003FF
	msr	CORTEX_A78C_IMP_CPUPCR_EL3, x0
workaround_reset_end cortex_a78c, ERRATUM(2242638)

check_erratum_range cortex_a78c, ERRATUM(2242638), CPU_REV(0, 1), CPU_REV(0, 2)

workaround_reset_start cortex_a78c, ERRATUM(2376749), ERRATA_A78C_2376749
	sysreg_bit_set CORTEX_A78C_CPUACTLR2_EL1, CORTEX_A78C_CPUACTLR2_EL1_BIT_0
workaround_reset_end cortex_a78c, ERRATUM(2376749)

check_erratum_range cortex_a78c, ERRATUM(2376749), CPU_REV(0, 1), CPU_REV(0, 2)

workaround_reset_start cortex_a78c, ERRATUM(2395411), ERRATA_A78C_2395411
	sysreg_bit_set CORTEX_A78C_CPUACTLR2_EL1, CORTEX_A78C_CPUACTLR2_EL1_BIT_40
workaround_reset_end cortex_a78c, ERRATUM(2395411)

check_erratum_range cortex_a78c, ERRATUM(2395411), CPU_REV(0, 1), CPU_REV(0, 2)

workaround_runtime_start cortex_a78c, ERRATUM(2772121), ERRATA_A78C_2772121
	/* dsb before isb of power down sequence */
	dsb	sy
workaround_runtime_end cortex_a78c, ERRATUM(2772121)

check_erratum_ls cortex_a78c, ERRATUM(2772121), CPU_REV(0, 2)

workaround_reset_start cortex_a78c, ERRATUM(2779484), ERRATA_A78C_2779484
	sysreg_bit_set CORTEX_A78C_ACTLR3_EL1, BIT(47)
workaround_reset_end cortex_a78c, ERRATUM(2779484)

check_erratum_range cortex_a78c, ERRATUM(2779484), CPU_REV(0, 1), CPU_REV(0, 2)

check_erratum_chosen cortex_a78c, CVE(2022, 23960), WORKAROUND_CVE_2022_23960

workaround_reset_start cortex_a78c, CVE(2022, 23960), WORKAROUND_CVE_2022_23960
#if IMAGE_BL31
	/*
	 * The Cortex-A78c generic vectors are overridden to apply errata
	 * mitigation on exception entry from lower ELs.
	 */
	override_vector_table wa_cve_vbar_cortex_a78c
#endif /* IMAGE_BL31 */
workaround_reset_end cortex_a78c, CVE(2022, 23960)

cpu_reset_func_start cortex_a78c
cpu_reset_func_end cortex_a78c

errata_report_shim cortex_a78c

	/* ----------------------------------------------------
	 * HW will do the cache maintenance while powering down
	 * ----------------------------------------------------
	 */
func cortex_a78c_core_pwr_dwn
	/* ---------------------------------------------------
	 * Enable CPU power down bit in power control register
	 * ---------------------------------------------------
	 */
	sysreg_bit_set CORTEX_A78C_CPUPWRCTLR_EL1, CORTEX_A78C_CPUPWRCTLR_EL1_CORE_PWRDN_EN_BIT

	apply_erratum cortex_a78c, ERRATUM(2772121), ERRATA_A78C_2772121

	isb
	ret
endfunc cortex_a78c_core_pwr_dwn

	/* ---------------------------------------------
	 * This function provides cortex_a78c specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.cortex_a78c_regs, "aS"
cortex_a78c_regs:  /* The ascii list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func cortex_a78c_cpu_reg_dump
	adr	x6, cortex_a78c_regs
	mrs	x8, CORTEX_A78C_CPUECTLR_EL1
	ret
endfunc cortex_a78c_cpu_reg_dump

declare_cpu_ops cortex_a78c, CORTEX_A78C_MIDR, \
	cortex_a78c_reset_func, \
	cortex_a78c_core_pwr_dwn
