/*
 * Copyright (c) 2022-2026, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <neoverse_v3.h>
#include "wa_cve_2022_23960_bhb_vector.S"

#include <cpu_macros.S>
#include <wa_cve_2025_0647_cpprctx.h>

#include <plat_macros.S>

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "Neoverse V3 must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

/* 64-bit only core */
#if CTX_INCLUDE_AARCH32_REGS == 1
#error "Neoverse V3 supports only AArch64. Compile with CTX_INCLUDE_AARCH32_REGS=0"
#endif

cpu_reset_prologue neoverse_v3

.global check_erratum_neoverse_v3_3701767

workaround_reset_start neoverse_v3, ERRATUM(2970647), ERRATA_V3_2970647
	/* Add ISB before MRS reads of MPIDR_EL1/MIDR_EL1 */
	ldr x0, =0x1
	msr S3_6_c15_c8_0, x0 	/* msr CPUPSELR_EL3, X0 */
	ldr x0, =0xd5380000
	msr S3_6_c15_c8_2, x0 	/* msr CPUPOR_EL3, X0 */
	ldr x0, =0xFFFFFF40
	msr S3_6_c15_c8_3,x0 	/* msr CPUPMR_EL3, X0 */
	ldr x0, =0x000080010033f
	msr S3_6_c15_c8_1, x0	/* msr CPUPCR_EL3, X0 */
	isb
workaround_reset_end neoverse_v3, ERRATUM(2970647)

check_erratum_ls neoverse_v3, ERRATUM(2970647), CPU_REV(0, 0)

workaround_runtime_start neoverse_v3, ERRATUM(3312417), ERRATA_V3_3312417
	speculation_barrier
workaround_runtime_end neoverse_v3, ERRATUM(3312417)

check_erratum_ls neoverse_v3, ERRATUM(3312417), CPU_REV(0, 1)

workaround_reset_start neoverse_v3, ERRATUM(3696307), ERRATA_V3_3696307
	sysreg_bit_set NEOVERSE_V3_CPUACTLR6_EL1, BIT(41)
workaround_reset_end neoverse_v3, ERRATUM(3696307)

check_erratum_ls neoverse_v3, ERRATUM(3696307), CPU_REV(0, 1)

add_erratum_entry neoverse_v3, ERRATUM(3701767), ERRATA_V3_3701767

check_erratum_ls neoverse_v3, ERRATUM(3701767), CPU_REV(0, 2)

workaround_reset_start neoverse_v3, ERRATUM(3734562), ERRATA_V3_3734562
	mov	x0, #2
	msr	NEOVERSE_V3_CPUPSELR_EL3, x0
	ldr	x0, =0xD503225F
	msr	NEOVERSE_V3_CPUPOR_EL3, x0
	mov	x0, 0xFFFFFFFF
	msr	NEOVERSE_V3_CPUPMR_EL3, x0
	ldr	x0, =0x404003FD
	msr	NEOVERSE_V3_CPUPCR_EL3, x0
workaround_reset_end neoverse_v3, ERRATUM(3734562)

check_erratum_ls neoverse_v3, ERRATUM(3734562), CPU_REV(0, 1)

workaround_reset_start neoverse_v3, ERRATUM(3782181), ERRATA_V3_3782181
        /* Disable retention control for WFI and WFE. */
        mrs     x0, NEOVERSE_V3_CPUPWRCTLR_EL1
        bfi     x0, xzr, #NEOVERSE_V3_CPUPWRCTLR_EL1_WFI_RET_CTRL_SHIFT, \
		#NEOVERSE_V3_CPUPWRCTLR_EL1_WFI_RET_CTRL_WIDTH
        bfi     x0, xzr, #NEOVERSE_V3_CPUPWRCTLR_EL1_WFE_RET_CTRL_SHIFT, \
		#NEOVERSE_V3_CPUPWRCTLR_EL1_WFE_RET_CTRL_WIDTH
        msr     NEOVERSE_V3_CPUPWRCTLR_EL1, x0
workaround_reset_end neoverse_v3, ERRATUM(3782181)

check_erratum_range neoverse_v3, ERRATUM(3782181), CPU_REV(0, 1), \
	CPU_REV(0, 1)

workaround_reset_start neoverse_v3, ERRATUM(3864536), ERRATA_V3_3864536
	sysreg_bit_set NEOVERSE_V3_CPUACTLR2_EL1, BIT(22)
workaround_reset_end neoverse_v3, ERRATUM(3864536)

check_erratum_ls neoverse_v3, ERRATUM(3864536), CPU_REV(0, 2)

workaround_reset_start neoverse_v3, ERRATUM(3878291), ERRATA_V3_3878291
	sysreg_bit_set NEOVERSE_V3_CPUACTLR4_EL1, BIT(57)
workaround_reset_end neoverse_v3, ERRATUM(3878291)

check_erratum_ls neoverse_v3, ERRATUM(3878291), CPU_REV(0, 2)

/* Disable hardware page aggregation. Enables mitigation for `CVE-2024-5660` */
workaround_reset_start neoverse_v3, CVE(2024, 5660), WORKAROUND_CVE_2024_5660
	sysreg_bit_set NEOVERSE_V3_CPUECTLR_EL1, BIT(46)
workaround_reset_end neoverse_v3, CVE(2024, 5660)

check_erratum_ls neoverse_v3, CVE(2024, 5660), CPU_REV(0, 1)

	/* ----------------------------------------------------------------
	 * CVE-2024-7881 is mitigated for Neoverse-V3 / Neoverse-V3AE
	 * using erratum 3696307 workaround by disabling the
	 * affected prefetcher setting CPUACTLR6_EL1[41].
	 * ----------------------------------------------------------------
	 */
workaround_reset_start neoverse_v3, CVE(2024, 7881), WORKAROUND_CVE_2024_7881
       sysreg_bit_set NEOVERSE_V3_CPUACTLR6_EL1, BIT(41)
workaround_reset_end neoverse_v3, CVE(2024, 7881)

check_erratum_ls neoverse_v3, CVE(2024, 7881), CPU_REV(0, 1)

	/*
	 * Instruction patch sequence to trap 'cpp rctx' instructions to EL3.
	 * Enables mitigation for CVE-2025-0647.
	 */
workaround_reset_start neoverse_v3, CVE(2025, 647), WORKAROUND_CVE_2025_0647
#if IMAGE_BL31
	mov	x0, #WA_PATCH_SLOT(3)
	bl	wa_cve_2025_0647_instruction_patch
#endif /* IMAGE_BL31 */
workaround_reset_end neoverse_v3, CVE(2025, 647)

check_erratum_chosen neoverse_v3, CVE(2025, 647), WORKAROUND_CVE_2025_0647

#if WORKAROUND_CVE_2025_0647
func neoverse_v3_impl_defined_el3_handler
	mov	x0, #0

	/* See if this call came from trap handler. */
	cmp	x1, #EC_IMP_DEF_EL3
	bne	wa_cve_2025_0647_do_cpp_wa
	orr	x0, x0, #WA_IS_TRAP_HANDLER
	b	wa_cve_2025_0647_do_cpp_wa
endfunc neoverse_v3_impl_defined_el3_handler
#endif

	/* ---------------------------------------------
	 * HW will do the cache maintenance while powering down
	 * ---------------------------------------------
	 */
func neoverse_v3_core_pwr_dwn
	/* ---------------------------------------------
	 * Enable CPU power down bit in power control register
	 * ---------------------------------------------
	 */
	sysreg_bit_set NEOVERSE_V3_CPUPWRCTLR_EL1, \
		NEOVERSE_V3_CPUPWRCTLR_EL1_CORE_PWRDN_BIT

	isb
	ret
endfunc neoverse_v3_core_pwr_dwn

cpu_reset_func_start neoverse_v3
	/* Disable speculative loads */
	msr	SSBS, xzr
	apply_erratum neoverse_v3, ERRATUM(3312417), ERRATA_V3_3312417
cpu_reset_func_end neoverse_v3

	/* ---------------------------------------------
	 * This function provides Neoverse V3 specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.neoverse_v3_regs, "aS"
neoverse_v3_regs:  /* The ascii list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func neoverse_v3_cpu_reg_dump
	adr	x6, neoverse_v3_regs
	mrs	x8, NEOVERSE_V3_CPUECTLR_EL1
	ret
endfunc neoverse_v3_cpu_reg_dump

#if WORKAROUND_CVE_2025_0647 && IMAGE_BL31
declare_cpu_ops_eh neoverse_v3, NEOVERSE_V3_VNAE_MIDR, \
	neoverse_v3_reset_func, \
	neoverse_v3_impl_defined_el3_handler, \
	neoverse_v3_core_pwr_dwn

declare_cpu_ops_eh neoverse_v3, NEOVERSE_V3_MIDR, \
	neoverse_v3_reset_func, \
	neoverse_v3_impl_defined_el3_handler, \
	neoverse_v3_core_pwr_dwn
#else
declare_cpu_ops neoverse_v3, NEOVERSE_V3_VNAE_MIDR, \
	neoverse_v3_reset_func, \
	neoverse_v3_core_pwr_dwn

declare_cpu_ops neoverse_v3, NEOVERSE_V3_MIDR, \
	neoverse_v3_reset_func, \
	neoverse_v3_core_pwr_dwn
#endif
