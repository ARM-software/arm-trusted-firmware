/*
 * Copyright (c) 2025-2026, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_features.h>
#include <lib/spinlock.h>

/*
 * Performs a compare-and-swap of 0 -> 1. If the lock is already held, uses
 * LDAXR/WFE to efficiently wait.
 */
static uint32_t ldaxr_32(volatile uint32_t *dst)
{
	uint32_t ret;

	__asm__ volatile (
	"ldaxr	%w[ret], %[dst]\n"
	: [ret] "=r" (ret)
	: [dst] "Q" (*dst));

	return ret;
}

static uint32_t stxr_32(uint32_t src, volatile uint32_t *dst)
{
	uint32_t ret;

	__asm__ volatile (
	"stxr	%w[ret], %w[src], %[dst]\n"
	: [dst] "=Q" (*dst), [ret] "=&r" (ret)
	: [src] "r" (src));

	return ret;
}

static uint32_t casa_32(uint32_t src, volatile uint32_t *dst)
{
	uint32_t ret = 0;

	__asm__ volatile (
	".arch_extension lse\n"
	"	casa	%w[ret], %w[src], %[dst]\n"
	".arch_extension nolse\n"
	: [dst] "+Q" (*dst), [ret] "+r" (ret)
	: [src] "r" (src));

	return ret;
}

/*
 * Check that the lock isn't held. Tries to compare-and-swap (CAS) it if not.
 * Uses WFE to efficiently wait.
 */
static void spin_lock_atomic(volatile uint32_t *dst)
{
	for (; 1; wfe()) {
		/* 1 means lock is held */
		if (ldaxr_32(dst) != 0) {
			continue;
		}

		if (casa_32(1, dst) == 0) {
			return;
		}
	}
}

/*
 * Uses the load-acquire (LDAXR) and store-exclusive (STXR) instruction pair.
 */
static void spin_lock_excl(volatile uint32_t *dst)
{
	sevl();
	while (1) {
		wfe();
spinlock_retry:
		if (ldaxr_32(dst) == 0) {
			if (stxr_32(1, dst) == 0) {
				return;
			} else {
				goto spinlock_retry;
			}
		}
	}
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

void spin_unlock(spinlock_t *lock)
{
	volatile uint32_t *dst = &(lock->lock);

	/*
	 * Store operations generate an event to other cores waiting in WFE when
	 * the address is monitored by the global monitor.
	 */
	__asm__ volatile (
	"stlr	wzr, %[dst]"
	: [dst] "=Q" (*dst));

	/* atomics don't generate events so wake others manually */
	if (is_feat_lse_supported()) {
		sev();
	}
}

static bool spin_trylock_atomic(volatile uint32_t *dst)
{
	uint32_t tmp = 0;
	bool out;

	__asm__ volatile (
	".arch_extension lse\n"
	"casa	%w[tmp], %w[src], %[dst]\n"
	"eor	%w[out], %w[tmp], #1\n" /* convert the result to bool */
	".arch_extension nolse\n"
	: [dst] "+Q" (*dst), [tmp] "+r" (tmp), [out] "=r" (out)
	: [src] "r" (1));

	return out;
}

static bool spin_trylock_excl(volatile uint32_t *dst)
{
	/*
	 * Loop until we either get the lock or are certain that we don't have
	 * it. The exclusive store can fail due to racing and not because we
	 * don't hold the lock.
	 */
	while (1) {
		/* 1 means lock is held */
		if (ldaxr_32(dst) != 0) {
			return false;
		}

		if (stxr_32(1, dst) == 0) {
			return true;
		}
	}
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
	".arch_extension lse\n"
	"1:	ldsetab	%w[mask], %w[tmp], %[dst]\n"
	"	tst	%w[tmp], %w[mask]\n"
	"	b.eq	2f\n"
	"	ldxrb	%w[tmp], %[dst]\n"
	"	tst	%w[tmp], %w[mask]\n"
	"	b.eq	1b\n"
	"	wfe\n"
	"	b	1b\n"
	"2:\n"
	".arch_extension nolse\n"
	: [dst] "+Q" (*dst), [tmp] "=&r" (tmp)
	: [mask] "r" (mask));
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
	".arch_extension lse\n"
	"stclrlb	%w[mask], %[dst]\n"
	".arch_extension nolse\n"
	: [dst] "+Q" (*dst)
	: [mask] "r" (mask));
}
#endif
