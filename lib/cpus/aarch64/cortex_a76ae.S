/*
 * Copyright (c) 2019-2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <cortex_a76ae.h>
#include <cpu_macros.S>
#include "wa_cve_2022_23960_bhb_vector.S"

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "Cortex-A76AE must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

/* 64-bit only core */
#if CTX_INCLUDE_AARCH32_REGS == 1
#error "Cortex-A76AE supports only AArch64. Compile with CTX_INCLUDE_AARCH32_REGS=0"
#endif

cpu_reset_prologue cortex_a76ae

#if WORKAROUND_CVE_2022_23960
	wa_cve_2022_23960_bhb_vector_table CORTEX_A76AE_BHB_LOOP_COUNT, cortex_a76ae
#endif /* WORKAROUND_CVE_2022_23960 */

check_erratum_chosen cortex_a76ae, CVE(2022, 23960), WORKAROUND_CVE_2022_23960

workaround_reset_start cortex_a76ae, ERRATUM(1931427), ERRATA_A76AE_1931427
	sysreg_bit_set CORTEX_A76AE_CPUACTLR2_EL1, BIT(2)
workaround_reset_end cortex_a76ae, ERRATUM(1931427)

check_erratum_ls cortex_a76ae, ERRATUM(1931427), CPU_REV(1, 0)

workaround_reset_start cortex_a76ae, ERRATUM(1931435), ERRATA_A76AE_1931435
	sysreg_bit_set CORTEX_A76AE_CPUACTLR_EL1, BIT(13)
workaround_reset_end cortex_a76ae, ERRATUM(1931435)

check_erratum_ls cortex_a76ae, ERRATUM(1931435), CPU_REV(1, 0)

workaround_reset_start cortex_a76ae, ERRATUM(1969401), ERRATA_A76AE_1969401
	ldr x0,=0x3
	msr CORTEX_A76AE_CPUPSELR_EL3,x0
	ldr x0,=0x10E3900002
	msr CORTEX_A76AE_CPUPOR_EL3,x0
	ldr x0,=0x10FFF00083
	msr CORTEX_A76AE_CPUPMR_EL3,x0
	ldr x0,=0x2001003FF
	msr CORTEX_A76AE_CPUPCR_EL3,x0

	ldr x0,=0x4
	msr CORTEX_A76AE_CPUPSELR_EL3,x0
	ldr x0,=0x10E3800082
	msr CORTEX_A76AE_CPUPOR_EL3,x0
	ldr x0,= 0x10FFF00083
	msr CORTEX_A76AE_CPUPMR_EL3,x0
	ldr x0,=0x2001003FF
	msr CORTEX_A76AE_CPUPCR_EL3,x0

	ldr x0,=0x5
	msr CORTEX_A76AE_CPUPSELR_EL3,x0
	ldr x0,=0x10E3800200
	msr CORTEX_A76AE_CPUPOR_EL3,x0
	ldr x0,=0x10FFF003E0
	msr CORTEX_A76AE_CPUPMR_EL3,x0
	ldr x0,=0x2001003FF
	msr CORTEX_A76AE_CPUPCR_EL3,x0
workaround_reset_end cortex_a76ae, ERRATUM(1969401)

check_erratum_ls cortex_a76ae, ERRATUM(1969401), CPU_REV(1, 0)

workaround_reset_start cortex_a76ae, ERRATUM(2371140), ERRATA_A76AE_2371140
	sysreg_bit_set CORTEX_A76AE_CPUACTLR2_EL1, BIT(0)
workaround_reset_end cortex_a76ae, ERRATUM(2371140)

check_erratum_ls cortex_a76ae, ERRATUM(2371140), CPU_REV(1, 1)

workaround_runtime_start cortex_a76ae, ERRATUM(2753838), ERRATA_A76AE_2753838
	dsb sy
workaround_runtime_end cortex_a76ae, ERRATUM(2753838)

check_erratum_ls cortex_a76ae, ERRATUM(2753838), CPU_REV(1, 1)

workaround_reset_start cortex_a76ae, CVE(2022, 23960), WORKAROUND_CVE_2022_23960
#if IMAGE_BL31
	/*
	 * The Cortex-A76ae generic vectors are overridden to apply errata
	 * mitigation on exception entry from lower ELs.
	 */
	override_vector_table wa_cve_vbar_cortex_a76ae
	isb
#endif /* IMAGE_BL31 */
workaround_reset_end cortex_a76ae, CVE(2022, 23960)

cpu_reset_func_start cortex_a76ae
cpu_reset_func_end cortex_a76ae

	/* ----------------------------------------------------
	 * HW will do the cache maintenance while powering down
	 * ----------------------------------------------------
	 */
func cortex_a76ae_core_pwr_dwn
	sysreg_bit_set CORTEX_A76AE_CPUPWRCTLR_EL1, CORTEX_A76AE_CORE_PWRDN_EN_MASK
	apply_erratum cortex_a76ae, ERRATUM(2753838), ERRATA_A76AE_2753838
	isb
	ret
endfunc cortex_a76ae_core_pwr_dwn

	/* ---------------------------------------------
	 * This function provides cortex_a76ae specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.cortex_a76ae_regs, "aS"
cortex_a76ae_regs:  /* The ASCII list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func cortex_a76ae_cpu_reg_dump
	adr	x6, cortex_a76ae_regs
	mrs	x8, CORTEX_A76AE_CPUECTLR_EL1
	ret
endfunc cortex_a76ae_cpu_reg_dump

declare_cpu_ops cortex_a76ae, CORTEX_A76AE_MIDR, cortex_a76ae_reset_func, \
	cortex_a76ae_core_pwr_dwn
