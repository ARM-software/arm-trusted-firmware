/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

	.globl	rom_lib_init
	.extern	__DATA_RAM_START__, __DATA_ROM_START__, __DATA_SIZE__
	.extern	memset, memcpy

rom_lib_init:
	cmp	w0, #1
	mov	w0, #0
	b.le	1f
	ret

1:	stp	x29, x30, [sp, #-16]!
	adrp	x0, __DATA_RAM_START__
	ldr	x1,= __DATA_ROM_START__
	ldr	x2, =__DATA_SIZE__
	bl	memcpy

	ldr	x0, =__BSS_START__
	mov	x1, #0
	ldr	x2, =__BSS_SIZE__
	bl	memset
	ldp	x29, x30, [sp], #16

	mov	w0, #1
	ret
