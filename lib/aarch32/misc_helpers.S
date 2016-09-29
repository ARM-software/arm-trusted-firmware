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

#include <arch.h>
#include <asm_macros.S>
#include <assert_macros.S>

	.globl	smc
	.globl	zeromem
	.globl	memcpy4
	.globl	disable_mmu_icache_secure
	.globl	disable_mmu_secure

func smc
	/*
	 * For AArch32 only r0-r3 will be in the registers;
	 * rest r4-r6 will be pushed on to the stack. So here, we'll
	 * have to load them from the stack to registers r4-r6 explicitly.
	 * Clobbers: r4-r6
	 */
	ldm	sp, {r4, r5, r6}
	smc	#0
endfunc smc

/* -----------------------------------------------------------------------
 * void zeromem(void *mem, unsigned int length);
 *
 * Initialise a memory region to 0.
 * The memory address and length must be 4-byte aligned.
 * -----------------------------------------------------------------------
 */
func zeromem
#if ASM_ASSERTION
	tst	r0, #0x3
	ASM_ASSERT(eq)
	tst	r1, #0x3
	ASM_ASSERT(eq)
#endif
	add	r2, r0, r1
	mov	r1, #0
z_loop:
	cmp	r2, r0
	beq	z_end
	str	r1, [r0], #4
	b	z_loop
z_end:
	bx	lr
endfunc zeromem

/* --------------------------------------------------------------------------
 * void memcpy4(void *dest, const void *src, unsigned int length)
 *
 * Copy length bytes from memory area src to memory area dest.
 * The memory areas should not overlap.
 * Destination and source addresses must be 4-byte aligned.
 * --------------------------------------------------------------------------
 */
func memcpy4
#if ASM_ASSERTION
	orr	r3, r0, r1
	tst	r3, #0x3
	ASM_ASSERT(eq)
#endif
/* copy 4 bytes at a time */
m_loop4:
	cmp	r2, #4
	blt	m_loop1
	ldr	r3, [r1], #4
	str	r3, [r0], #4
	sub	r2, r2, #4
	b	m_loop4
/* copy byte per byte */
m_loop1:
	cmp	r2,#0
	beq	m_end
	ldrb	r3, [r1], #1
	strb	r3, [r0], #1
	subs	r2, r2, #1
	bne	m_loop1
m_end:
	bx	lr
endfunc memcpy4

/* ---------------------------------------------------------------------------
 * Disable the MMU in Secure State
 * ---------------------------------------------------------------------------
 */

func disable_mmu_secure
	mov	r1, #(SCTLR_M_BIT | SCTLR_C_BIT)
do_disable_mmu:
	ldcopr	r0, SCTLR
	bic	r0, r0, r1
	stcopr	r0, SCTLR
	isb				// ensure MMU is off
	dsb	sy
	bx	lr
endfunc disable_mmu_secure


func disable_mmu_icache_secure
	ldr	r1, =(SCTLR_M_BIT | SCTLR_C_BIT | SCTLR_I_BIT)
	b	do_disable_mmu
endfunc disable_mmu_icache_secure
