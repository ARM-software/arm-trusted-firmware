/*
 * Copyright (c) 2013-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <string.h>

#include <arch_helpers.h>
#include <lib/bakery_lock.h>
#include <lib/el3_runtime/cpu_data.h>
#include <plat/common/platform.h>

/*
 * Functions in this file implement Bakery Algorithm for mutual exclusion with the
 * bakery lock data structures in coherent memory.
 *
 * ARM architecture offers a family of exclusive access instructions to
 * efficiently implement mutual exclusion with hardware support. However, as
 * well as depending on external hardware, the these instructions have defined
 * behavior only on certain memory types (cacheable and Normal memory in
 * particular; see ARMv8 Architecture Reference Manual section B2.10). Use cases
 * in trusted firmware are such that mutual exclusion implementation cannot
 * expect that accesses to the lock have the specific type required by the
 * architecture for these primitives to function (for example, not all
 * contenders may have address translation enabled).
 *
 * This implementation does not use mutual exclusion primitives. It expects
 * memory regions where the locks reside to be fully ordered and coherent
 * (either by disabling address translation, or by assigning proper attributes
 * when translation is enabled).
 *
 * Note that the ARM architecture guarantees single-copy atomicity for aligned
 * accesses regardless of status of address translation.
 */

#define assert_bakery_entry_valid(_entry, _bakery) do {	\
	assert((_bakery) != NULL);			\
	assert((_entry) < BAKERY_LOCK_MAX_CPUS);	\
} while (false)

/* Obtain a ticket for a given CPU */
static unsigned int bakery_get_ticket(bakery_lock_t *bakery, unsigned int me)
{
	unsigned int my_ticket, their_ticket;
	unsigned int they;

	/* Prevent recursive acquisition */
	assert(bakery_ticket_number(bakery->lock_data[me]) == 0U);

	/*
	 * Flag that we're busy getting our ticket. All CPUs are iterated in the
	 * order of their ordinal position to decide the maximum ticket value
	 * observed so far. Our priority is set to be greater than the maximum
	 * observed priority
	 *
	 * Note that it's possible that more than one contender gets the same
	 * ticket value. That's OK as the lock is acquired based on the priority
	 * value, not the ticket value alone.
	 */
	my_ticket = 0U;
	bakery->lock_data[me] = make_bakery_data(CHOOSING_TICKET, my_ticket);
	for (they = 0U; they < BAKERY_LOCK_MAX_CPUS; they++) {
		their_ticket = bakery_ticket_number(bakery->lock_data[they]);
		if (their_ticket > my_ticket)
			my_ticket = their_ticket;
	}

	/*
	 * Compute ticket; then signal to other contenders waiting for us to
	 * finish calculating our ticket value that we're done
	 */
	++my_ticket;
	bakery->lock_data[me] = make_bakery_data(CHOSEN_TICKET, my_ticket);

	return my_ticket;
}


/*
 * Acquire bakery lock
 *
 * Contending CPUs need first obtain a non-zero ticket and then calculate
 * priority value. A contending CPU iterate over all other CPUs in the platform,
 * which may be contending for the same lock, in the order of their ordinal
 * position (CPU0, CPU1 and so on). A non-contending CPU will have its ticket
 * (and priority) value as 0. The contending CPU compares its priority with that
 * of others'. The CPU with the highest priority (lowest numerical value)
 * acquires the lock
 */
void bakery_lock_get(bakery_lock_t *bakery)
{
	unsigned int they, me;
	unsigned int my_ticket, my_prio, their_ticket;
	unsigned int their_bakery_data;

	me = plat_my_core_pos();

	assert_bakery_entry_valid(me, bakery);

	/* Get a ticket */
	my_ticket = bakery_get_ticket(bakery, me);

	/*
	 * Now that we got our ticket, compute our priority value, then compare
	 * with that of others, and proceed to acquire the lock
	 */
	my_prio = bakery_get_priority(my_ticket, me);
	for (they = 0U; they < BAKERY_LOCK_MAX_CPUS; they++) {
		if (me == they)
			continue;

		/* Wait for the contender to get their ticket */
		do {
			their_bakery_data = bakery->lock_data[they];
		} while (bakery_is_choosing(their_bakery_data));

		/*
		 * If the other party is a contender, they'll have non-zero
		 * (valid) ticket value. If they do, compare priorities
		 */
		their_ticket = bakery_ticket_number(their_bakery_data);
		if ((their_ticket != 0U) &&
		    (bakery_get_priority(their_ticket, they) < my_prio)) {
			/*
			 * They have higher priority (lower value). Wait for
			 * their ticket value to change (either release the lock
			 * to have it dropped to 0; or drop and probably content
			 * again for the same lock to have an even higher value)
			 */
			do {
				wfe();
			} while (their_ticket ==
				bakery_ticket_number(bakery->lock_data[they]));
		}
	}

	/*
	 * Lock acquired. Ensure that any reads from a shared resource in the
	 * critical section read values after the lock is acquired.
	 */
	dmbld();
}


/* Release the lock and signal contenders */
void bakery_lock_release(bakery_lock_t *bakery)
{
	unsigned int me = plat_my_core_pos();

	assert_bakery_entry_valid(me, bakery);
	assert(bakery_ticket_number(bakery->lock_data[me]) != 0U);

	/*
	 * Ensure that other observers see any stores in the critical section
	 * before releasing the lock. Release the lock by resetting ticket.
	 * Then signal other waiting contenders.
	 */
	dmbst();
	bakery->lock_data[me] = 0U;
	dsb();
	sev();
}
