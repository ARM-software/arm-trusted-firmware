/*
 * Copyright (c) 2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_features.h>
#include <lib/spinlock.h>

/*
 * Performs a compare-and-swap of 0 -> 1. If the lock is already held, uses
 * LDAXR/WFE to efficiently wait.
 */
static void spin_lock_atomic(volatile uint32_t *dst)
{
	uint32_t src = 1;
	uint32_t tmp;

	__asm__ volatile (
	".arch_extension lse\n"
	"1:	mov	%w[tmp], wzr\n"
	"2:	casa	%w[tmp], %w[src], [%[dst]]\n"
	"	cbz	%w[tmp], 3f\n"
	"	ldxr	%w[tmp], [%[dst]]\n"
	"	cbz	%w[tmp], 2b\n"
	"	wfe\n"
	"	b	1b\n"
	"3:\n"
	: "+m" (*dst), [tmp] "=&r" (tmp), [src] "+r" (src)
	: [dst] "r" (dst));
}

/*
 * Uses the load-acquire (LDAXR) and store-exclusive (STXR) instruction pair.
 */
static void spin_lock_excl(volatile uint32_t *dst)
{
	uint32_t src = 1;
	uint32_t tmp;

	__asm__ volatile (
	"	sevl\n"
	"1:	wfe\n"
	"2:	ldaxr	%w[tmp], [%[dst]]\n"
	"	cbnz	%w[tmp], 1b\n"
	"	stxr	%w[tmp], %w[src], [%[dst]]\n"
	"	cbnz	%w[tmp], 2b\n"
	: "+m" (*dst), [tmp] "=&r" (tmp), [src] "+r" (src)
	: [dst] "r" (dst));
}

void spin_lock(spinlock_t *lock)
{
	volatile uint32_t *dst = &(lock->lock);

	if (is_feat_lse_supported()) {
		spin_lock_atomic(dst);
	} else {
		spin_lock_excl(dst);
	}
}

/*
 * Use store-release to unconditionally clear the spinlock variable. Store
 * operation generates an event to all cores waiting in WFE when address is
 * monitored by the global monitor.
 */
void spin_unlock(spinlock_t *lock)
{
	volatile uint32_t *dst = &(lock->lock);

	__asm__ volatile (
	"stlr	wzr, [%[dst]]"
	: "=m" (dst)
	: [dst] "r" (dst));
}

static bool spin_trylock_atomic(volatile uint32_t *dst)
{
	uint32_t src = 1;
	uint32_t tmp = 0;
	bool out;

	__asm__ volatile (
	".arch_extension lse\n"
	"casa	%w[tmp], %w[src], [%[dst]]\n"
	"eor	%w[out], %w[tmp], #1\n" /* convert the result to bool */
	: "+m" (*dst), [tmp] "+r" (tmp), [out] "=r" (out)
	: [src] "r" (src), [dst] "r" (dst));

	return out;
}

static bool spin_trylock_excl(volatile uint32_t *dst)
{
	uint32_t src = 1;
	uint32_t tmp;
	bool out;

	__asm__ volatile (
	"ldaxr	%w[tmp], [%[dst]]\n"
	"cbnz	%w[tmp], 1f\n"
	"stxr	%w[tmp], %w[src], [%[dst]]\n"
	"cbnz	%w[tmp], 1f\n"
	"mov	%w[out], #1\n"
	"1:\n" /* fail */
	"mov	%w[out], #0\n"
	: "+m" (*dst), [tmp] "=&r" (tmp), [out] "=r" (out)
	:  [src] "r" (src), [dst] "r" (dst));

	return out;
}

/*
 * Attempts to acquire the spinlock once without spinning. If unlocked (0),
 * attempts to store 1 to acquire it.
 */
bool spin_trylock(spinlock_t *lock)
{
	volatile uint32_t *dst = &(lock->lock);

	if (is_feat_lse_supported()) {
		return spin_trylock_atomic(dst);
	} else {
		return spin_trylock_excl(dst);
	}
}

#if USE_SPINLOCK_CAS
/*
 * Acquire bitlock using atomic bit set on byte. If the original read value
 * has the bit set, use load exclusive semantics to monitor the address and
 * enter WFE.
 */
void bit_lock(bitlock_t *lock, uint8_t mask)
{
	volatile uint8_t *dst = &(lock->lock);
	uint32_t tmp;

	/* there is no exclusive fallback */
	assert(is_feat_lse_supported());

	__asm__ volatile (
	"1:	ldsetab	%w[mask], %w[tmp], [%[dst]]\n"
	"	tst	%w[tmp], %w[mask]\n"
	"	b.eq	2f\n"
	"	ldxrb	%w[tmp], [%[dst]]\n"
	"	tst	%w[tmp], %w[mask]\n"
	"	b.eq	1b\n"
	"	wfe\n"
	"	b	1b\n"
	"2:\n"
	: "+m" (*dst), [tmp] "=&r" (tmp)
	: [mask] "r" (mask), [dst] "r" (dst));
}

/*
 * Use atomic bit clear store-release to unconditionally clear bitlock variable.
 * Store operation generates an event to all cores waiting in WFE when address
 * is monitored by the global monitor.
 */
void bit_unlock(bitlock_t *lock, uint8_t mask)
{
	volatile uint8_t *dst = &(lock->lock);

	/* there is no exclusive fallback */
	assert(is_feat_lse_supported());

	__asm__ volatile (
	"stclrlb	%w[mask], [%[dst]]"
	: "=m" (dst)
	: [mask] "r" (mask), [dst] "r" (dst));
}
#endif
