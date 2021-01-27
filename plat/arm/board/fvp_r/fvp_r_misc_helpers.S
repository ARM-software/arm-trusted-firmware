/*
 * Copyright (c) 2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <asm_macros.S>

	.globl	disable_mpu_el2
	.globl	disable_mpu_icache_el2

/* ---------------------------------------------------------------------------
 * Disable the MPU at EL2.
 * ---------------------------------------------------------------------------
 */

func disable_mpu_el2
	mov	x1, #(SCTLR_M_BIT | SCTLR_C_BIT)
do_disable_mpu_el2:
	mrs	x0, sctlr_el2
	bic	x0, x0, x1
	msr	sctlr_el2, x0
	isb	/* ensure MMU is off */
	dsb	sy
	ret
endfunc disable_mpu_el2


func disable_mpu_icache_el2
	mov	x1, #(SCTLR_M_BIT | SCTLR_C_BIT | SCTLR_I_BIT)
	b	do_disable_mpu_el2
endfunc disable_mpu_icache_el2
