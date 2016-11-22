/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
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
	stp	x1, x2, [x8]
	stp	x3, x4, [x8, #16]

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
