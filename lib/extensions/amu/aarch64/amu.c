/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <amu.h>
#include <amu_private.h>
#include <arch.h>
#include <arch_helpers.h>
#include <assert.h>
#include <debug.h>

#define AMU_GROUP0_NR_COUNTERS	4

int amu_supported(void)
{
	uint64_t features;

	features = read_id_aa64pfr0_el1() >> ID_AA64PFR0_AMU_SHIFT;
	return (features & ID_AA64PFR0_AMU_MASK) == 1;
}

/*
 * Enable counters.  This function is meant to be invoked
 * by the context management library before exiting from EL3.
 */
void amu_enable(int el2_unused)
{
	uint64_t v;

	if (!amu_supported()) {
		WARN("Cannot enable AMU - not supported\n");
		return;
	}

	if (el2_unused) {
		/*
		 * CPTR_EL2.TAM: Set to zero so any accesses to
		 * the Activity Monitor registers do not trap to EL2.
		 */
		v = read_cptr_el2();
		v &= ~CPTR_EL2_TAM_BIT;
		write_cptr_el2(v);
	}

	/*
	 * CPTR_EL3.TAM: Set to zero so that any accesses to
	 * the Activity Monitor registers do not trap to EL3.
	 */
	v = read_cptr_el3();
	v &= ~TAM_BIT;
	write_cptr_el3(v);

	/* Enable group 0 counters */
	write_amcntenset0_el0(AMU_GROUP0_COUNTERS_MASK);
	/* Enable group 1 counters */
	write_amcntenset1_el0(AMU_GROUP1_COUNTERS_MASK);
}

/* Read the group 0 counter identified by the given `idx`. */
uint64_t amu_group0_cnt_read(int idx)
{
	assert(amu_supported());
	assert(idx >= 0 && idx < AMU_GROUP0_NR_COUNTERS);

	return amu_group0_cnt_read_internal(idx);
}

/* Write the group 0 counter identified by the given `idx` with `val`. */
void amu_group0_cnt_write(int idx, uint64_t val)
{
	assert(amu_supported());
	assert(idx >= 0 && idx < AMU_GROUP0_NR_COUNTERS);

	amu_group0_cnt_write_internal(idx, val);
	isb();
}

/* Read the group 1 counter identified by the given `idx`. */
uint64_t amu_group1_cnt_read(int idx)
{
	assert(amu_supported());
	assert(idx >= 0 && idx < AMU_GROUP1_NR_COUNTERS);

	return amu_group1_cnt_read_internal(idx);
}

/* Write the group 1 counter identified by the given `idx` with `val`. */
void amu_group1_cnt_write(int idx, uint64_t val)
{
	assert(amu_supported());
	assert(idx >= 0 && idx < AMU_GROUP1_NR_COUNTERS);

	amu_group1_cnt_write_internal(idx, val);
	isb();
}

/*
 * Program the event type register for the given `idx` with
 * the event number `val`.
 */
void amu_group1_set_evtype(int idx, unsigned int val)
{
	assert(amu_supported());
	assert (idx >= 0 && idx < AMU_GROUP1_NR_COUNTERS);

	amu_group1_set_evtype_internal(idx, val);
	isb();
}
