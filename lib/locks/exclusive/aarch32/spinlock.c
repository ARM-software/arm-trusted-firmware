/*
 * Copyright (c) 2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/spinlock.h>

void __attribute__((target("arm"))) spin_lock(spinlock_t *lock)
{
	volatile uint32_t *dst = &(lock->lock);
	uint32_t src = 1;
	uint32_t tmp;

	__asm__ volatile (
	"1:\n"
	"	ldrex		%[tmp], [%[dst]]\n"
	"	cmp		%[tmp], #0\n"
	"	wfene\n"
	"	strexeq		%[tmp], %[src], [%[dst]]\n"
	"	cmpeq		%[tmp], #0\n"
	"	bne		1b\n"
	"	dmb\n"
	: "+m" (*dst), [tmp] "=&r" (tmp), [src] "+r" (src)
	: [dst] "r" (dst));
}

void __attribute__((target("arm"))) spin_unlock(spinlock_t *lock)
{
	volatile uint32_t *dst = &(lock->lock);
	uint32_t val = 0;

	/*
	 * According to the ARMv8-A Architecture Reference Manual, "when the
	 * global monitor for a PE changes from Exclusive Access state to Open
	 * Access state, an event is generated.". This applies to both AArch32
	 * and AArch64 modes of ARMv8-A. As a result, no explicit SEV with
	 * unlock is required.
	 */
	__asm__ volatile (
/* ARMv7 does not support stl instruction */
#if ARM_ARCH_MAJOR == 7
	"dmb\n"
	"str	%[val], [%[dst]]\n"
	"dsb\n"
	"sev\n"
#else
	"stl	%[val], [%[dst]]\n"
#endif
	: "=m" (dst)
	: [val] "r" (val), [dst] "r" (dst));
}

bool __attribute__((target("arm"))) spin_trylock(spinlock_t *lock)
{
	volatile uint32_t *dst = &(lock->lock);
	uint32_t src = 1;
	uint32_t tmp;
	bool out;

	__asm__ volatile (
	"ldrex		%[tmp], [%[dst]]\n"
	"cmp		%[tmp], #0\n"
	"strexeq	%[tmp], %[src], [%[dst]]\n"
	"cmpeq		%[tmp], #0\n"
	"dmb\n"
	"moveq		%[out], #1\n"
	"movne		%[out], #0\n"
	: "+m" (*dst), [tmp] "=&r" (tmp), [out] "=r" (out)
	:  [src] "r" (src), [dst] "r" (dst));

	return out;
}
