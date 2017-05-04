/*
 * Copyright (c) 2015-2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <assert_macros.S>
#include <denver.h>
#include <cpu_macros.S>
#include <plat_macros.S>

	.global	denver_disable_dco

	/* ---------------------------------------------
	 * Disable debug interfaces
	 * ---------------------------------------------
	 */
func denver_disable_ext_debug
	mov	x0, #1
	msr	osdlr_el1, x0
	isb
	dsb	sy
	ret
endfunc denver_disable_ext_debug

	/* ----------------------------------------------------
	 * Enable dynamic code optimizer (DCO)
	 * ----------------------------------------------------
	 */
func denver_enable_dco
	mrs	x0, mpidr_el1
	and	x0, x0, #0xF
	mov	x1, #1
	lsl	x1, x1, x0
	msr	s3_0_c15_c0_2, x1
	ret
endfunc denver_enable_dco

	/* ----------------------------------------------------
	 * Disable dynamic code optimizer (DCO)
	 * ----------------------------------------------------
	 */
func denver_disable_dco

	/* turn off background work */
	mrs	x0, mpidr_el1
	and	x0, x0, #0xF
	mov	x1, #1
	lsl	x1, x1, x0
	lsl	x2, x1, #16
	msr	s3_0_c15_c0_2, x2
	isb

	/* wait till the background work turns off */
1:	mrs	x2, s3_0_c15_c0_2
	lsr	x2, x2, #32
	and	w2, w2, 0xFFFF
	and	x2, x2, x1
	cbnz	x2, 1b

	ret
endfunc denver_disable_dco

	/* -------------------------------------------------
	 * The CPU Ops reset function for Denver.
	 * -------------------------------------------------
	 */
func denver_reset_func

	mov	x19, x30

	/* ----------------------------------------------------
	 * Enable dynamic code optimizer (DCO)
	 * ----------------------------------------------------
	 */
	bl	denver_enable_dco

	ret	x19
endfunc denver_reset_func

	/* ----------------------------------------------------
	 * The CPU Ops core power down function for Denver.
	 * ----------------------------------------------------
	 */
func denver_core_pwr_dwn

	mov	x19, x30

	/* ---------------------------------------------
	 * Force the debug interfaces to be quiescent
	 * ---------------------------------------------
	 */
	bl	denver_disable_ext_debug

	ret	x19
endfunc denver_core_pwr_dwn

	/* -------------------------------------------------------
	 * The CPU Ops cluster power down function for Denver.
	 * -------------------------------------------------------
	 */
func denver_cluster_pwr_dwn
	ret
endfunc denver_cluster_pwr_dwn

	/* ---------------------------------------------
	 * This function provides Denver specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.denver_regs, "aS"
denver_regs:  /* The ascii list of register names to be reported */
	.asciz	"actlr_el1", ""

func denver_cpu_reg_dump
	adr	x6, denver_regs
	mrs	x8, ACTLR_EL1
	ret
endfunc denver_cpu_reg_dump

declare_cpu_ops denver, DENVER_MIDR_PN0, \
	denver_reset_func, \
	denver_core_pwr_dwn, \
	denver_cluster_pwr_dwn

declare_cpu_ops denver, DENVER_MIDR_PN1, \
	denver_reset_func, \
	denver_core_pwr_dwn, \
	denver_cluster_pwr_dwn

declare_cpu_ops denver, DENVER_MIDR_PN2, \
	denver_reset_func, \
	denver_core_pwr_dwn, \
	denver_cluster_pwr_dwn

declare_cpu_ops denver, DENVER_MIDR_PN3, \
	denver_reset_func, \
	denver_core_pwr_dwn, \
	denver_cluster_pwr_dwn

declare_cpu_ops denver, DENVER_MIDR_PN4, \
	denver_reset_func, \
	denver_core_pwr_dwn, \
	denver_cluster_pwr_dwn
