/*
 * Copyright (c) 2020-2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <generic.h>
#include <cpu_macros.S>
#include <plat_macros.S>

cpu_reset_prologue generic

func generic_core_pwr_dwn
	mov	x18, x30

	/* ---------------------------------------------
	 * Flush L1 caches.
	 * ---------------------------------------------
	 */
	mov	x0, #DCCISW
	bl	dcsw_op_level1

	ret	x18
endfunc generic_core_pwr_dwn

func generic_cluster_pwr_dwn
	mov	x18, x30

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

	ret	x18

endfunc generic_cluster_pwr_dwn

/* ---------------------------------------------
 * Unimplemented functions.
 * ---------------------------------------------
 */
.equ	generic_cpu_reg_dump,		0

cpu_reset_func_start generic
cpu_reset_func_end generic

declare_cpu_ops generic, AARCH64_GENERIC_MIDR, \
	generic_reset_func, \
	generic_core_pwr_dwn, \
	generic_cluster_pwr_dwn
