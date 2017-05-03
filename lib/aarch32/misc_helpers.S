/*
 * Copyright (c) 2016-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <assert_macros.S>

	.globl	smc
	.globl	zeromem
	.globl	zero_normalmem
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
 * void zeromem(void *mem, unsigned int length)
 *
 * Initialise a region in normal memory to 0. This functions complies with the
 * AAPCS and can be called from C code.
 *
 * -----------------------------------------------------------------------
 */
func zeromem
	/*
	 * Readable names for registers
	 *
	 * Registers r0, r1 and r2 are also set by zeromem which
	 * branches into the fallback path directly, so cursor, length and
	 * stop_address should not be retargeted to other registers.
	 */
	cursor       .req r0 /* Start address and then current address */
	length       .req r1 /* Length in bytes of the region to zero out */
	/*
	 * Reusing the r1 register as length is only used at the beginning of
	 * the function.
	 */
	stop_address .req r1  /* Address past the last zeroed byte */
	zeroreg1     .req r2  /* Source register filled with 0 */
	zeroreg2     .req r3  /* Source register filled with 0 */
	tmp	     .req r12 /* Temporary scratch register */

	mov	zeroreg1, #0

	/* stop_address is the address past the last to zero */
	add	stop_address, cursor, length

	/*
	 * Length cannot be used anymore as it shares the same register with
	 * stop_address.
	 */
	.unreq	length

	/*
	 * If the start address is already aligned to 8 bytes, skip this loop.
	 */
	tst	cursor, #(8-1)
	beq	.Lzeromem_8bytes_aligned

	/* Calculate the next address aligned to 8 bytes */
	orr	tmp, cursor, #(8-1)
	adds	tmp, tmp, #1
	/* If it overflows, fallback to byte per byte zeroing */
	beq	.Lzeromem_1byte_aligned
	/* If the next aligned address is after the stop address, fall back */
	cmp	tmp, stop_address
	bhs	.Lzeromem_1byte_aligned

	/* zero byte per byte */
1:
	strb	zeroreg1, [cursor], #1
	cmp	cursor, tmp
	bne	1b

	/* zero 8 bytes at a time */
.Lzeromem_8bytes_aligned:

	/* Calculate the last 8 bytes aligned address. */
	bic	tmp, stop_address, #(8-1)

	cmp	cursor, tmp
	bhs	2f

	mov	zeroreg2, #0
1:
	stmia	cursor!, {zeroreg1, zeroreg2}
	cmp	cursor, tmp
	blo	1b
2:

	/* zero byte per byte */
.Lzeromem_1byte_aligned:
	cmp	cursor, stop_address
	beq	2f
1:
	strb	zeroreg1, [cursor], #1
	cmp	cursor, stop_address
	bne	1b
2:
	bx	lr

	.unreq	cursor
	/*
	 * length is already unreq'ed to reuse the register for another
	 * variable.
	 */
	.unreq	stop_address
	.unreq	zeroreg1
	.unreq	zeroreg2
	.unreq	tmp
endfunc zeromem

/*
 * AArch32 does not have special ways of zeroing normal memory as AArch64 does
 * using the DC ZVA instruction, so we just alias zero_normalmem to zeromem.
 */
.equ	zero_normalmem, zeromem

/* --------------------------------------------------------------------------
 * void memcpy4(void *dest, const void *src, unsigned int length)
 *
 * Copy length bytes from memory area src to memory area dest.
 * The memory areas should not overlap.
 * Destination and source addresses must be 4-byte aligned.
 * --------------------------------------------------------------------------
 */
func memcpy4
#if ENABLE_ASSERTIONS
	orr	r3, r0, r1
	tst	r3, #0x3
	ASM_ASSERT(eq)
#endif
/* copy 4 bytes at a time */
m_loop4:
	cmp	r2, #4
	blo	m_loop1
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
