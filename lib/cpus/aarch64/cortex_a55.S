/*
 * Copyright (c) 2017-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <cortex_a55.h>
#include <cpu_macros.S>
#include <plat_macros.S>

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "Cortex-A55 must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

	.globl cortex_a55_reset_func
	.globl cortex_a55_core_pwr_dwn

/* ERRATA_DSU_798953:
 * The errata is defined in dsu_helpers.S but applies to cortex_a55
 * as well. Henceforth creating symbolic names to the already existing errata
 * workaround functions to get them registered under the Errata Framework.
 */
.equ check_erratum_cortex_a55_798953, check_errata_dsu_798953
.equ erratum_cortex_a55_798953_wa, errata_dsu_798953_wa
add_erratum_entry cortex_a55, ERRATUM(798953), ERRATA_DSU_798953, APPLY_AT_RESET

/* ERRATA_DSU_936184:
 * The errata is defined in dsu_helpers.S but applies to cortex_a55
 * as well. Henceforth creating symbolic names to the already existing errata
 * workaround functions to get them registered under the Errata Framework.
 */
.equ check_erratum_cortex_a55_936184, check_errata_dsu_936184
.equ erratum_cortex_a55_936184_wa, errata_dsu_936184_wa
add_erratum_entry cortex_a55, ERRATUM(936184), ERRATA_DSU_936184, APPLY_AT_RESET

workaround_reset_start cortex_a55, ERRATUM(768277), ERRATA_A55_768277
	sysreg_bit_set CORTEX_A55_CPUACTLR_EL1, CORTEX_A55_CPUACTLR_EL1_DISABLE_DUAL_ISSUE
workaround_reset_end cortex_a55, ERRATUM(768277)

check_erratum_ls cortex_a55, ERRATUM(768277), CPU_REV(0, 0)

workaround_reset_start cortex_a55, ERRATUM(778703), ERRATA_A55_778703
	sysreg_bit_set CORTEX_A55_CPUECTLR_EL1, CORTEX_A55_CPUECTLR_EL1_L1WSCTL
	sysreg_bit_set CORTEX_A55_CPUACTLR_EL1, CORTEX_A55_CPUACTLR_EL1_DISABLE_WRITE_STREAMING
workaround_reset_end cortex_a55, ERRATUM(778703)

check_erratum_custom_start cortex_a55, ERRATUM(778703)
	mov	x16, x30
	mov	x1, #0x00
	bl	cpu_rev_var_ls
	/*
	 * Check that no private L2 cache is configured
	 */
	mrs	x1, CORTEX_A55_CLIDR_EL1
	and	x1, x1, CORTEX_A55_CLIDR_EL1_CTYPE3
	cmp	x1, #0
	mov	x2, #ERRATA_NOT_APPLIES
	csel	x0, x0, x2, eq
	ret	x16
check_erratum_custom_end cortex_a55, ERRATUM(778703)

workaround_reset_start cortex_a55, ERRATUM(798797), ERRATA_A55_798797
	sysreg_bit_set CORTEX_A55_CPUACTLR_EL1, CORTEX_A55_CPUACTLR_EL1_DISABLE_L1_PAGEWALKS
workaround_reset_end cortex_a55, ERRATUM(798797)

check_erratum_ls cortex_a55, ERRATUM(798797), CPU_REV(0, 0)

workaround_reset_start cortex_a55, ERRATUM(846532), ERRATA_A55_846532
	sysreg_bit_set CORTEX_A55_CPUACTLR_EL1, CORTEX_A55_CPUACTLR_EL1_DISABLE_DUAL_ISSUE
workaround_reset_end cortex_a55, ERRATUM(846532)

check_erratum_ls cortex_a55, ERRATUM(846532), CPU_REV(0, 1)

workaround_reset_start cortex_a55, ERRATUM(903758), ERRATA_A55_903758
	sysreg_bit_set CORTEX_A55_CPUACTLR_EL1, CORTEX_A55_CPUACTLR_EL1_DISABLE_L1_PAGEWALKS
workaround_reset_end cortex_a55, ERRATUM(903758)

check_erratum_ls cortex_a55, ERRATUM(903758), CPU_REV(0, 1)

workaround_reset_start cortex_a55, ERRATUM(1221012), ERRATA_A55_1221012
	mov	x0, #0x0020
	movk	x0, #0x0850, lsl #16
	msr	CPUPOR_EL3, x0
	mov	x0, #0x0000
	movk	x0, #0x1FF0, lsl #16
	movk	x0, #0x2, lsl #32
	msr	CPUPMR_EL3, x0
	mov	x0, #0x03fd
	movk	x0, #0x0110, lsl #16
	msr	CPUPCR_EL3, x0
	mov	x0, #0x1
	msr	CPUPSELR_EL3, x0
	mov	x0, #0x0040
	movk	x0, #0x08D0, lsl #16
	msr	CPUPOR_EL3, x0
	mov	x0, #0x0040
	movk	x0, #0x1FF0, lsl #16
	movk	x0, #0x2, lsl #32
	msr	CPUPMR_EL3, x0
	mov	x0, #0x03fd
	movk	x0, #0x0110, lsl #16
	msr	CPUPCR_EL3, x0
workaround_reset_end cortex_a55, ERRATUM(1221012)

check_erratum_ls cortex_a55, ERRATUM(1221012), CPU_REV(1, 0)

check_erratum_chosen cortex_a55, ERRATUM(1530923), ERRATA_A55_1530923

/* erratum has no workaround in the cpu. Generic code must take care */
add_erratum_entry cortex_a55, ERRATUM(1530923), ERRATA_A55_1530923, NO_APPLY_AT_RESET

cpu_reset_func_start cortex_a55
cpu_reset_func_end cortex_a55

errata_report_shim cortex_a55

	/* ---------------------------------------------
	 * HW will do the cache maintenance while powering down
	 * ---------------------------------------------
	 */
func cortex_a55_core_pwr_dwn
	sysreg_bit_set CORTEX_A55_CPUPWRCTLR_EL1, CORTEX_A55_CORE_PWRDN_EN_MASK
	isb
	ret
endfunc cortex_a55_core_pwr_dwn

	/* ---------------------------------------------
	 * This function provides cortex_a55 specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.cortex_a55_regs, "aS"
cortex_a55_regs:  /* The ascii list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func cortex_a55_cpu_reg_dump
	adr	x6, cortex_a55_regs
	mrs	x8, CORTEX_A55_CPUECTLR_EL1
	ret
endfunc cortex_a55_cpu_reg_dump

declare_cpu_ops cortex_a55, CORTEX_A55_MIDR, \
	cortex_a55_reset_func, \
	cortex_a55_core_pwr_dwn
