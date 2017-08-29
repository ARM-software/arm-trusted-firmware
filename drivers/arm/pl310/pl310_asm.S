/*
 * Copyright (c) 2014-2016, STMicroelectronics International N.V.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <asm_macros.S>
#include <pl310.h>

	.globl	pl310_clean_by_way
	.globl	pl310_invalidate_by_way
	.globl	pl310_clean_invalidate_by_way

/*
 * Set sync operation mask according to ways associativity.
 * Preserve r0 = pl310 iomem base address
 */
.macro syncbyway_set_mask reg
	ldr	\reg, [r0, #PL310_AUX_CTRL]
	tst	\reg, #PL310_AUX_16WAY_BIT
	mov	\reg, #PL310_8WAYS_MASK
	orrne	\reg, \reg, #PL310_16WAYS_UPPERMASK
.endm

/*
 * r0: vaddr_t pl310_base
 * r3: ptr to PL310_CLEAN_BY_WAY, PL310_INV_BY_WAY or PL310_FLUSH_BY_WAY.
 */
func __pl310_maintenace_by_way

	syncbyway_set_mask r1
	ldr	r3, [r3]
	str	r1, [r0, r3]

	/* loop on ways */
1:	ldr	r2, [r0, r3]
	and	r2, r2, r1
	cmp	r2, #0
	bne	1b

	/* wait synchro */
1:	ldr	r1, [r0, #PL310_SYNC]
	cmp	r1, #0
	bne	1b
	mov	r1, #1
	str	r1, [r0, #PL310_SYNC]
1:	ldr	r1, [r0, #PL310_SYNC]
	cmp	r1, #0
	bne	1b

	mov	pc, lr

endfunc __pl310_maintenace_by_way

/*
 * void pl310_clean_by_way(vaddr_t base)
 */
func pl310_clean_by_way

	ldr	r3, =1f
	b	__pl310_maintenace_by_way
1:	.word PL310_CLEAN_BY_WAY

endfunc pl310_clean_by_way

/*
 * void pl310_invalidate_by_way(vaddr_t base)
 */
func pl310_invalidate_by_way

	ldr	r3, =1f
	b	__pl310_maintenace_by_way
1:	.word PL310_INV_BY_WAY

endfunc pl310_invalidate_by_way

/*
 * void pl310_clean_invalidate_by_way(vaddr_t base)
 */
func pl310_clean_invalidate_by_way

	ldr	r3, =1f
	b	__pl310_maintenace_by_way
1:	.word PL310_FLUSH_BY_WAY

endfunc pl310_clean_invalidate_by_way
