/*
 * Copyright (c) 2016-2025, Arm Limited and Contributors. All rights reserved.
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

/*
 * Function: spin_lock
 * -------------------
 * Acquires a spinlock by continuously attempting to set it.
 * Will block (spin) until the lock is successfully acquired.
 *
 * Arguments:
 *   r0 - Pointer to the spinlock variable (uint32_t *lock)
 *
 * Return:
 *   None
 *
 * Description:
 *   Blocks until the lock is acquired using LDREX/STREX with WFE for wait.
 */
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

/*
 * Function: spin_trylock
 * ----------------------
 * Attempts to acquire the spinlock once without blocking.
 *
 * Arguments:
 *   r0 - Pointer to the spinlock variable (uint32_t *lock)
 *
 * Return:
 *   r0 - 1 if lock was successfully acquired
 *        0 if lock was already held
 *
 * Description:
 *   Tries once to acquire the lock using LDREX/STREX.
 */
func spin_trylock
        mov     r2, #1
        ldrex   r1, [r0]
        cmp     r1, #0
        strexeq r1, r2, [r0]
        cmpeq   r1, #0
        dmb
        moveq   r0, #1
        movne   r0, #0
        bx      lr
endfunc spin_trylock

/*
 * Function: spin_unlock
 * ---------------------
 * Releases the spinlock by clearing its value.
 *
 * Arguments:
 *   r0 - Pointer to the spinlock variable (uint32_t *lock)
 *
 * Return:
 *   None
 *
 * Description:
 *   Releases the lock using store-release and sends SEV.
 */
func spin_unlock
	mov	r1, #0
	stl	r1, [r0]
	COND_SEV()
	bx	lr
endfunc spin_unlock
