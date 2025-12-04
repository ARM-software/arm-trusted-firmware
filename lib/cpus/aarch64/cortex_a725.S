/*
 * Copyright (c) 2023-2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <cortex_a725.h>
#include <cpu_macros.S>
#include <dsu_macros.S>
#include <plat_macros.S>

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "Cortex-A725 must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

/* 64-bit only core */
#if CTX_INCLUDE_AARCH32_REGS == 1
#error "Cortex-A725 supports only AArch64. Compile with CTX_INCLUDE_AARCH32_REGS=0"
#endif

cpu_reset_prologue cortex_a725

.global check_erratum_cortex_a725_3699564

#if ENABLE_SPE_FOR_NS
workaround_reset_start cortex_a725, ERRATUM(2874943), ERRATA_A725_2874943
	sysreg_bit_set CORTEX_A725_CPUACTLR_EL1, BIT(57)
	sysreg_bit_set CORTEX_A725_CPUACTLR_EL1, BIT(58)
workaround_reset_end cortex_a725, ERRATUM(2874943)

check_erratum_ls cortex_a725, ERRATUM(2874943), CPU_REV(0, 0)
#endif

workaround_reset_start cortex_a725, ERRATUM(2900952), ERRATA_DSU_2900952
	errata_dsu_2900952_wa_apply
workaround_reset_end cortex_a725, ERRATUM(2900952)

check_erratum_custom_start cortex_a725, ERRATUM(2900952)
	check_errata_dsu_2900952_applies
	ret
check_erratum_custom_end cortex_a725, ERRATUM(2900952)

workaround_reset_start cortex_a725, ERRATUM(2936490), ERRATA_A725_2936490
	sysreg_bit_set CORTEX_A725_CPUACTLR2_EL1, BIT(37)
workaround_reset_end cortex_a725, ERRATUM(2936490)

check_erratum_ls cortex_a725, ERRATUM(2936490), CPU_REV(0, 0)

workaround_runtime_start cortex_a725, ERRATUM(3456106), ERRATA_A725_3456106
	speculation_barrier
workaround_runtime_end cortex_a725, ERRATUM(3456106)

/* Due to the nature of the errata it is applied unconditionally when chosen */
check_erratum_chosen cortex_a725, ERRATUM(3456106), ERRATA_A725_3456106

add_erratum_entry cortex_a725, ERRATUM(3699564), ERRATA_A725_3699564

check_erratum_ls cortex_a725, ERRATUM(3699564), CPU_REV(0, 1)

workaround_reset_start cortex_a725, ERRATUM(3711914), ERRATA_A725_3711914
	mov x0, #5
	msr CORTEX_A725_CPUPSELR_EL3, x0
	isb

	ldr x0, =0xd503329f
	msr CORTEX_A725_CPUPOR_EL3, x0
	ldr x0, =0xfffff3ff
	msr CORTEX_A725_CPUPMR_EL3, x0
	mov x1, #0
	orr x1, x1, #(1<<0)
	orr x1, x1, #(3<<4)
	orr x1, x1, #(0xf<<6)
	orr x1, x1, #(1<<22)
	orr x1, x1, #(1<<32)
	msr CORTEX_A725_CPUPCR_EL3, x1
	isb
workaround_reset_end cortex_a725, ERRATUM(3711914)

check_erratum_ls cortex_a725, ERRATUM(3711914), CPU_REV(0, 1)

cpu_reset_func_start cortex_a725
	/* Disable speculative loads */
	msr	SSBS, xzr
	apply_erratum cortex_a725, ERRATUM(3456106), ERRATA_A725_3456106
	enable_mpmm
cpu_reset_func_end cortex_a725

	/* ----------------------------------------------------
	 * HW will do the cache maintenance while powering down
	 * ----------------------------------------------------
	 */
func cortex_a725_core_pwr_dwn
	/* ---------------------------------------------------
	 * Enable CPU power down bit in power control register
	 * ---------------------------------------------------
	 */
	sysreg_bit_set CORTEX_A725_CPUPWRCTLR_EL1, CORTEX_A725_CPUPWRCTLR_EL1_CORE_PWRDN_BIT
	isb
	ret
endfunc cortex_a725_core_pwr_dwn

	/* ---------------------------------------------
	 * This function provides Cortex-A725 specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.cortex_a725_regs, "aS"
cortex_a725_regs:  /* The ascii list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func cortex_a725_cpu_reg_dump
	adr	x6, cortex_a725_regs
	mrs	x8, CORTEX_A725_CPUECTLR_EL1
	ret
endfunc cortex_a725_cpu_reg_dump

declare_cpu_ops cortex_a725, CORTEX_A725_MIDR, \
	cortex_a725_reset_func, \
	cortex_a725_core_pwr_dwn
