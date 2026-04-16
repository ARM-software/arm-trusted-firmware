/*
 * Copyright (c) 2023-2026, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <c1_ultra.h>
#include <common/bl_common.h>
#include <cpu_macros.S>
#include <wa_cve_2025_0647_cpprctx.h>

#include <plat_macros.S>

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "Arm C1-Ultra must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

/* 64-bit only core */
#if CTX_INCLUDE_AARCH32_REGS == 1
#error "Arm C1-Ultra supports only AArch64. Compile with CTX_INCLUDE_AARCH32_REGS=0"
#endif

#if ERRATA_SME_POWER_DOWN == 0
#error "Arm C1-Ultra needs ERRATA_SME_POWER_DOWN=1 to powerdown correctly"
#endif

cpu_reset_prologue c1_ultra

workaround_runtime_start c1_ultra, ERRATUM(3324333), ERRATA_C1ULTRA_3324333
	speculation_barrier
workaround_runtime_end c1_ultra, ERRATUM(3324333)

check_erratum_ls c1_ultra, ERRATUM(3324333), CPU_REV(0, 0)

workaround_reset_start c1_ultra, ERRATUM(3502731), ERRATA_C1ULTRA_3502731
	sysreg_bit_set C1_ULTRA_IMP_CPUACTLR4_EL1, BIT(23)
workaround_reset_end c1_ultra, ERRATUM(3502731)

check_erratum_ls c1_ultra, ERRATUM(3502731), CPU_REV(0, 0)

.global check_erratum_c1_ultra_3658374
add_erratum_entry c1_ultra, ERRATUM(3658374), ERRATA_C1ULTRA_3658374
check_erratum_ls c1_ultra, ERRATUM(3658374), CPU_REV(1, 0)

workaround_reset_start c1_ultra, ERRATUM(3684152), ERRATA_C1ULTRA_3684152
	sysreg_bitfield_insert C1_ULTRA_IMP_CPUACTLR_EL1, C1_ULTRA_IMP_CPUACTLR_EL1_LOAD_BIT, \
	C1_ULTRA_IMP_CPUACTLR_EL1_LOAD_SHIFT, C1_ULTRA_IMP_CPUACTLR_EL1_LOAD_WIDTH
workaround_reset_end c1_ultra, ERRATUM(3684152)

check_erratum_ls c1_ultra, ERRATUM(3684152), CPU_REV(0, 0)

workaround_reset_start c1_ultra, ERRATUM(3705939), ERRATA_C1ULTRA_3705939
	sysreg_bit_set C1_ULTRA_IMP_CPUACTLR_EL1, BIT(48)
workaround_reset_end c1_ultra, ERRATUM(3705939)

check_erratum_ls c1_ultra, ERRATUM(3705939), CPU_REV(1, 0)

workaround_reset_start c1_ultra, ERRATUM(3815514), ERRATA_C1ULTRA_3815514
	sysreg_bit_set C1_ULTRA_IMP_CPUACTLR5_EL1, BIT(13)
workaround_reset_end c1_ultra, ERRATUM(3815514)

check_erratum_ls c1_ultra, ERRATUM(3815514), CPU_REV(1, 0)

workaround_reset_start c1_ultra, ERRATUM(3865171), ERRATA_C1ULTRA_3865171
	sysreg_bit_set C1_ULTRA_IMP_CPUACTLR2_EL1, BIT(22)
workaround_reset_end c1_ultra, ERRATUM(3865171)

check_erratum_ls c1_ultra, ERRATUM(3865171), CPU_REV(1, 0)

workaround_reset_start c1_ultra, ERRATUM(3926381), ERRATA_C1ULTRA_3926381
	/* Convert WFx to NOP */
	ldr x0,=0x0
	msr C1_ULTRA_IMP_CPUPSELR_EL3, x0
	ldr x0,=0xD503205f
	msr C1_ULTRA_IMP_CPUPOR_EL3, x0
	ldr x0,=0xFFFFFFDF
	msr C1_ULTRA_IMP_CPUPMR_EL3, x0
	ldr x0,=0x1000002043ff
	msr C1_ULTRA_IMP_CPUPCR_EL3, x0
	/* Convert WFxT to NOP */
	ldr x0,=0x1
	msr C1_ULTRA_IMP_CPUPSELR_EL3, x0
	ldr x0,=0xD5031000
	msr C1_ULTRA_IMP_CPUPOR_EL3, x0
	ldr x0,=0xFFFFFFC0
	msr C1_ULTRA_IMP_CPUPMR_EL3, x0
	ldr x0,=0x1000002043ff
	msr C1_ULTRA_IMP_CPUPCR_EL3, x0
	isb
workaround_reset_end c1_ultra, ERRATUM(3926381)

check_erratum_range c1_ultra, ERRATUM(3926381), CPU_REV(1, 0), CPU_REV(1, 0)

workaround_reset_start c1_ultra, ERRATUM(4102704), ERRATA_C1ULTRA_4102704
	sysreg_bit_set C1_ULTRA_IMP_CPUACTLR4_EL1, BIT(23)
workaround_reset_end c1_ultra, ERRATUM(4102704)

check_erratum_ls c1_ultra, ERRATUM(4102704), CPU_REV(1, 0)

	/* -------------------------------------------------------------
	 * CVE-2024-7881 is mitigated for C1-Ultra using erratum 3651221
	 * workaround by disabling the affected prefetcher setting
	 * CPUACTLR6_EL1[41].
	 * -------------------------------------------------------------
	 */
workaround_reset_start c1_ultra, CVE(2024, 7881), WORKAROUND_CVE_2024_7881
	sysreg_bit_set C1_ULTRA_IMP_CPUACTLR6_EL1, BIT(41)
workaround_reset_end c1_ultra, CVE(2024, 7881)

check_erratum_ls c1_ultra, CVE(2024, 7881), CPU_REV(0, 0)

	/*
	 * Instruction patch sequence to trap 'cpp rctx' instructions to EL3.
	 * Enables mitigation for CVE-2025-0647.
	 */
workaround_reset_start c1_ultra, CVE(2025, 647), WORKAROUND_CVE_2025_0647
#if IMAGE_BL31
	mov	x0, #WA_PATCH_SLOT(3)
	bl	wa_cve_2025_0647_instruction_patch
#endif /* IMAGE_BL31 */
workaround_reset_end c1_ultra, CVE(2025, 647)

check_erratum_chosen c1_ultra, CVE(2025, 647), WORKAROUND_CVE_2025_0647

#if WORKAROUND_CVE_2025_0647
func c1_ultra_impl_defined_el3_handler
	mov	x0, #WA_LS_RCG_EN

	/* See if this call came from trap handler. */
	cmp	x1, #EC_IMP_DEF_EL3
	bne	wa_cve_2025_0647_do_cpp_wa
	orr	x0, x0, #WA_IS_TRAP_HANDLER
	b	wa_cve_2025_0647_do_cpp_wa
endfunc c1_ultra_impl_defined_el3_handler
#endif

cpu_reset_func_start c1_ultra
	/* ----------------------------------------------------
	 * Disable speculative loads
	 * ----------------------------------------------------
	 */
	msr	SSBS, xzr
	apply_erratum c1_ultra, ERRATUM(3324333), ERRATA_C1ULTRA_3324333
	/* model bug: not cleared on reset */
	sysreg_bit_clear C1_ULTRA_IMP_CPUPWRCTLR_EL1, \
		C1_ULTRA_IMP_CPUPWRCTLR_EL1_CORE_PWRDN_EN_BIT
	enable_mpmm
cpu_reset_func_end c1_ultra

func c1_ultra_core_pwr_dwn
	/* ---------------------------------------------------
	 * Flip CPU power down bit in power control register.
	 * It will be set on powerdown and cleared on wakeup
	 * ---------------------------------------------------
	 */
	sysreg_bit_toggle C1_ULTRA_IMP_CPUPWRCTLR_EL1, \
		C1_ULTRA_IMP_CPUPWRCTLR_EL1_CORE_PWRDN_EN_BIT
	isb
	signal_pabandon_handled
	ret
endfunc c1_ultra_core_pwr_dwn

.section .rodata.c1_ultra_regs, "aS"
c1_ultra_regs: /* The ASCII list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func c1_ultra_cpu_reg_dump
	adr 	x6, c1_ultra_regs
	mrs	x8, C1_ULTRA_IMP_CPUECTLR_EL1
	ret
endfunc c1_ultra_cpu_reg_dump

#if WORKAROUND_CVE_2025_0647 && IMAGE_BL31
declare_cpu_ops_eh c1_ultra, C1_ULTRA_MIDR, \
	c1_ultra_reset_func, \
	c1_ultra_impl_defined_el3_handler, \
	c1_ultra_core_pwr_dwn
#else
declare_cpu_ops c1_ultra, C1_ULTRA_MIDR, \
	c1_ultra_reset_func, \
	c1_ultra_core_pwr_dwn
#endif
