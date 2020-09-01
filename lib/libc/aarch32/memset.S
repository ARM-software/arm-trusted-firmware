/*
 * Copyright (c) 2020, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <asm_macros.S>

	.syntax unified
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
	mov	r12, r0			/* keep r0 */
	tst	r0, #3
	beq	aligned			/* 4-bytes aligned */

	/* Unaligned 'dst' */
unaligned:
	subs	r2, r2, #1
	strbhs	r1, [r12], #1
	bxls	lr			/* return if 0 */
	tst	r12, #3
	bne	unaligned		/* continue while unaligned */

	/* 4-bytes aligned */
aligned:bfi	r1, r1, #8, #8		/* propagate 'val' */
	bfi	r1, r1, #16, #16

	mov	r3, r1

	cmp	r2, #16
	blo	less_16			/* < 16 */

	push	{r4, lr}
	mov	r4, r1
	mov	lr, r1

write_32:
	subs	r2, r2, #32
	stmiahs	r12!, {r1, r3, r4, lr}
	stmiahs	r12!, {r1, r3, r4, lr}
	bhi	write_32		/* write 32 bytes in a loop */
	popeq	{r4, pc}		/* return if 0 */
	lsls	r2, r2, #28		/* C = r2[4]; N = r2[3]; Z = r2[3:0] */
	stmiacs	r12!, {r1, r3, r4, lr}	/* write 16 bytes */
	popeq	{r4, pc}		/* return if 16 */
	stmiami	r12!, {r1, r3}		/* write 8 bytes */
	lsls	r2, r2, #2		/* C = r2[2]; N = r2[1]; Z = r2[1:0] */
	strcs	r1, [r12], #4		/* write 4 bytes */
	popeq	{r4, pc}		/* return if 8 or 4 */
	strhmi	r1, [r12], #2		/* write 2 bytes */
	lsls	r2, r2, #1		/* N = Z = r2[0] */
	strbmi	r1, [r12]		/* write 1 byte */
	pop	{r4, pc}

less_16:lsls	r2, r2, #29		/* C = r2[3]; N = r2[2]; Z = r2[2:0] */
	stmiacs	r12!, {r1, r3}		/* write 8 bytes */
	bxeq	lr			/* return if 8 */
	strmi	r1, [r12], #4		/* write 4 bytes */
	lsls	r2, r2, #2		/* C = r2[1]; N = Z = r2[0] */
	strhcs	r1, [r12], #2		/* write 2 bytes */
	strbmi	r1, [r12]		/* write 1 byte */
	bx	lr

endfunc memset
