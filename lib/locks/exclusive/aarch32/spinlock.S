/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <asm_macros.S>

	.globl	spin_lock
	.globl	spin_unlock


func spin_lock
	mov	r2, #1
1:
	ldrex	r1, [r0]
	cmp	r1, #0
	wfene
	strexeq	r1, r2, [r0]
	cmpeq	r1, #0
	bne	1b
	dmb
	bx	lr
endfunc spin_lock


func spin_unlock
	mov	r1, #0
	stl	r1, [r0]
	bx	lr
endfunc spin_unlock
