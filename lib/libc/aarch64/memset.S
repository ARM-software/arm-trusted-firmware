/*
 * Copyright (c) 2020, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <asm_macros.S>

	.global	memset

/* -----------------------------------------------------------------------
 * void *memset(void *dst, int val, size_t count)
 *
 * Copy the value of 'val' (converted to an unsigned char) into
 * each of the first 'count' characters of the object pointed to by 'dst'.
 *
 * Returns the value of 'dst'.
 * -----------------------------------------------------------------------
 */
func memset
	cbz	x2, exit		/* exit if 'count' = 0 */
	mov	x3, x0			/* keep x0 */
	tst	x0, #7
	b.eq	aligned			/* 8-bytes aligned */

	/* Unaligned 'dst' */
unaligned:
	strb	w1, [x3], #1
	subs	x2, x2, #1
	b.eq	exit			/* exit if 0 */
	tst	x3, #7
	b.ne	unaligned		/* continue while unaligned */

	/* 8-bytes aligned */
aligned:cbz	x1, x1_zero
	bfi	w1, w1, #8, #8		/* propagate 'val' */
	bfi	w1, w1, #16, #16
	bfi	x1, x1, #32, #32

x1_zero:ands	x4, x2, #~0x3f
	b.eq	less_64

write_64:
	.rept	4
	stp	x1, x1, [x3], #16	/* write 64 bytes in a loop */
	.endr
	subs	x4, x4, #64
	b.ne	write_64
less_64:tbz	w2, #5, less_32		/* < 32 bytes */
	stp	x1, x1, [x3], #16	/* write 32 bytes */
	stp	x1, x1, [x3], #16
less_32:tbz	w2, #4, less_16		/* < 16 bytes */
	stp	x1, x1, [x3], #16	/* write 16 bytes */
less_16:tbz	w2, #3, less_8		/* < 8 bytes */
	str	x1, [x3], #8		/* write 8 bytes */
less_8:	tbz	w2, #2, less_4		/* < 4 bytes */
	str	w1, [x3], #4		/* write 4 bytes */
less_4:	tbz	w2, #1, less_2		/* < 2 bytes */
	strh	w1, [x3], #2		/* write 2 bytes */
less_2:	tbz	w2, #0, exit
	strb	w1, [x3]		/* write 1 byte */
exit:	ret

endfunc	memset
