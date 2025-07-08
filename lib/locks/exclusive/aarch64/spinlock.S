/*
 * Copyright (c) 2013-2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <asm_macros.S>

	.globl	spin_lock
	.globl	spin_unlock
	.globl	bit_lock
	.globl	bit_unlock

#if USE_SPINLOCK_CAS
#if !ARM_ARCH_AT_LEAST(8, 1)
#error USE_SPINLOCK_CAS option requires at least an ARMv8.1 platform
#endif

/*
 * When compiled for ARMv8.1 or later, choose spin locks based on Compare and
 * Swap instruction.
 */

/*
 * Function: spin_lock
 * -------------------
 * Acquires a spinlock using the Compare-And-Swap (CASA) instruction.
 * Spins until the lock is successfully acquired.
 *
 * Arguments:
 *   x0 - Pointer to the spinlock variable (spinlock_t *lock)
 *
 * Return:
 *   None
 *
 * Description:
 *   - Attempts to acquire the lock by performing a compare-and-swap of 0 -> 1.
 *   - If the lock is already held, uses LDAXR/WFE to efficiently wait.
 *   - Loops until the lock is acquired.
 */

func spin_lock
	mov	w2, #1
1:	mov	w1, wzr
2:	casa	w1, w2, [x0]
	cbz	w1, 3f
	ldxr	w1, [x0]
	cbz	w1, 2b
	wfe
	b	1b
3:
	ret
endfunc spin_lock

/*
 * Function: spin_trylock
 * ----------------------
 * Attempts to acquire the spinlock using the CASA instruction without spinning.
 *
 * Arguments:
 *   x0 - Pointer to the spinlock variable (spinlock_t *lock)
 *
 * Return:
 *   w0 - 1 if lock was successfully acquired
 *        0 if lock was already held
 *
 * Description:
 *   - Performs a single compare-and-swap operation.
 *   - If the lock is already held, returns failure immediately.
 */
func spin_trylock
        mov     w1, wzr
        mov     w2, #1
        casa    w1, w2, [x0]
        eor     w0, w1, #1
        ret
endfunc spin_trylock

#else /* !USE_SPINLOCK_CAS */

/*
 * Function: spin_lock
 * -------------------
 * Acquires a spinlock using the load-acquire (LDAXR) and store-exclusive
 * (STXR) instruction pair.Spins until the lock is acquired.
 *
 * Arguments:
 *   x0 - Pointer to the spinlock variable (spinlock_t *lock)
 *
 * Return:
 *   None
 *
 * Description:
 *   - Waits for the lock to be released using WFE.
 *   - Attempts to acquire it by setting the value to 1 using LDAXR/STXR.
 *   - Uses SEVL/WFE to reduce power while waiting.
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

/*
 * Function: spin_trylock
 * ----------------------
 * Attempts to acquire the spinlock once using LDAXR/STXR without spinning.
 *
 * Arguments:
 *   x0 - Pointer to the spinlock variable (spinlock_t *lock)
 *
 * Return:
 *   w0 - 1 if lock was successfully acquired
 *        0 if lock was already held
 *
 * Description:
 *   - Loads the lock value.
 *   - If unlocked (0), attempts to store 1 to acquire it.
 *   - Returns success or failure based on the outcome.
 */
func spin_trylock
        mov     w2, #1
        ldaxr   w1, [x0]
        cbnz    w1, fail
        stxr    w1, w2, [x0]
        cbnz    w1, fail
        mov     w0, #1
        ret
fail:
        mov     w0, #0
        ret
endfunc spin_trylock

#endif /* USE_SPINLOCK_CAS */

/*
 * Function: spin_unlock
 * ---------------------
 * Releases the spinlock previously acquired by spin_lock or spin_trylock.
 *
 * Arguments:
 *   x0 - Pointer to the spinlock variable (spinlock_t *lock)
 *
 * Return:
 *   None
 *
 * Description:
 *     Use store-release to unconditionally clear the spinlock variable.
 *     Store operation generates an event to all cores waiting in WFE
 *     when address is monitored by the global monitor
 */
func spin_unlock
	stlr	wzr, [x0]
	ret
endfunc spin_unlock

/*
 * Atomic bit clear and set instructions require FEAT_LSE which is
 * mandatory from Armv8.1.
 */
#if ARM_ARCH_AT_LEAST(8, 1)

/*
 * Acquire bitlock using atomic bit set on byte. If the original read value
 * has the bit set, use load exclusive semantics to monitor the address and
 * enter WFE.
 *
 * void bit_lock(bitlock_t *lock, uint8_t mask);
 */
func bit_lock
1:	ldsetab	w1, w2, [x0]
	tst	w2, w1
	b.eq	2f
	ldxrb	w2, [x0]
	tst	w2, w1
	b.eq	1b
	wfe
	b	1b
2:
	ret
endfunc bit_lock

/*
 * Use atomic bit clear store-release to unconditionally clear bitlock variable.
 * Store operation generates an event to all cores waiting in WFE when address
 * is monitored by the global monitor.
 *
 * void bit_unlock(bitlock_t *lock, uint8_t mask);
 */
func bit_unlock
	stclrlb	w1, [x0]
	ret
endfunc bit_unlock

#endif /* ARM_ARCH_AT_LEAST(8, 1) */
