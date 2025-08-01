/*
 * Copyright (c) 2014-2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <arch.h>
#include <asm_macros.S>
#include <cpu_macros.S>
#include <qemu_max.h>

cpu_reset_prologue qemu_max

func qemu_max_core_pwr_dwn
	/* ---------------------------------------------
	 * Flush L1 cache to L2.
	 * ---------------------------------------------
	 */
	mov	x18, lr
	mov	x0, #DCCISW
	bl	dcsw_op_level1
	mov	lr, x18
	ret
endfunc qemu_max_core_pwr_dwn

func qemu_max_cluster_pwr_dwn
	/* ---------------------------------------------
	 * Flush all caches to PoC.
	 * ---------------------------------------------
	 */
	mov	x0, #DCCISW
	b	dcsw_op_all
endfunc qemu_max_cluster_pwr_dwn

cpu_reset_func_start qemu_max
cpu_reset_func_end qemu_max

	/* ---------------------------------------------
	 * This function provides cpu specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.qemu_max_regs, "aS"
qemu_max_regs:  /* The ascii list of register names to be reported */
	.asciz	"" /* no registers to report */

func qemu_max_cpu_reg_dump
	adr	x6, qemu_max_regs
	ret
endfunc qemu_max_cpu_reg_dump


/* cpu_ops for QEMU MAX */
declare_cpu_ops qemu_max, QEMU_MAX_MIDR, qemu_max_reset_func, \
	qemu_max_core_pwr_dwn, \
	qemu_max_cluster_pwr_dwn
