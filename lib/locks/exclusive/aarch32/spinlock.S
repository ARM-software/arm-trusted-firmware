/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <asm_macros.S>

	.globl	spin_lock
	.globl	spin_unlock

#if ARM_ARCH_AT_LEAST(8, 0)
/*
 * According to the ARMv8-A Architecture Reference Manual, "when the global
 * monitor for a PE changes from Exclusive Access state to Open Access state,
 * an event is generated.". This applies to both AArch32 and AArch64 modes of
 * ARMv8-A. As a result, no explicit SEV with unlock is required.
 */
#define COND_SEV()
#else
#define COND_SEV()	sev
#endif

func spin_lock
	mov	r2, #1
1:
	ldrex	r1, [r0]
	cmp	r1, #0
	wfene
	strexeq	r1, r2, [r0]
	cmpeq	r1, #0
	bne	1b
	dmb
	bx	lr
endfunc spin_lock


func spin_unlock
	mov	r1, #0
	stl	r1, [r0]
	COND_SEV()
	bx	lr
endfunc spin_unlock
