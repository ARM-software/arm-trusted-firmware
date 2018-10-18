/*
 * Copyright (c) 2018, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include "rcar_def.h"

.global rcar_pwrc_switch_stack
.global rcar_pwrc_save_generic_timer
.global rcar_pwrc_restore_generic_timer

#define OFFSET_SP_X9_X10	(0x00)
#define OFFSET_CNTFID0		(0x10)
#define OFFSET_CNTPCT_EL0	(0x18)
#define OFFSET_TIMER_COUNT	(0x20)

/*
 * x0 : jump address,
 * x1 : stack address,
 * x2 : arg,
 * x3 : stack address (temporary)
 */
func rcar_pwrc_switch_stack

	/* lr to stack */
	stp	x29, x30, [sp,#-16]

	/* change stack pointer */
	mov	x3, sp
	mov	sp, x1

	/* save stack pointer */
	sub	sp, sp, #16
	stp	x0, x3, [sp]

	/* data synchronization barrier */
	dsb	sy

	/* jump to code */
	mov	x1, x0
	mov	x0, x2
	blr	x1

	/* load stack pointer */
	ldp 	x0, x2, [sp,#0]

	/* change stack pointer */
	mov	sp, x2

	/* return */
	ldp	x29, x30, [sp,#-16]
	ret
endfunc rcar_pwrc_switch_stack

/* x0 : stack pointer base address */
func rcar_pwrc_save_generic_timer

	stp	x9, x10, [x0, #OFFSET_SP_X9_X10]

	/* save CNTFID0 and cntpct_el0 */
	mov_imm	x10, (RCAR_CNTC_BASE + CNTFID_OFF)
	ldr	x9, [x10]
	mrs	x10, cntpct_el0
	stp	x9, x10, [x0, #OFFSET_CNTFID0]

	ldp	x9, x10, [x0, #OFFSET_SP_X9_X10]

	ret
endfunc rcar_pwrc_save_generic_timer

/* x0 : Stack pointer base address */
func rcar_pwrc_restore_generic_timer

	stp	x9, x10, [x0, #OFFSET_SP_X9_X10]

	/* restore CNTFID0 and cntpct_el0 */
	ldr	x10, [x0, #OFFSET_CNTFID0]
	mov_imm	x9, (RCAR_CNTC_BASE + CNTFID_OFF)
	str	x10, [x9]
	ldp	x9, x10, [x0, #OFFSET_CNTPCT_EL0]
	add	x9, x9, x10
	str	x9, [x0, #OFFSET_TIMER_COUNT]

	ldp	x9, x10, [x0, #OFFSET_SP_X9_X10]

	ret
endfunc rcar_pwrc_restore_generic_timer
