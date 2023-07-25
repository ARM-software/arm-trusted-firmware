/*
 * Copyright (c) 2022-2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <neoverse_poseidon.h>
#include <cpu_macros.S>
#include <plat_macros.S>
#include "wa_cve_2022_23960_bhb_vector.S"

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "Neoverse Poseidon must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

/* 64-bit only core */
#if CTX_INCLUDE_AARCH32_REGS == 1
#error "Neoverse Poseidon supports only AArch64. Compile with CTX_INCLUDE_AARCH32_REGS=0"
#endif

#if WORKAROUND_CVE_2022_23960
	wa_cve_2022_23960_bhb_vector_table NEOVERSE_POSEIDON_BHB_LOOP_COUNT, neoverse_poseidon
#endif /* WORKAROUND_CVE_2022_23960 */

workaround_reset_start neoverse_poseidon, CVE(2022,23960), WORKAROUND_CVE_2022_23960
#if IMAGE_BL31
	/*
	 * The Neoverse-poseidon generic vectors are overridden to apply errata
         * mitigation on exception entry from lower ELs.
	 */
	override_vector_table wa_cve_vbar_neoverse_poseidon

#endif /* IMAGE_BL31 */
workaround_reset_end neoverse_poseidon, CVE(2022,23960)

check_erratum_chosen neoverse_poseidon, CVE(2022, 23960), WORKAROUND_CVE_2022_23960

	/* ---------------------------------------------
	 * HW will do the cache maintenance while powering down
	 * ---------------------------------------------
	 */
func neoverse_poseidon_core_pwr_dwn
	/* ---------------------------------------------
	 * Enable CPU power down bit in power control register
	 * ---------------------------------------------
	 */
	sysreg_bit_set NEOVERSE_POSEIDON_CPUPWRCTLR_EL1, \
		NEOVERSE_POSEIDON_CPUPWRCTLR_EL1_CORE_PWRDN_BIT

	isb
	ret
endfunc neoverse_poseidon_core_pwr_dwn

cpu_reset_func_start neoverse_poseidon
	/* Disable speculative loads */
	msr	SSBS, xzr
cpu_reset_func_end neoverse_poseidon

errata_report_shim neoverse_poseidon

	/* ---------------------------------------------
	 * This function provides Neoverse-Poseidon specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.neoverse_poseidon_regs, "aS"
neoverse_poseidon_regs:  /* The ascii list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func neoverse_poseidon_cpu_reg_dump
	adr	x6, neoverse_poseidon_regs
	mrs	x8, NEOVERSE_POSEIDON_CPUECTLR_EL1
	ret
endfunc neoverse_poseidon_cpu_reg_dump

declare_cpu_ops neoverse_poseidon, NEOVERSE_POSEIDON_MIDR, \
	neoverse_poseidon_reset_func, \
	neoverse_poseidon_core_pwr_dwn
