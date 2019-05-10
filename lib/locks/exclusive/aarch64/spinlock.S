/*
 * Copyright (c) 2013-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <asm_macros.S>

	.globl	spin_lock
	.globl	spin_unlock

#if ARM_ARCH_AT_LEAST(8, 1)

/*
 * When compiled for ARMv8.1 or later, choose spin locks based on Compare and
 * Swap instruction.
 */
# define USE_CAS	1

/*
 * Lock contenders using CAS, upon failing to acquire the lock, wait with the
 * monitor in open state. Therefore, a normal store upon unlocking won't
 * generate an SEV. Use explicit SEV instruction with CAS unlock.
 */
# define COND_SEV()	sev

#else

# define USE_CAS	0

/*
 * Lock contenders using exclusive pairs, upon failing to acquire the lock, wait
 * with the monitor in exclusive state. A normal store upon unlocking will
 * implicitly generate an envent; so, no explicit SEV with unlock is required.
 */
# define COND_SEV()

#endif

#if USE_CAS

/*
 * Acquire lock using Compare and Swap instruction.
 *
 * Compare for 0 with acquire semantics, and swap 1. Wait until CAS returns
 * 0.
 *
 * void spin_lock(spinlock_t *lock);
 */
func spin_lock
	mov	w2, #1
	sevl
1:
	wfe
	mov	w1, wzr
	casa	w1, w2, [x0]
	cbnz	w1, 1b
	ret
endfunc spin_lock

#else /* !USE_CAS */

/*
 * Acquire lock using load-/store-exclusive instruction pair.
 *
 * void spin_lock(spinlock_t *lock);
 */
func spin_lock
	mov	w2, #1
	sevl
l1:	wfe
l2:	ldaxr	w1, [x0]
	cbnz	w1, l1
	stxr	w1, w2, [x0]
	cbnz	w1, l2
	ret
endfunc spin_lock

#endif /* USE_CAS */

/*
 * Release lock previously acquired by spin_lock.
 *
 * Unconditionally write 0, and conditionally generate an event.
 *
 * void spin_unlock(spinlock_t *lock);
 */
func spin_unlock
	stlr	wzr, [x0]
	COND_SEV()
	ret
endfunc spin_unlock
