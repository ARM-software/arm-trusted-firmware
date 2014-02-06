/*
 * Copyright (c) 2013-2014, ARM Limited and Contributors. All rights reserved.
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

#include <assert.h>
#include <string.h>

#include <bakery_lock.h>

#define assert_bakery_entry_valid(entry, bakery) do {	\
	assert(bakery);					\
	assert(entry < BAKERY_LOCK_MAX_CPUS);		\
} while(0)

void bakery_lock_init(bakery_lock * bakery)
{
	assert(bakery);
	memset(bakery, 0, sizeof(*bakery));
	bakery->owner = NO_OWNER;
}

void bakery_lock_get(unsigned long mpidr, bakery_lock * bakery)
{
	unsigned int i, max = 0, my_full_number, his_full_number, entry;

	entry = platform_get_core_pos(mpidr);

	assert_bakery_entry_valid(entry, bakery);

	// Catch recursive attempts to take the lock under the same entry:
	assert(bakery->owner != entry);

	// Get a ticket
	bakery->entering[entry] = 1;
	for (i = 0; i < BAKERY_LOCK_MAX_CPUS; ++i) {
		if (bakery->number[i] > max) {
			max = bakery->number[i];
		}
	}
	++max;
	bakery->number[entry] = max;
	bakery->entering[entry] = 0;

	// Wait for our turn
	my_full_number = (max << 8) + entry;
	for (i = 0; i < BAKERY_LOCK_MAX_CPUS; ++i) {
		while (bakery->entering[i]) ;	/* Wait */
		do {
			his_full_number = bakery->number[i];
			if (his_full_number) {
				his_full_number = (his_full_number << 8) + i;
			}
		}
		while (his_full_number && (his_full_number < my_full_number));
	}

	bakery->owner = entry;
}

void bakery_lock_release(unsigned long mpidr, bakery_lock * bakery)
{
	unsigned int entry = platform_get_core_pos(mpidr);

	assert_bakery_entry_valid(entry, bakery);
	assert(bakery->owner == entry);

	bakery->owner = NO_OWNER;
	bakery->number[entry] = 0;
}
