/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

	.globl	rom_lib_init
	.extern	__DATA_RAM_START__, __DATA_ROM_START__, __DATA_RAM_END__
	.extern	memset, memcpy

rom_lib_init:
	cmp	w0, #1
	mov	w0, #0
	b.le	1f
	ret

1:	stp	x29, x30, [sp, #-16]!
	adrp	x0, __DATA_RAM_START__
	adrp	x1, __DATA_ROM_START__
	add	x1, x1, :lo12:__DATA_ROM_START__
	adrp	x2, __DATA_RAM_END__
	add	x2, x2, :lo12:__DATA_RAM_END__
	sub	x2, x2, x0
	bl	memcpy

	adrp	x0,__BSS_START__
	add	x0, x0, :lo12:__BSS_START__
	mov	x1, #0
	adrp	x2, __BSS_END__
	add	x2, x2, :lo12:__BSS_END__
	sub	x2, x2, x0
	bl	memset
	ldp	x29, x30, [sp], #16

	mov	w0, #1
	ret
