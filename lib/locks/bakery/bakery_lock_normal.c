/*
 * Copyright (c) 2015-2020, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2020, NVIDIA Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <string.h>

#include <arch_helpers.h>
#include <lib/bakery_lock.h>
#include <lib/el3_runtime/cpu_data.h>
#include <lib/utils_def.h>
#include <plat/common/platform.h>

/*
 * Functions in this file implement Bakery Algorithm for mutual exclusion with the
 * bakery lock data structures in cacheable and Normal memory.
 *
 * ARM architecture offers a family of exclusive access instructions to
 * efficiently implement mutual exclusion with hardware support. However, as
 * well as depending on external hardware, these instructions have defined
 * behavior only on certain memory types (cacheable and Normal memory in
 * particular; see ARMv8 Architecture Reference Manual section B2.10). Use cases
 * in trusted firmware are such that mutual exclusion implementation cannot
 * expect that accesses to the lock have the specific type required by the
 * architecture for these primitives to function (for example, not all
 * contenders may have address translation enabled).
 *
 * This implementation does not use mutual exclusion primitives. It expects
 * memory regions where the locks reside to be cacheable and Normal.
 *
 * Note that the ARM architecture guarantees single-copy atomicity for aligned
 * accesses regardless of status of address translation.
 */

#ifdef PLAT_PERCPU_BAKERY_LOCK_SIZE
/*
 * Verify that the platform defined value for the per-cpu space for bakery locks is
 * a multiple of the cache line size, to prevent multiple CPUs writing to the same
 * bakery lock cache line
 *
 * Using this value, if provided, rather than the linker generated value results in
 * more efficient code
 */
CASSERT((PLAT_PERCPU_BAKERY_LOCK_SIZE & (CACHE_WRITEBACK_GRANULE - 1)) == 0, \
	PLAT_PERCPU_BAKERY_LOCK_SIZE_not_cacheline_multiple);
#define PERCPU_BAKERY_LOCK_SIZE (PLAT_PERCPU_BAKERY_LOCK_SIZE)
#else
/*
 * Use the linker defined symbol which has evaluated the size reqiurement.
 * This is not as efficient as using a platform defined constant
 */
IMPORT_SYM(uintptr_t, __PERCPU_BAKERY_LOCK_START__, BAKERY_LOCK_START);
IMPORT_SYM(uintptr_t, __PERCPU_BAKERY_LOCK_END__, BAKERY_LOCK_END);
#define PERCPU_BAKERY_LOCK_SIZE (BAKERY_LOCK_END - BAKERY_LOCK_START)
#endif

static inline bakery_lock_t *get_bakery_info(unsigned int cpu_ix,
					     bakery_lock_t *lock)
{
	return (bakery_info_t *)((uintptr_t)lock +
				cpu_ix * PERCPU_BAKERY_LOCK_SIZE);
}

static inline void write_cache_op(uintptr_t addr, bool cached)
{
	if (cached)
		dccvac(addr);
	else
		dcivac(addr);

	dsbish();
}

static inline void read_cache_op(uintptr_t addr, bool cached)
{
	if (cached)
		dccivac(addr);

	dmbish();
}

/* Helper function to check if the lock is acquired */
static inline bool is_lock_acquired(const bakery_info_t *my_bakery_info,
				    bool is_cached)
{
	/*
	 * Even though lock data is updated only by the owning cpu and
	 * appropriate cache maintenance operations are performed,
	 * if the previous update was done when the cpu was not participating
	 * in coherency, then there is a chance that cache maintenance
	 * operations were not propagated to all the caches in the system.
	 * Hence do a `read_cache_op()` prior to read.
	 */
	read_cache_op((uintptr_t)my_bakery_info, is_cached);
	return bakery_ticket_number(my_bakery_info->lock_data) != 0U;
}

static unsigned int bakery_get_ticket(bakery_lock_t *lock,
				      unsigned int me, bool is_cached)
{
	unsigned int my_ticket, their_ticket;
	unsigned int they;
	bakery_info_t *my_bakery_info, *their_bakery_info;

	/*
	 * Obtain a reference to the bakery information for this cpu and ensure
	 * it is not NULL.
	 */
	my_bakery_info = get_bakery_info(me, lock);
	assert(my_bakery_info != NULL);

	/* Prevent recursive acquisition.*/
	assert(!is_lock_acquired(my_bakery_info, is_cached));

	/*
	 * Tell other contenders that we are through the bakery doorway i.e.
	 * going to allocate a ticket for this cpu.
	 */
	my_ticket = 0U;
	my_bakery_info->lock_data = make_bakery_data(CHOOSING_TICKET, my_ticket);

	write_cache_op((uintptr_t)my_bakery_info, is_cached);

	/*
	 * Iterate through the bakery information of each contender to allocate
	 * the highest ticket number for this cpu.
	 */
	for (they = 0U; they < BAKERY_LOCK_MAX_CPUS; they++) {
		if (me == they)
			continue;

		/*
		 * Get a reference to the other contender's bakery info and
		 * ensure that a stale copy is not read.
		 */
		their_bakery_info = get_bakery_info(they, lock);
		assert(their_bakery_info != NULL);

		read_cache_op((uintptr_t)their_bakery_info, is_cached);

		/*
		 * Update this cpu's ticket number if a higher ticket number is
		 * seen
		 */
		their_ticket = bakery_ticket_number(their_bakery_info->lock_data);
		if (their_ticket > my_ticket)
			my_ticket = their_ticket;
	}

	/*
	 * Compute ticket; then signal to other contenders waiting for us to
	 * finish calculating our ticket value that we're done
	 */
	++my_ticket;
	my_bakery_info->lock_data = make_bakery_data(CHOSEN_TICKET, my_ticket);

	write_cache_op((uintptr_t)my_bakery_info, is_cached);

	return my_ticket;
}

void bakery_lock_get(bakery_lock_t *lock)
{
	unsigned int they, me;
	unsigned int my_ticket, my_prio, their_ticket;
	bakery_info_t *their_bakery_info;
	unsigned int their_bakery_data;
	bool is_cached;

	me = plat_my_core_pos();
	is_cached = is_dcache_enabled();

	/* Get a ticket */
	my_ticket = bakery_get_ticket(lock, me, is_cached);

	/*
	 * Now that we got our ticket, compute our priority value, then compare
	 * with that of others, and proceed to acquire the lock
	 */
	my_prio = bakery_get_priority(my_ticket, me);
	for (they = 0U; they < BAKERY_LOCK_MAX_CPUS; they++) {
		if (me == they)
			continue;

		/*
		 * Get a reference to the other contender's bakery info and
		 * ensure that a stale copy is not read.
		 */
		their_bakery_info = get_bakery_info(they, lock);
		assert(their_bakery_info != NULL);

		/* Wait for the contender to get their ticket */
		do {
			read_cache_op((uintptr_t)their_bakery_info, is_cached);
			their_bakery_data = their_bakery_info->lock_data;
		} while (bakery_is_choosing(their_bakery_data));

		/*
		 * If the other party is a contender, they'll have non-zero
		 * (valid) ticket value. If they do, compare priorities
		 */
		their_ticket = bakery_ticket_number(their_bakery_data);
		if (their_ticket && (bakery_get_priority(their_ticket, they) < my_prio)) {
			/*
			 * They have higher priority (lower value). Wait for
			 * their ticket value to change (either release the lock
			 * to have it dropped to 0; or drop and probably content
			 * again for the same lock to have an even higher value)
			 */
			do {
				wfe();
				read_cache_op((uintptr_t)their_bakery_info, is_cached);
			} while (their_ticket
				== bakery_ticket_number(their_bakery_info->lock_data));
		}
	}

	/*
	 * Lock acquired. Ensure that any reads and writes from a shared
	 * resource in the critical section read/write values after the lock is
	 * acquired.
	 */
	dmbish();
}

void bakery_lock_release(bakery_lock_t *lock)
{
	bakery_info_t *my_bakery_info;
	bool is_cached = is_dcache_enabled();

	my_bakery_info = get_bakery_info(plat_my_core_pos(), lock);

	assert(is_lock_acquired(my_bakery_info, is_cached));

	/*
	 * Ensure that other observers see any stores in the critical section
	 * before releasing the lock. Also ensure all loads in the critical
	 * section are complete before releasing the lock. Release the lock by
	 * resetting ticket. Then signal other waiting contenders.
	 */
	dmbish();
	my_bakery_info->lock_data = 0U;
	write_cache_op((uintptr_t)my_bakery_info, is_cached);

	/* This sev is ordered by the dsbish in write_cahce_op */
	sev();
}
