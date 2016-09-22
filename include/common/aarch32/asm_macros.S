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
#ifndef __ASM_MACROS_S__
#define __ASM_MACROS_S__

#include <arch.h>
#include <asm_macros_common.S>

#define WORD_SIZE	4

	/*
	 * Co processor register accessors
	 */
	.macro ldcopr reg, coproc, opc1, CRn, CRm, opc2
	mrc	\coproc, \opc1, \reg, \CRn, \CRm, \opc2
	.endm

	.macro ldcopr16 reg1, reg2, coproc, opc1, CRm
	mrrc	\coproc, \opc1, \reg1, \reg2, \CRm
	.endm

	.macro stcopr reg, coproc, opc1, CRn, CRm, opc2
	mcr	\coproc, \opc1, \reg, \CRn, \CRm, \opc2
	.endm

	.macro stcopr16 reg1, reg2, coproc, opc1, CRm
	mcrr	\coproc, \opc1, \reg1, \reg2, \CRm
	.endm

	/* Cache line size helpers */
	.macro	dcache_line_size  reg, tmp
	ldcopr	\tmp, CTR
	ubfx	\tmp, \tmp, #CTR_DMINLINE_SHIFT, #CTR_DMINLINE_WIDTH
	mov	\reg, #WORD_SIZE
	lsl	\reg, \reg, \tmp
	.endm

	.macro	icache_line_size  reg, tmp
	ldcopr	\tmp, CTR
	and	\tmp, \tmp, #CTR_IMINLINE_MASK
	mov	\reg, #WORD_SIZE
	lsl	\reg, \reg, \tmp
	.endm

	/*
	 * Declare the exception vector table, enforcing it is aligned on a
	 * 32 byte boundary.
	 */
	.macro vector_base  label
	.section .vectors, "ax"
	.align 5
	\label:
	.endm

	/*
	 * This macro calculates the base address of the current CPU's multi
	 * processor(MP) stack using the plat_my_core_pos() index, the name of
	 * the stack storage and the size of each stack.
	 * Out: r0 = physical address of stack base
	 * Clobber: r14, r1, r2
	 */
	.macro get_my_mp_stack _name, _size
	bl  plat_my_core_pos
	ldr r2, =(\_name + \_size)
	mov r1, #\_size
	mla r0, r0, r1, r2
	.endm

	/*
	 * This macro calculates the base address of a uniprocessor(UP) stack
	 * using the name of the stack storage and the size of the stack
	 * Out: r0 = physical address of stack base
	 */
	.macro get_up_stack _name, _size
	ldr r0, =(\_name + \_size)
	.endm

#endif /* __ASM_MACROS_S__ */
