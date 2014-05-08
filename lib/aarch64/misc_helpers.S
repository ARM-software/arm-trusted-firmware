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

	.globl	enable_irq
	.globl	disable_irq

	.globl	enable_fiq
	.globl	disable_fiq

	.globl	enable_serror
	.globl	disable_serror

	.globl	enable_debug_exceptions
	.globl	disable_debug_exceptions

	.globl	read_daif
	.globl	write_daif

	.globl	read_spsr_el1
	.globl	read_spsr_el2
	.globl	read_spsr_el3

	.globl	write_spsr_el1
	.globl	write_spsr_el2
	.globl	write_spsr_el3

	.globl	read_elr_el1
	.globl	read_elr_el2
	.globl	read_elr_el3

	.globl	write_elr_el1
	.globl	write_elr_el2
	.globl	write_elr_el3

	.globl	get_afflvl_shift
	.globl	mpidr_mask_lower_afflvls
	.globl	dsb
	.globl	isb
	.globl	sev
	.globl	wfe
	.globl	wfi
	.globl	eret
	.globl	smc

	.globl	zeromem16
	.globl	memcpy16

	.globl	disable_mmu_el3
	.globl	disable_mmu_icache_el3


func get_afflvl_shift
	cmp	x0, #3
	cinc	x0, x0, eq
	mov	x1, #MPIDR_AFFLVL_SHIFT
	lsl	x0, x0, x1
	ret

func mpidr_mask_lower_afflvls
	cmp	x1, #3
	cinc	x1, x1, eq
	mov	x2, #MPIDR_AFFLVL_SHIFT
	lsl	x2, x1, x2
	lsr	x0, x0, x2
	lsl	x0, x0, x2
	ret

	/* -----------------------------------------------------
	 * Asynchronous exception manipulation accessors
	 * -----------------------------------------------------
	 */
func enable_irq
	msr	daifclr, #DAIF_IRQ_BIT
	ret


func enable_fiq
	msr	daifclr, #DAIF_FIQ_BIT
	ret


func enable_serror
	msr	daifclr, #DAIF_ABT_BIT
	ret


func enable_debug_exceptions
	msr	daifclr, #DAIF_DBG_BIT
	ret


func disable_irq
	msr	daifset, #DAIF_IRQ_BIT
	ret


func disable_fiq
	msr	daifset, #DAIF_FIQ_BIT
	ret


func disable_serror
	msr	daifset, #DAIF_ABT_BIT
	ret


func disable_debug_exceptions
	msr	daifset, #DAIF_DBG_BIT
	ret


func read_daif
	mrs	x0, daif
	ret


func write_daif
	msr	daif, x0
	ret


func read_spsr_el1
	mrs	x0, spsr_el1
	ret


func read_spsr_el2
	mrs	x0, spsr_el2
	ret


func read_spsr_el3
	mrs	x0, spsr_el3
	ret


func write_spsr_el1
	msr	spsr_el1, x0
	ret


func write_spsr_el2
	msr	spsr_el2, x0
	ret


func write_spsr_el3
	msr	spsr_el3, x0
	ret


func read_elr_el1
	mrs	x0, elr_el1
	ret


func read_elr_el2
	mrs	x0, elr_el2
	ret


func read_elr_el3
	mrs	x0, elr_el3
	ret


func write_elr_el1
	msr	elr_el1, x0
	ret


func write_elr_el2
	msr	elr_el2, x0
	ret


func write_elr_el3
	msr	elr_el3, x0
	ret


func dsb
	dsb	sy
	ret


func isb
	isb
	ret


func sev
	sev
	ret


func wfe
	wfe
	ret


func wfi
	wfi
	ret


func eret
	eret


func smc
	smc	#0

/* -----------------------------------------------------------------------
 * void zeromem16(void *mem, unsigned int length);
 *
 * Initialise a memory region to 0.
 * The memory address must be 16-byte aligned.
 * -----------------------------------------------------------------------
 */
func zeromem16
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
z_end:	ret


/* --------------------------------------------------------------------------
 * void memcpy16(void *dest, const void *src, unsigned int length)
 *
 * Copy length bytes from memory area src to memory area dest.
 * The memory areas should not overlap.
 * Destination and source addresses must be 16-byte aligned.
 * --------------------------------------------------------------------------
 */
func memcpy16
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
m_end:	ret

/* ---------------------------------------------------------------------------
 * Disable the MMU at EL3
 * This is implemented in assembler to ensure that the data cache is cleaned
 * and invalidated after the MMU is disabled without any intervening cacheable
 * data accesses
 * ---------------------------------------------------------------------------
 */

func disable_mmu_el3
	mov	x1, #(SCTLR_M_BIT | SCTLR_C_BIT)
do_disable_mmu:
	mrs	x0, sctlr_el3
	bic	x0, x0, x1
	msr	sctlr_el3, x0
	isb				// ensure MMU is off
	mov	x0, #DCCISW		// DCache clean and invalidate
	b	dcsw_op_all


func disable_mmu_icache_el3
	mov	x1, #(SCTLR_M_BIT | SCTLR_C_BIT | SCTLR_I_BIT)
	b	do_disable_mmu

