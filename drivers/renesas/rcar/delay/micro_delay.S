/*
 * Copyright (c) 2015-2018, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include "micro_delay.h"

#define CPG_BASE		(0xE6150000)
#define CPG_SMSTPCR1		(0x0134)
#define CPG_CPGWPR		(0x0900)

/* Module bit for TMU ch3-5 */
#define MSTPCR1_TMU1		(1 << 24)

#define TMU3_BASE		(0xE6FC0000)
#define TMU_TSTR		(0x0004)
#define TMU_TCOR		(0x0008)
#define TMU_TCNT		(0x000C)
#define TMU_TCR		(0x0010)
/* Start bit for TMU ch3 */
#define TSTR1_TMU3		(1 << 0)

#define MIDR_CA57		(0x0D07 << MIDR_PN_SHIFT)
#define MIDR_CA53		(0x0D03 << MIDR_PN_SHIFT)

	.globl	rcar_micro_delay
#if (TMU3_MEASUREMENT == 1)
	.globl	tmu3_init
	.globl	tmu3_start
	.globl	tmu3_stop
	.globl	tcnt3_snapshot
#endif
	/* Aligned with the cache line */
	.align	6

func rcar_micro_delay
	cbz	x0, micro_delay_e
	mrs	x1, midr_el1
	and	x1, x1, #MIDR_PN_MASK << MIDR_PN_SHIFT
	mov	w2, #MIDR_CA53
	cmp	w1, w2
	b.eq	micro_delay_ca53
	b	micro_delay_ca57
micro_delay_e:
	ret
endfunc rcar_micro_delay

func micro_delay_ca57
ca57_loop_1:
	mov	x1, #185
ca57_loop_2:
	subs	x1, x1, #1
	b.ne	ca57_loop_2
	subs	x0, x0, #1
	b.ne	ca57_loop_1
	ret
endfunc micro_delay_ca57

func micro_delay_ca53
ca53_loop_1:
	mov	x1, #134
ca53_loop_2:
	subs	x1, x1, #1
	b.ne	ca53_loop_2
	subs	x0, x0, #1
	b.ne	ca53_loop_1
	ret
endfunc micro_delay_ca53

#if (TMU3_MEASUREMENT == 1)
func tmu3_init
	ldr	x2, =CPG_BASE
	ldr	w0, [x2, #CPG_SMSTPCR1]
	ldr	w1, [x2, #CPG_MSTPSR1]
	ldr	w2, #MSTPCR1_TMU1
	bl	mstpcr_write
	ret
endfunc tmu3_init

func tmu3_start
	ldr	x0, =TMU3_BASE
	mov	w1, #0xFFFFFFFF
	str	w1, [x0, TMU_TCNT]

	ldr	x0, =TMU3_BASE
	ldrb	w1, [x0, TMU_TSTR]
	orr	w1, w1, #TSTR1_TMU3
	strb	w1, [x0, TMU_TSTR]
	ret
endfunc tmu3_start

func tcnt3_snapshot
	ldr	x0, =TMU3_BASE
	ldr	w0, [x0, TMU_TCNT]
	ret
endfunc tcnt3_snapshot


func tmu3_stop
	ldr	x0, =TMU3_BASE
	ldrb	w1, [x0, TMU_TSTR]
	and	w1, w1, #~TSTR1_TMU3
	strb	w1, [x0, TMU_TSTR]
	ret
endfunc tmu3_stop
#endif
