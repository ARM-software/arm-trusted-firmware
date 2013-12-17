/*
 * Copyright (c) 2013, ARM Limited and Contributors. All rights reserved.
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

#include <arch_helpers.h>
#include <runtime_svc.h>

	.globl	save_regs
	.globl	restore_regs

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

	.globl	read_spsr
	.globl	read_spsr_el1
	.globl	read_spsr_el2
	.globl	read_spsr_el3

	.globl	write_spsr
	.globl	write_spsr_el1
	.globl	write_spsr_el2
	.globl	write_spsr_el3

	.globl	read_elr
	.globl	read_elr_el1
	.globl	read_elr_el2
	.globl	read_elr_el3

	.globl	write_elr
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

	.section	.text, "ax"

save_regs:; .type save_regs, %function
	sub	sp, sp, #GPREGS_FP_OFF
	stp	x0, x1, [sp, #GPREGS_X0_OFF]
	stp	x2, x3, [sp, #GPREGS_X2_OFF]
	stp	x4, x5, [sp, #GPREGS_X4_OFF]
	stp	x6, x7, [sp, #GPREGS_X6_OFF]
	stp	x8, x9, [sp, #GPREGS_X8_OFF]
	stp	x10, x11, [sp, #GPREGS_X10_OFF]
	stp	x12, x13, [sp, #GPREGS_X12_OFF]
	stp	x14, x15, [sp, #GPREGS_X14_OFF]
	stp	x16, x17, [sp, #GPREGS_X16_OFF]
	stp	x18, x19, [sp, #GPREGS_X18_OFF]
	stp	x20, x21, [sp, #GPREGS_X20_OFF]
	stp	x22, x23, [sp, #GPREGS_X22_OFF]
	stp	x24, x25, [sp, #GPREGS_X24_OFF]
	stp	x26, x27, [sp, #GPREGS_X26_OFF]
	mrs     x0, sp_el0
	stp	x28, x0, [sp, #GPREGS_X28_OFF]
	mrs     x0, spsr_el3
	str	w0, [sp, #GPREGS_SPSR_OFF]
	ret


restore_regs:; .type restore_regs, %function
	ldr	w9, [sp, #GPREGS_SPSR_OFF]
	msr	spsr_el3, x9
	ldp	x28, x9, [sp, #GPREGS_X28_OFF]
	msr	sp_el0, x9
	ldp	x26, x27, [sp, #GPREGS_X26_OFF]
	ldp	x24, x25, [sp, #GPREGS_X24_OFF]
	ldp	x22, x23, [sp, #GPREGS_X22_OFF]
	ldp	x20, x21, [sp, #GPREGS_X20_OFF]
	ldp	x18, x19, [sp, #GPREGS_X18_OFF]
	ldp	x16, x17, [sp, #GPREGS_X16_OFF]
	ldp	x14, x15, [sp, #GPREGS_X14_OFF]
	ldp	x12, x13, [sp, #GPREGS_X12_OFF]
	ldp	x10, x11, [sp, #GPREGS_X10_OFF]
	ldp	x8, x9, [sp, #GPREGS_X8_OFF]
	ldp	x6, x7, [sp, #GPREGS_X6_OFF]
	ldp	x4, x5, [sp, #GPREGS_X4_OFF]
	ldp	x2, x3, [sp, #GPREGS_X2_OFF]
	ldp	x0, x1, [sp, #GPREGS_X0_OFF]
	add	sp, sp, #GPREGS_FP_OFF
	ret

get_afflvl_shift:; .type get_afflvl_shift, %function
	cmp	x0, #3
	cinc	x0, x0, eq
	mov	x1, #MPIDR_AFFLVL_SHIFT
	lsl	x0, x0, x1
	ret

mpidr_mask_lower_afflvls:; .type mpidr_mask_lower_afflvls, %function
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
enable_irq:; .type enable_irq, %function
	msr	daifclr, #DAIF_IRQ_BIT
	ret


enable_fiq:; .type enable_fiq, %function
	msr	daifclr, #DAIF_FIQ_BIT
	ret


enable_serror:; .type enable_serror, %function
	msr	daifclr, #DAIF_ABT_BIT
	ret


enable_debug_exceptions:
	msr	daifclr, #DAIF_DBG_BIT
	ret


disable_irq:; .type disable_irq, %function
	msr	daifset, #DAIF_IRQ_BIT
	ret


disable_fiq:; .type disable_fiq, %function
	msr	daifset, #DAIF_FIQ_BIT
	ret


disable_serror:; .type disable_serror, %function
	msr	daifset, #DAIF_ABT_BIT
	ret


disable_debug_exceptions:
	msr	daifset, #DAIF_DBG_BIT
	ret


read_daif:; .type read_daif, %function
	mrs	x0, daif
	ret


write_daif:; .type write_daif, %function
	msr	daif, x0
	ret


read_spsr:; .type read_spsr, %function
	mrs	x0, CurrentEl
	cmp	x0, #(MODE_EL1 << MODE_EL_SHIFT)
	b.eq	read_spsr_el1
	cmp	x0, #(MODE_EL2 << MODE_EL_SHIFT)
	b.eq	read_spsr_el2
	cmp	x0, #(MODE_EL3 << MODE_EL_SHIFT)
	b.eq	read_spsr_el3


read_spsr_el1:; .type read_spsr_el1, %function
	mrs	x0, spsr_el1
	ret


read_spsr_el2:; .type read_spsr_el2, %function
	mrs	x0, spsr_el2
	ret


read_spsr_el3:; .type read_spsr_el3, %function
	mrs	x0, spsr_el3
	ret


write_spsr:; .type write_spsr, %function
	mrs	x1, CurrentEl
	cmp	x1, #(MODE_EL1 << MODE_EL_SHIFT)
	b.eq	write_spsr_el1
	cmp	x1, #(MODE_EL2 << MODE_EL_SHIFT)
	b.eq	write_spsr_el2
	cmp	x1, #(MODE_EL3 << MODE_EL_SHIFT)
	b.eq	write_spsr_el3


write_spsr_el1:; .type write_spsr_el1, %function
	msr	spsr_el1, x0
	isb
	ret


write_spsr_el2:; .type write_spsr_el2, %function
	msr	spsr_el2, x0
	isb
	ret


write_spsr_el3:; .type write_spsr_el3, %function
	msr	spsr_el3, x0
	isb
	ret


read_elr:; .type read_elr, %function
	mrs	x0, CurrentEl
	cmp	x0, #(MODE_EL1 << MODE_EL_SHIFT)
	b.eq	read_elr_el1
	cmp	x0, #(MODE_EL2 << MODE_EL_SHIFT)
	b.eq	read_elr_el2
	cmp	x0, #(MODE_EL3 << MODE_EL_SHIFT)
	b.eq	read_elr_el3


read_elr_el1:; .type read_elr_el1, %function
	mrs	x0, elr_el1
	ret


read_elr_el2:; .type read_elr_el2, %function
	mrs	x0, elr_el2
	ret


read_elr_el3:; .type read_elr_el3, %function
	mrs	x0, elr_el3
	ret


write_elr:; .type write_elr, %function
	mrs	x1, CurrentEl
	cmp	x1, #(MODE_EL1 << MODE_EL_SHIFT)
	b.eq	write_elr_el1
	cmp	x1, #(MODE_EL2 << MODE_EL_SHIFT)
	b.eq	write_elr_el2
	cmp	x1, #(MODE_EL3 << MODE_EL_SHIFT)
	b.eq	write_elr_el3


write_elr_el1:; .type write_elr_el1, %function
	msr	elr_el1, x0
	isb
	ret


write_elr_el2:; .type write_elr_el2, %function
	msr	elr_el2, x0
	isb
	ret


write_elr_el3:; .type write_elr_el3, %function
	msr	elr_el3, x0
	isb
	ret


dsb:; .type dsb, %function
	dsb	sy
	ret


isb:; .type isb, %function
	isb
	ret


sev:; .type sev, %function
	sev
	ret


wfe:; .type wfe, %function
	wfe
	ret


wfi:; .type wfi, %function
	wfi
	ret


eret:; .type eret, %function
	eret


smc:; .type smc, %function
	smc	#0

/* -----------------------------------------------------------------------
 * void zeromem16(void *mem, unsigned int length);
 *
 * Initialise a memory region to 0.
 * The memory address must be 16-byte aligned.
 * -----------------------------------------------------------------------
 */
zeromem16:
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
memcpy16:
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
