/********************************************************************
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *********************************************************************/

/*
 * Copyright (c) 2015-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <assert_macros.S>
#include <cpu_macros.S>
#include <plat_macros.S>
#include <qti_cpu.h>

	/* ---------------------------------------------
	 * Clean RW sections used by ATF
	 * This includes 'data' and 'bss'
	 * ---------------------------------------------
	 */
	.p2align 3
.type qti_kryo3_gold_clean_atf, %function
qti_kryo3_gold_clean_atf:
	mov	x17, x30
	// clean .data section till POC
	ldr	x0, =__DATA_START__
	ldr	x1, =__DATA_END__
	sub	x1, x1, x0
	bl	clean_dcache_range
	// clean .bss section till POC
	ldr	x0, =__BSS_START__
	ldr	x1, =__BSS_END__
	sub	x1, x1, x0
	bl	clean_dcache_range
	mov	x30, x17
	ret	// End of function

	/* ---------------------------------------------
	 * Disable L1 data cache and unified L2 cache
	 * ---------------------------------------------
	 */
	.p2align 3
func qti_kryo3_gold_disable_dcache
	mrs	x1, sctlr_el3
	bic	x1, x1, #SCTLR_C_BIT
	msr	sctlr_el3, x1
	isb
	ret
endfunc qti_kryo3_gold_disable_dcache

/* -------------------------------------------------
 * The CPU Ops reset function for Kryo-3 Gold
 * -------------------------------------------------
 */
func qti_kryo3_gold_reset_func
#if IMAGE_BL31 && WORKAROUND_CVE_2017_5715
	adr x0, wa_cve_2017_5715_bpiall_vbar
	msr vbar_el3, x0
	isb
#endif

	mov x19, x30

	bl qtiseclib_kryo3_gold_reset_func

	mov	x30, x19
	ret

endfunc qti_kryo3_gold_reset_func

/* ----------------------------------------------------
 * The CPU Ops core power down function for Kryo-3 Gold
 * ----------------------------------------------------
 */
func qti_kryo3_gold_core_pwr_dwn
	ret
endfunc qti_kryo3_gold_core_pwr_dwn

/* -------------------------------------------------------
 * The CPU Ops cluster power down function for Kryo-3 Gold
 * -------------------------------------------------------
 */
func qti_kryo3_gold_cluster_pwr_dwn
	ret
endfunc qti_kryo3_gold_cluster_pwr_dwn

/* ---------------------------------------------
 * This function provides kryo3_gold specific
 * register information for crash reporting.
 * It needs to return with x6 pointing to
 * a list of register names in ascii and
 * x8 - x15 having values of registers to be
 * reported.
 * ---------------------------------------------
 */
.section .rodata.qti_kryo3_gold_regs, "aS"
qti_kryo3_gold_regs:  /* The ascii list of register names to be reported */
	.asciz	""

func qti_kryo3_gold_cpu_reg_dump
	adr	x6, qti_kryo3_gold_regs
	ret
endfunc qti_kryo3_gold_cpu_reg_dump

declare_cpu_ops	qti_kryo3_gold, QTI_KRYO3_GOLD_MIDR,	\
		qti_kryo3_gold_reset_func,		\
		qti_kryo3_gold_core_pwr_dwn,	\
		qti_kryo3_gold_cluster_pwr_dwn
