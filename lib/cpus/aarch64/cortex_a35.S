/*
 * Copyright (c) 2016-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <cortex_a35.h>
#include <cpu_macros.S>
#include <plat_macros.S>

	/* ---------------------------------------------
	 * Disable L1 data cache and unified L2 cache
	 * ---------------------------------------------
	 */
func cortex_a35_disable_dcache
	sysreg_bit_clear sctlr_el3, SCTLR_C_BIT
	isb
	ret
endfunc cortex_a35_disable_dcache

	/* ---------------------------------------------
	 * Disable intra-cluster coherency
	 * ---------------------------------------------
	 */
func cortex_a35_disable_smp
	sysreg_bit_clear CORTEX_A35_CPUECTLR_EL1, CORTEX_A35_CPUECTLR_SMPEN_BIT
	isb
	dsb	sy
	ret
endfunc cortex_a35_disable_smp

workaround_reset_start cortex_a35, ERRATUM(855472), ERRATA_A35_855472
	sysreg_bit_set CORTEX_A35_CPUACTLR_EL1, CORTEX_A35_CPUACTLR_EL1_ENDCCASCI
workaround_reset_end cortex_a35, ERRATUM(855472)

check_erratum_ls cortex_a35, ERRATUM(855472), CPU_REV(0, 0)

	/* -------------------------------------------------
	 * The CPU Ops reset function for Cortex-A35.
	 * -------------------------------------------------
	 */
cpu_reset_func_start cortex_a35
	/* ---------------------------------------------
	 * Enable the SMP bit.
	 * ---------------------------------------------
	 */
	sysreg_bit_set CORTEX_A35_CPUECTLR_EL1, CORTEX_A35_CPUECTLR_SMPEN_BIT
cpu_reset_func_end cortex_a35

func cortex_a35_core_pwr_dwn
	mov	x18, x30

	/* ---------------------------------------------
	 * Turn off caches.
	 * ---------------------------------------------
	 */
	bl	cortex_a35_disable_dcache

	/* ---------------------------------------------
	 * Flush L1 caches.
	 * ---------------------------------------------
	 */
	mov	x0, #DCCISW
	bl	dcsw_op_level1

	/* ---------------------------------------------
	 * Come out of intra cluster coherency
	 * ---------------------------------------------
	 */
	mov	x30, x18
	b	cortex_a35_disable_smp
endfunc cortex_a35_core_pwr_dwn

func cortex_a35_cluster_pwr_dwn
	mov	x18, x30

	/* ---------------------------------------------
	 * Turn off caches.
	 * ---------------------------------------------
	 */
	bl	cortex_a35_disable_dcache

	/* ---------------------------------------------
	 * Flush L1 caches.
	 * ---------------------------------------------
	 */
	mov	x0, #DCCISW
	bl	dcsw_op_level1

	/* ---------------------------------------------
	 * Disable the optional ACP.
	 * ---------------------------------------------
	 */
	bl	plat_disable_acp

	/* ---------------------------------------------
	 * Flush L2 caches.
	 * ---------------------------------------------
	 */
	mov	x0, #DCCISW
	bl	dcsw_op_level2

	/* ---------------------------------------------
	 * Come out of intra cluster coherency
	 * ---------------------------------------------
	 */
	mov	x30, x18
	b	cortex_a35_disable_smp
endfunc cortex_a35_cluster_pwr_dwn

errata_report_shim cortex_a35

	/* ---------------------------------------------
	 * This function provides cortex_a35 specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.cortex_a35_regs, "aS"
cortex_a35_regs:  /* The ascii list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func cortex_a35_cpu_reg_dump
	adr	x6, cortex_a35_regs
	mrs	x8, CORTEX_A35_CPUECTLR_EL1
	ret
endfunc cortex_a35_cpu_reg_dump

declare_cpu_ops cortex_a35, CORTEX_A35_MIDR, \
	cortex_a35_reset_func, \
	cortex_a35_core_pwr_dwn, \
	cortex_a35_cluster_pwr_dwn
