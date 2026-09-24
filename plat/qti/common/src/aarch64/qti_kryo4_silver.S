/*
 * Copyright (c) 2015-2024, Arm Limited and Contributors. All rights reserved.
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



declare_cpu_ops	qti_kryo4_silver, QTI_KRYO4_SILVER_MIDR,	\
		qti_kryo4_silver_reset_func,		\
		cortex_a55_core_pwr_dwn,		\
		qti_kryo4_silver_cluster_pwr_dwn
