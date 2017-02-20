/*
 * Copyright (c) 2013-2017, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <asm_macros.S>

	.globl	spin_lock
	.globl	spin_unlock

#if (ARM_ARCH_MAJOR > 8) || ((ARM_ARCH_MAJOR == 8) && (ARM_ARCH_MINOR >= 1))

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

	.arch	armv8.1-a

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

	.arch	armv8-a

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
