/*
 * Copyright (c) 2015-2024, Arm Limited and Contributors. All rights reserved.
 * Copyright (c) 2018-2021, The Linux Foundation. All rights reserved.
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
 * The CPU Ops reset function for Kryo-3 Gold
 * -------------------------------------------------
 */
func qti_kryo6_gold_reset_func
#if IMAGE_BL31 && WORKAROUND_CVE_2017_5715
	adr	x0, wa_cve_2017_5715_bpiall_vbar
	msr	vbar_el3, x0
	isb
#endif

	mov	x19, x30

	bl	qtiseclib_kryo6_gold_reset_asm
	mov	x30, x19
	b	cortex_a78_reset_func

endfunc qti_kryo6_gold_reset_func

/* -------------------------------------------------------
 * The CPU Ops cluster power down function for Kryo-3 Gold
 * -------------------------------------------------------
 */
func qti_kryo6_gold_cluster_pwr_dwn
	ret
endfunc qti_kryo6_gold_cluster_pwr_dwn


declare_cpu_ops	qti_kryo6_gold, QTI_KRYO6_GOLD_MIDR,	\
		qti_kryo6_gold_reset_func,		\
		cortex_a78_core_pwr_dwn,	\
		qti_kryo6_gold_cluster_pwr_dwn
