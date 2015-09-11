/*
 * Copyright (c) 2013-2014, ARM Limited and Contributors. All rights reserved.
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

	.globl	get_afflvl_shift
	.globl	mpidr_mask_lower_afflvls
	.globl	eret
	.globl	smc

	.globl	zeromem16
	.globl	memcpy16

	.globl	disable_mmu_el3
	.globl	disable_mmu_icache_el3

#if SUPPORT_VFP
	.globl	enable_vfp
#endif

func get_afflvl_shift
	cmp	x0, #3
	cinc	x0, x0, eq
	mov	x1, #MPIDR_AFFLVL_SHIFT
	lsl	x0, x0, x1
	ret
endfunc get_afflvl_shift

func mpidr_mask_lower_afflvls
	cmp	x1, #3
	cinc	x1, x1, eq
	mov	x2, #MPIDR_AFFLVL_SHIFT
	lsl	x2, x1, x2
	lsr	x0, x0, x2
	lsl	x0, x0, x2
	ret
endfunc mpidr_mask_lower_afflvls


func eret
	eret
endfunc eret


func smc
	smc	#0
endfunc smc

/* -----------------------------------------------------------------------
 * void zeromem16(void *mem, unsigned int length);
 *
 * Initialise a memory region to 0.
 * The memory address must be 16-byte aligned.
 * -----------------------------------------------------------------------
 */
func zeromem16
#if ASM_ASSERTION
	tst	x0, #0xf
	ASM_ASSERT(eq)
#endif
	add	x2, x0, x1
/* zero 16 bytes at a time */
z_loop16:
	sub	x3, x2, x0
	cmp	x3, #16
	b.lt	z_loop1
	stp	xzr, xzr, [x0], #16
	b	z_loop16
/* zero byte per byte */
z_loop1:
	cmp	x0, x2
	b.eq	z_end
	strb	wzr, [x0], #1
	b	z_loop1
z_end:
	ret
endfunc zeromem16


/* --------------------------------------------------------------------------
 * void memcpy16(void *dest, const void *src, unsigned int length)
 *
 * Copy length bytes from memory area src to memory area dest.
 * The memory areas should not overlap.
 * Destination and source addresses must be 16-byte aligned.
 * --------------------------------------------------------------------------
 */
func memcpy16
#if ASM_ASSERTION
	orr	x3, x0, x1
	tst	x3, #0xf
	ASM_ASSERT(eq)
#endif
/* copy 16 bytes at a time */
m_loop16:
	cmp	x2, #16
	b.lt	m_loop1
	ldp	x3, x4, [x1], #16
	stp	x3, x4, [x0], #16
	sub	x2, x2, #16
	b	m_loop16
/* copy byte per byte */
m_loop1:
	cbz	x2, m_end
	ldrb	w3, [x1], #1
	strb	w3, [x0], #1
	subs	x2, x2, #1
	b.ne	m_loop1
m_end:
	ret
endfunc memcpy16

/* ---------------------------------------------------------------------------
 * Disable the MMU at EL3
 * ---------------------------------------------------------------------------
 */

func disable_mmu_el3
	mov	x1, #(SCTLR_M_BIT | SCTLR_C_BIT)
do_disable_mmu:
	mrs	x0, sctlr_el3
	bic	x0, x0, x1
	msr	sctlr_el3, x0
	isb				// ensure MMU is off
	dsb	sy
	ret
endfunc disable_mmu_el3


func disable_mmu_icache_el3
	mov	x1, #(SCTLR_M_BIT | SCTLR_C_BIT | SCTLR_I_BIT)
	b	do_disable_mmu
endfunc disable_mmu_icache_el3

/* ---------------------------------------------------------------------------
 * Enable the use of VFP at EL3
 * ---------------------------------------------------------------------------
 */
#if SUPPORT_VFP
func enable_vfp
	mrs	x0, cpacr_el1
	orr	x0, x0, #CPACR_VFP_BITS
	msr	cpacr_el1, x0
	mrs	x0, cptr_el3
	mov	x1, #AARCH64_CPTR_TFP
	bic	x0, x0, x1
	msr	cptr_el3, x0
	isb
	ret
endfunc enable_vfp
#endif
