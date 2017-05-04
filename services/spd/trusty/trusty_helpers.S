/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <asm_macros.S>

.macro push ra, rb, sp=sp
	stp \ra, \rb, [\sp,#-16]!
.endm

.macro pop ra, rb, sp=sp
	ldp \ra, \rb, [\sp], #16
.endm

	.global trusty_context_switch_helper
func trusty_context_switch_helper
	push	x8, xzr
	push	x19, x20
	push	x21, x22
	push	x23, x24
	push	x25, x26
	push	x27, x28
	push	x29, x30

	mov	x9, sp
	ldr	x10, [x0]
	mov	sp, x10
	str	x9, [x0]

	pop	x29, x30
	pop	x27, x28
	pop	x25, x26
	pop	x23, x24
	pop	x21, x22
	pop	x19, x20
	pop	x8, xzr

        ldr     x2, [x1]
        ldr     x3, [x1, #0x08]
        ldr     x4, [x1, #0x10]
        ldr     x5, [x1, #0x18]
        ldr     x6, [x1, #0x20]
        ldr     x7, [x1, #0x28]
        ldr     x10, [x1, #0x30]
        ldr     x11, [x1, #0x38]

        stp     x2, x3, [x8]
        stp     x4, x5, [x8, #16]
        stp     x6, x7, [x8, #32]
        stp     x10, x11, [x8, #48]

	ret
endfunc trusty_context_switch_helper

	.global trusty_init_context_stack
func trusty_init_context_stack
	push	x8, xzr, x1
	push	xzr, xzr, x1
	push	xzr, xzr, x1
	push	xzr, xzr, x1
	push	xzr, xzr, x1
	push	xzr, xzr, x1
	adr	x9, el3_exit
	push	xzr, x9, x1
	str	x1, [x0]
	ret
endfunc trusty_init_context_stack
