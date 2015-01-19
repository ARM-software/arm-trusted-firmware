/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
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

#include <arch_helpers.h>
#include <assert.h>
#include <bakery_lock.h>
#include <cpu_data.h>
#include <platform.h>
#include <string.h>

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

/* Convert a ticket to priority */
#define PRIORITY(t, pos)	(((t) << 8) | (pos))

#define CHOOSING_TICKET		0x1
#define CHOOSING_DONE		0x0

#define bakery_is_choosing(info)	(info & 0x1)
#define bakery_ticket_number(info)	((info >> 1) & 0x7FFF)
#define make_bakery_data(choosing, number) \
		(((choosing & 0x1) | (number << 1)) & 0xFFFF)

/* This macro assumes that the bakery_info array is located at the offset specified */
#define get_my_bakery_info(offset, id)		\
	(((bakery_info_t *) (((uint8_t *)_cpu_data()) + offset)) + id)

#define get_bakery_info_by_index(offset, id, ix)	\
	(((bakery_info_t *) (((uint8_t *)_cpu_data_by_index(ix)) + offset)) + id)

#define write_cache_op(addr, cached)	\
				do {	\
					(cached ? dccvac((uint64_t)addr) :\
						dcivac((uint64_t)addr));\
						dsbish();\
				} while (0)

#define read_cache_op(addr, cached)	if (cached) \
					    dccivac((uint64_t)addr)

static unsigned int bakery_get_ticket(int id, unsigned int offset,
						unsigned int me, int is_cached)
{
	unsigned int my_ticket, their_ticket;
	unsigned int they;
	bakery_info_t *my_bakery_info, *their_bakery_info;

	/*
	 * Obtain a reference to the bakery information for this cpu and ensure
	 * it is not NULL.
	 */
	my_bakery_info = get_my_bakery_info(offset, id);
	assert(my_bakery_info);

	/*
	 * Tell other contenders that we are through the bakery doorway i.e.
	 * going to allocate a ticket for this cpu.
	 */
	my_ticket = 0;
	my_bakery_info->lock_data = make_bakery_data(CHOOSING_TICKET, my_ticket);

	write_cache_op(my_bakery_info, is_cached);

	/*
	 * Iterate through the bakery information of each contender to allocate
	 * the highest ticket number for this cpu.
	 */
	for (they = 0; they < BAKERY_LOCK_MAX_CPUS; they++) {
		if (me == they)
			continue;

		/*
		 * Get a reference to the other contender's bakery info and
		 * ensure that a stale copy is not read.
		 */
		their_bakery_info = get_bakery_info_by_index(offset, id, they);
		assert(their_bakery_info);

		read_cache_op(their_bakery_info, is_cached);

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
	my_bakery_info->lock_data = make_bakery_data(CHOOSING_DONE, my_ticket);

	write_cache_op(my_bakery_info, is_cached);

	return my_ticket;
}

void bakery_lock_get(unsigned int id, unsigned int offset)
{
	unsigned int they, me, is_cached;
	unsigned int my_ticket, my_prio, their_ticket;
	bakery_info_t *their_bakery_info;
	uint16_t their_bakery_data;

	me = platform_get_core_pos(read_mpidr_el1());

	is_cached = read_sctlr_el3() & SCTLR_C_BIT;

	/* Get a ticket */
	my_ticket = bakery_get_ticket(id, offset, me, is_cached);

	/*
	 * Now that we got our ticket, compute our priority value, then compare
	 * with that of others, and proceed to acquire the lock
	 */
	my_prio = PRIORITY(my_ticket, me);
	for (they = 0; they < BAKERY_LOCK_MAX_CPUS; they++) {
		if (me == they)
			continue;

		/*
		 * Get a reference to the other contender's bakery info and
		 * ensure that a stale copy is not read.
		 */
		their_bakery_info = get_bakery_info_by_index(offset, id, they);
		assert(their_bakery_info);
		read_cache_op(their_bakery_info, is_cached);

		their_bakery_data = their_bakery_info->lock_data;

		/* Wait for the contender to get their ticket */
		while (bakery_is_choosing(their_bakery_data)) {
			read_cache_op(their_bakery_info, is_cached);
			their_bakery_data = their_bakery_info->lock_data;
		}

		/*
		 * If the other party is a contender, they'll have non-zero
		 * (valid) ticket value. If they do, compare priorities
		 */
		their_ticket = bakery_ticket_number(their_bakery_data);
		if (their_ticket && (PRIORITY(their_ticket, they) < my_prio)) {
			/*
			 * They have higher priority (lower value). Wait for
			 * their ticket value to change (either release the lock
			 * to have it dropped to 0; or drop and probably content
			 * again for the same lock to have an even higher value)
			 */
			do {
				wfe();
				read_cache_op(their_bakery_info, is_cached);
			} while (their_ticket
				== bakery_ticket_number(their_bakery_info->lock_data));
		}
	}
}

void bakery_lock_release(unsigned int id, unsigned int offset)
{
	bakery_info_t *my_bakery_info;
	unsigned int is_cached = read_sctlr_el3() & SCTLR_C_BIT;

	my_bakery_info = get_my_bakery_info(offset, id);
	my_bakery_info->lock_data = 0;
	write_cache_op(my_bakery_info, is_cached);
	sev();
}
