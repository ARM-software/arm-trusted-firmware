/*
 * Copyright (c) 2021-2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <cortex_a720.h>
#include <cpu_macros.S>
#include <dsu_macros.S>
#include <plat_macros.S>
#include "wa_cve_2022_23960_bhb_vector.S"

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "Cortex A720 must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

/* 64-bit only core */
#if CTX_INCLUDE_AARCH32_REGS == 1
#error "Cortex A720 supports only AArch64. Compile with CTX_INCLUDE_AARCH32_REGS=0"
#endif

cpu_reset_prologue cortex_a720

.global check_erratum_cortex_a720_3699561

workaround_reset_start cortex_a720, ERRATUM(2729604), ERRATA_A720_2729604
	sysreg_bit_set CORTEX_A720_CPUACTLR_EL1, (BIT(60) | BIT(61))
workaround_reset_end cortex_a720, ERRATUM(2729604)

check_erratum_ls cortex_a720, ERRATUM(2729604), CPU_REV(0, 1)

workaround_reset_start cortex_a720, ERRATUM(2792132), ERRATA_A720_2792132
        sysreg_bit_set CORTEX_A720_CPUACTLR2_EL1, BIT(26)
workaround_reset_end cortex_a720, ERRATUM(2792132)

check_erratum_ls cortex_a720, ERRATUM(2792132), CPU_REV(0, 1)

workaround_reset_start cortex_a720, ERRATUM(2844092), ERRATA_A720_2844092
        sysreg_bit_set CORTEX_A720_CPUACTLR4_EL1, BIT(11)
workaround_reset_end cortex_a720, ERRATUM(2844092)

check_erratum_ls cortex_a720, ERRATUM(2844092), CPU_REV(0, 1)

workaround_reset_start cortex_a720, ERRATUM(2900952), ERRATA_DSU_2900952
	errata_dsu_2900952_wa_apply
workaround_reset_end cortex_a720, ERRATUM(2900952)

check_erratum_custom_start cortex_a720, ERRATUM(2900952)
	check_errata_dsu_2900952_applies
	ret
check_erratum_custom_end cortex_a720, ERRATUM(2900952)

workaround_reset_start cortex_a720, ERRATUM(2926083), ERRATA_A720_2926083
/* Erratum 2926083 workaround is required only if SPE is enabled */
#if ENABLE_SPE_FOR_NS != 0
	/* Check if Static profiling extension is implemented or present. */
	mrs x1, id_aa64dfr0_el1
	ubfx x0, x1, ID_AA64DFR0_PMS_SHIFT, #4
	cbz x0, 1f
	/* Apply the workaround by setting CPUACTLR_EL1[58:57] = 0b11. */
	sysreg_bit_set CORTEX_A720_CPUACTLR_EL1, BIT(57)
	sysreg_bit_set CORTEX_A720_CPUACTLR_EL1, BIT(58)
1:
#endif
workaround_reset_end cortex_a720, ERRATUM(2926083)

check_erratum_ls cortex_a720, ERRATUM(2926083), CPU_REV(0, 1)

workaround_reset_start cortex_a720, ERRATUM(2940794), ERRATA_A720_2940794
        sysreg_bit_set CORTEX_A720_CPUACTLR2_EL1, BIT(37)
workaround_reset_end cortex_a720, ERRATUM(2940794)

check_erratum_ls cortex_a720, ERRATUM(2940794), CPU_REV(0, 1)

workaround_runtime_start cortex_a720, ERRATUM(3456091), ERRATA_A720_3456091
	speculation_barrier
workaround_runtime_end cortex_a720, ERRATUM(3456091)

check_erratum_ls cortex_a720, ERRATUM(3456091), CPU_REV(0, 2)

add_erratum_entry cortex_a720, ERRATUM(3699561), ERRATA_A720_3699561

check_erratum_ls cortex_a720, ERRATUM(3699561), CPU_REV(0, 2)

workaround_reset_start cortex_a720, ERRATUM(3711910), ERRATA_A720_3711910
	mov	x0, #5
	msr	CORTEX_A720_CPUPSELR_EL3, x0
	ldr	x0, =0xD503329F
	msr	CORTEX_A720_CPUPOR_EL3, x0
	ldr	x0, =0xFFFFF3FF
	msr	CORTEX_A720_CPUPMR_EL3, x0
	ldr	x0, =0x1004003F1
	msr	CORTEX_A720_CPUPCR_EL3, x0
workaround_reset_end cortex_a720, ERRATUM(3711910)

check_erratum_ls cortex_a720, ERRATUM(3711910), CPU_REV(0, 2)

cpu_reset_func_start cortex_a720
	/* Disable speculative loads */
	msr	SSBS, xzr
	apply_erratum cortex_a720, ERRATUM(3456091), ERRATA_A720_3456091

	enable_mpmm
cpu_reset_func_end cortex_a720

	/* ----------------------------------------------------
	 * HW will do the cache maintenance while powering down
	 * ----------------------------------------------------
	 */
func cortex_a720_core_pwr_dwn
	/* ---------------------------------------------------
	 * Enable CPU power down bit in power control register
	 * ---------------------------------------------------
	 */
	sysreg_bit_set CORTEX_A720_CPUPWRCTLR_EL1, CORTEX_A720_CPUPWRCTLR_EL1_CORE_PWRDN_BIT

	isb
	ret
endfunc cortex_a720_core_pwr_dwn

	/* ---------------------------------------------
	 * This function provides Cortex A720-specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.cortex_a720_regs, "aS"
cortex_a720_regs:  /* The ascii list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func cortex_a720_cpu_reg_dump
	adr	x6, cortex_a720_regs
	mrs	x8, CORTEX_A720_CPUECTLR_EL1
	ret
endfunc cortex_a720_cpu_reg_dump

declare_cpu_ops cortex_a720, CORTEX_A720_MIDR, \
	cortex_a720_reset_func, \
	cortex_a720_core_pwr_dwn
