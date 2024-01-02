/*
 * Copyright (c) 2021-2024, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <cortex_a715.h>
#include <cpu_macros.S>
#include <plat_macros.S>
#include "wa_cve_2022_23960_bhb_vector.S"

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "Cortex-A715 must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

/* 64-bit only core */
#if CTX_INCLUDE_AARCH32_REGS == 1
#error "Cortex-A715 supports only AArch64. Compile with CTX_INCLUDE_AARCH32_REGS=0"
#endif

#if WORKAROUND_CVE_2022_23960
	wa_cve_2022_23960_bhb_vector_table CORTEX_A715_BHB_LOOP_COUNT, cortex_a715
#endif /* WORKAROUND_CVE_2022_23960 */

workaround_reset_start cortex_a715, ERRATUM(2331818), ERRATA_A715_2331818
        sysreg_bit_set CORTEX_A715_CPUACTLR2_EL1, BIT(20)
workaround_reset_end cortex_a715, ERRATUM(2331818)

check_erratum_ls cortex_a715, ERRATUM(2331818), CPU_REV(1, 0)

workaround_reset_start cortex_a715, ERRATUM(2344187), ERRATA_A715_2344187
	/* GCR_EL1 is only present with FEAT_MTE2. */
	mrs x1, ID_AA64PFR1_EL1
	ubfx x0, x1, ID_AA64PFR1_EL1_MTE_SHIFT, #4
	cmp x0, #MTE_IMPLEMENTED_ELX
	bne #1f
	sysreg_bit_set GCR_EL1, GCR_EL1_RRND_BIT

1:
	/* Mitigation upon ERETAA and ERETAB. */
	mov x0, #2
	msr CORTEX_A715_CPUPSELR_EL3, x0
	isb
	ldr x0, =0xd69f0bff
	msr CORTEX_A715_CPUPOR_EL3, x0
	ldr x0, =0xfffffbff
	msr CORTEX_A715_CPUPMR_EL3, x0
	mov x1, #0
	orr x1, x1, #(1<<0)
	orr x1, x1, #(3<<4)
	orr x1, x1, #(0xf<<6)
	orr x1, x1, #(1<<13)
	orr x1, x1, #(1<<53)
	msr CORTEX_A715_CPUPCR_EL3, x1
workaround_reset_end cortex_a715, ERRATUM(2344187)

check_erratum_ls cortex_a715, ERRATUM(2344187), CPU_REV(1, 0)

workaround_reset_start cortex_a715, ERRATUM(2420947), ERRATA_A715_2420947
        sysreg_bit_set CORTEX_A715_CPUACTLR2_EL1, BIT(33)
workaround_reset_end cortex_a715, ERRATUM(2420947)

check_erratum_range cortex_a715, ERRATUM(2420947), CPU_REV(1, 0), CPU_REV(1, 0)

workaround_reset_start cortex_a715, ERRATUM(2429384), ERRATA_A715_2429384
        sysreg_bit_set CORTEX_A715_CPUACTLR2_EL1, BIT(27)
workaround_reset_end cortex_a715, ERRATUM(2429384)

check_erratum_range cortex_a715, ERRATUM(2429384), CPU_REV(1, 0), CPU_REV(1, 0)

workaround_runtime_start cortex_a715, ERRATUM(2561034), ERRATA_A715_2561034
	sysreg_bit_set	CORTEX_A715_CPUACTLR2_EL1, BIT(26)
workaround_runtime_end cortex_a715, ERRATUM(2561034), NO_ISB

check_erratum_range cortex_a715, ERRATUM(2561034), CPU_REV(1, 0), CPU_REV(1, 0)

workaround_reset_start cortex_a715, CVE(2022, 23960), WORKAROUND_CVE_2022_23960
#if IMAGE_BL31
	/*
	 * The Cortex-A715 generic vectors are overridden to apply errata
	 * mitigation on exception entry from lower ELs.
	 */
	override_vector_table wa_cve_vbar_cortex_a715
#endif /* IMAGE_BL31 */
workaround_reset_end cortex_a715, CVE(2022, 23960)

check_erratum_chosen cortex_a715, CVE(2022, 23960), WORKAROUND_CVE_2022_23960

cpu_reset_func_start cortex_a715
	/* Disable speculative loads */
	msr	SSBS, xzr
cpu_reset_func_end cortex_a715

	/* ----------------------------------------------------
	 * HW will do the cache maintenance while powering down
	 * ----------------------------------------------------
	 */
func cortex_a715_core_pwr_dwn
	/* ---------------------------------------------------
	 * Enable CPU power down bit in power control register
	 * ---------------------------------------------------
	 */
	mrs	x0, CORTEX_A715_CPUPWRCTLR_EL1
	orr	x0, x0, #CORTEX_A715_CPUPWRCTLR_EL1_CORE_PWRDN_BIT
	msr	CORTEX_A715_CPUPWRCTLR_EL1, x0
	isb
	ret
endfunc cortex_a715_core_pwr_dwn

errata_report_shim cortex_a715

	/* ---------------------------------------------
	 * This function provides Cortex-A715 specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.cortex_a715_regs, "aS"
cortex_a715_regs:  /* The ascii list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func cortex_a715_cpu_reg_dump
	adr	x6, cortex_a715_regs
	mrs	x8, CORTEX_A715_CPUECTLR_EL1
	ret
endfunc cortex_a715_cpu_reg_dump

declare_cpu_ops cortex_a715, CORTEX_A715_MIDR, \
	cortex_a715_reset_func, \
	cortex_a715_core_pwr_dwn
