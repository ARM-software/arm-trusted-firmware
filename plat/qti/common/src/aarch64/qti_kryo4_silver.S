/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2018-2020, The Linux Foundation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <cpu_macros.S>

#include <plat_macros.S>
#include <qti_cpu.h>

	.p2align 3

/* -------------------------------------------------
 * The CPU Ops reset function for Kryo-3 Silver
 * -------------------------------------------------
 */
func qti_kryo4_silver_reset_func
	mov	x19, x30

	bl	qtiseclib_kryo4_silver_reset_asm
	mov	x30, x19
	b	cortex_a55_reset_func

endfunc qti_kryo4_silver_reset_func

/* ---------------------------------------------------------
 * The CPU Ops cluster power down function for Kryo-3 Silver
 * ---------------------------------------------------------
 */
func qti_kryo4_silver_cluster_pwr_dwn
	ret
endfunc qti_kryo4_silver_cluster_pwr_dwn

#if REPORT_ERRATA
/*
 * Errata printing function for Kryo4 Silver. Must follow AAPCS.
 */
func qti_kryo4_silver_errata_report
	/* TODO : Need to add support. Required only for debug bl31 image.*/
	ret
endfunc qti_kryo4_silver_errata_report
#endif


/* ---------------------------------------------
 * This function provides kryo4_silver specific
 * register information for crash reporting.
 * It needs to return with x6 pointing to
 * a list of register names in ASCII and
 * x8 - x15 having values of registers to be
 * reported.
 * ---------------------------------------------
 */
.section .rodata.qti_kryo4_silver_regs, "aS"
qti_kryo4_silver_regs:  /* The ASCII list of register names to be reported */
	.asciz	""

func qti_kryo4_silver_cpu_reg_dump
	adr	x6, qti_kryo4_silver_regs
	ret
endfunc qti_kryo4_silver_cpu_reg_dump


declare_cpu_ops	qti_kryo4_silver, QTI_KRYO4_SILVER_MIDR,	\
		qti_kryo4_silver_reset_func,		\
		cortex_a55_core_pwr_dwn,		\
		qti_kryo4_silver_cluster_pwr_dwn
