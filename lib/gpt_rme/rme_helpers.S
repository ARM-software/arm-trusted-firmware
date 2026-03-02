/*
 * Copyright (c) 2026, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <asm_macros.S>
#include <lib/gpt_rme/gpc_fault.h>

	.globl	get_el3_gpf_resume_pc

/*
 * Find the safe resume PC for the current EL3 exception if it is a
 * recoverable GPF raised by a GPF-tolerant RME helper.
 *
 * Returns:
 * x0 - Resume PC if the current exception is recoverable, otherwise 0.
 *
 * Clobbers: x1-x4
 */
func get_el3_gpf_resume_pc
	mrs	x1, esr_el3
	ubfx	x2, x1, #ESR_EC_SHIFT, #6
	cmp	x2, #EC_DABORT_CUR_EL
	b.ne	3f

	and	x2, x1, #ISS_DFSC_MASK
	cmp	x2, #DFSC_GPF_DABORT
	b.ne	3f

	mrs	x0, elr_el3

	adrp	x1, __GPC_GPF_TRAPS_START__
	add	x1, x1, :lo12:__GPC_GPF_TRAPS_START__
	adrp	x2, __GPC_GPF_TRAPS_END__
	add	x2, x2, :lo12:__GPC_GPF_TRAPS_END__

	cmp	x1, x2
	b.hs	3f

1:
	ldp	x3, x4, [x1]
	cmp	x0, x3
	b.ne	2f

	mov	x0, x4
	ret

2:
	add	x1, x1, #GPC_GPF_TRAP_SIZE
	cmp	x1, x2
	b.lo	1b

3:
	mov	x0, xzr
	ret
endfunc get_el3_gpf_resume_pc
