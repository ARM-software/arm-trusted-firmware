/*
 * Copyright (c) 2021-2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <neoverse_v2.h>
#include <cpu_macros.S>
#include <plat_macros.S>
#include "wa_cve_2022_23960_bhb_vector.S"

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "Neoverse V2 must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

/* 64-bit only core */
#if CTX_INCLUDE_AARCH32_REGS == 1
#error "Neoverse V2 supports only AArch64. Compile with CTX_INCLUDE_AARCH32_REGS=0"
#endif

workaround_reset_start neoverse_v2, ERRATUM(2331132), ERRATA_V2_2331132
	sysreg_bitfield_insert NEOVERSE_V2_CPUECTLR2_EL1, NEOVERSE_V2_CPUECTLR2_EL1_PF_MODE_CNSRV, \
		NEOVERSE_V2_CPUECTLR2_EL1_PF_MODE_LSB, NEOVERSE_V2_CPUECTLR2_EL1_PF_MODE_WIDTH
workaround_reset_end neoverse_v2, ERRATUM(2331132)

check_erratum_ls neoverse_v2, ERRATUM(2331132), CPU_REV(0, 2)

workaround_reset_start neoverse_v2, ERRATUM(2719105), ERRATA_V2_2719105
	sysreg_bit_set NEOVERSE_V2_CPUACTLR2_EL1, NEOVERSE_V2_CPUACTLR2_EL1_BIT_0
workaround_reset_end neoverse_v2, ERRATUM(2719105)

check_erratum_ls neoverse_v2, ERRATUM(2719105), CPU_REV(0, 1)

workaround_reset_start neoverse_v2, ERRATUM(2743011), ERRATA_V2_2743011
	sysreg_bit_set NEOVERSE_V2_CPUACTLR5_EL1, NEOVERSE_V2_CPUACTLR5_EL1_BIT_55
	sysreg_bit_clear NEOVERSE_V2_CPUACTLR5_EL1, NEOVERSE_V2_CPUACTLR5_EL1_BIT_56
workaround_reset_end neoverse_v2, ERRATUM(2743011)

check_erratum_ls neoverse_v2, ERRATUM(2743011), CPU_REV(0, 1)

workaround_reset_start neoverse_v2, ERRATUM(2779510), ERRATA_V2_2779510
	sysreg_bit_set NEOVERSE_V2_CPUACTLR3_EL1, NEOVERSE_V2_CPUACTLR3_EL1_BIT_47
workaround_reset_end neoverse_v2, ERRATUM(2779510)

check_erratum_ls neoverse_v2, ERRATUM(2779510), CPU_REV(0, 1)

workaround_runtime_start neoverse_v2, ERRATUM(2801372), ERRATA_V2_2801372
	/* dsb before isb of power down sequence */
	dsb	sy
workaround_runtime_end neoverse_v2, ERRATUM(2801372), ERRATA_V2_2801372

check_erratum_ls neoverse_v2, ERRATUM(2801372), CPU_REV(0, 1)

workaround_reset_start neoverse_v2, CVE(2022,23960), WORKAROUND_CVE_2022_23960
#if IMAGE_BL31
	/*
	 * The Neoverse-V2 generic vectors are overridden to apply errata
         * mitigation on exception entry from lower ELs.
	 */
	override_vector_table wa_cve_vbar_neoverse_v2
#endif /* IMAGE_BL31 */
workaround_reset_end neoverse_v2, CVE(2022,23960)

check_erratum_chosen neoverse_v2, CVE(2022, 23960), WORKAROUND_CVE_2022_23960

#if WORKAROUND_CVE_2022_23960
	wa_cve_2022_23960_bhb_vector_table NEOVERSE_V2_BHB_LOOP_COUNT, neoverse_v2
#endif /* WORKAROUND_CVE_2022_23960 */

	/* ----------------------------------------------------
	 * HW will do the cache maintenance while powering down
	 * ----------------------------------------------------
	 */
func neoverse_v2_core_pwr_dwn
	/* ---------------------------------------------------
	 * Enable CPU power down bit in power control register
	 * ---------------------------------------------------
	 */
	sysreg_bit_set NEOVERSE_V2_CPUPWRCTLR_EL1, NEOVERSE_V2_CPUPWRCTLR_EL1_CORE_PWRDN_BIT
	apply_erratum neoverse_v2, ERRATUM(2801372), ERRATA_V2_2801372

	isb
	ret
endfunc neoverse_v2_core_pwr_dwn

cpu_reset_func_start neoverse_v2
	/* Disable speculative loads */
	msr	SSBS, xzr
cpu_reset_func_end neoverse_v2

errata_report_shim neoverse_v2
	/* ---------------------------------------------
	 * This function provides Neoverse V2-
	 * specific register information for crash
	 * reporting. It needs to return with x6
	 * pointing to a list of register names in ascii
	 * and x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.neoverse_v2_regs, "aS"
neoverse_v2_regs:  /* The ascii list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func neoverse_v2_cpu_reg_dump
	adr	x6, neoverse_v2_regs
	mrs	x8, NEOVERSE_V2_CPUECTLR_EL1
	ret
endfunc neoverse_v2_cpu_reg_dump

declare_cpu_ops neoverse_v2, NEOVERSE_V2_MIDR, \
	neoverse_v2_reset_func, \
	neoverse_v2_core_pwr_dwn
