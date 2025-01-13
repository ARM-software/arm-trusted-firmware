/*
 * Copyright (c) 2024, Altera Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <arch.h>
#include <asm_macros.S>
#include <cpu_macros.S>
#include <plat_macros.S>

	.globl invalidate_dcache_all
	.globl invalidate_cache_low_el
	/* --------------------------------------------------------
	 * Invalidate for NS EL2 and EL1
	 * --------------------------------------------------------
	 */
func invalidate_cache_low_el
	mrs	x0, SCR_EL3
	orr	x1, x0, #SCR_NS_BIT
	msr	SCR_EL3, x1
	isb
	tlbi	ALLE2
	dsb	sy
	tlbi	ALLE1
	dsb	sy
	msr	SCR_EL3, x0
	isb
endfunc invalidate_cache_low_el

.pushsection .text.asm_dcache_level, "ax"
func asm_dcache_level
	lsl	x12, x0, #1
	msr	csselr_el1, x12		/* select cache level */
	isb				/* sync change of cssidr_el1 */
	mrs	x6, ccsidr_el1		/* read the new cssidr_el1 */
	ubfx	x2, x6,  #0,  #3	/* x2 <- log2(cache line size)-4 */
	ubfx	x3, x6,  #3, #10	/* x3 <- number of cache ways - 1 */
	ubfx	x4, x6, #13, #15	/* x4 <- number of cache sets - 1 */
	add	x2, x2, #4		/* x2 <- log2(cache line size) */
	clz	w5, w3			/* bit position of #ways */
	/* x12 <- cache level << 1 */
	/* x2 <- line length offset */
	/* x3 <- number of cache ways - 1 */
	/* x4 <- number of cache sets - 1 */
	/* x5 <- bit position of #ways */

loop_set:
	mov	x6, x3			/* x6 <- working copy of #ways */
loop_way:
	lsl	x7, x6, x5
	orr	x9, x12, x7		/* map way and level to cisw value */
	lsl	x7, x4, x2
	orr	x9, x9, x7		/* map set number to cisw value */
	tbz	w1, #0, 1f
	dc	isw, x9
	b	2f
1:	dc	cisw, x9		/* clean & invalidate by set/way */
2:	subs	x6, x6, #1		/* decrement the way */
	b.ge	loop_way
	subs	x4, x4, #1		/* decrement the set */
	b.ge	loop_set

	ret
endfunc asm_dcache_level
.popsection

/*
 * void __asm_flush_dcache_all(int invalidate_only)
 *
 * x0: 0 clean & invalidate, 1 invalidate only
 *
 * flush or invalidate all data cache by SET/WAY.
 */
.pushsection .text.asm_dcache_all, "ax"
func asm_dcache_all
	mov	x1, x0
	dsb	sy
	mrs	x10, clidr_el1		/* read clidr_el1 */
	ubfx	x11, x10, #24, #3	/* x11 <- loc */
	cbz	x11, finished		/* if loc is 0, exit */
	mov	x15, x30
	mov	x0, #0			/* start flush at cache level 0 */
	/* x0  <- cache level */
	/* x10 <- clidr_el1 */
	/* x11 <- loc */
	/* x15 <- return address */

loop_level:
	add	x12, x0, x0, lsl #1	/* x12 <- tripled cache level */
	lsr	x12, x10, x12
	and	x12, x12, #7		/* x12 <- cache type */
	cmp	x12, #2
	b.lt	skip			/* skip if no cache or icache */
	bl	asm_dcache_level	/* x1 = 0 flush, 1 invalidate */
skip:
	add	x0, x0, #1		/* increment cache level */
	cmp	x11, x0
	b.gt	loop_level

	mov	x0, #0
	msr	csselr_el1, x0		/* restore csselr_el1 */
	dsb	sy
	isb
	mov	x30, x15

finished:
	ret
endfunc asm_dcache_all
.popsection

.pushsection .text.invalidate_dcache_all, "ax"
func invalidate_dcache_all
	mov	x0, #0x1
	b	asm_dcache_all
endfunc invalidate_dcache_all
.popsection
