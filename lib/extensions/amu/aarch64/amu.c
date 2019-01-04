/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdbool.h>

#include <arch.h>
#include <arch_helpers.h>
#include <lib/el3_runtime/pubsub_events.h>
#include <lib/extensions/amu.h>
#include <lib/extensions/amu_private.h>
#include <plat/common/platform.h>

#define AMU_GROUP0_NR_COUNTERS	4

struct amu_ctx {
	uint64_t group0_cnts[AMU_GROUP0_NR_COUNTERS];
	uint64_t group1_cnts[AMU_GROUP1_NR_COUNTERS];
};

static struct amu_ctx amu_ctxs[PLATFORM_CORE_COUNT];

bool amu_supported(void)
{
	uint64_t features;

	features = read_id_aa64pfr0_el1() >> ID_AA64PFR0_AMU_SHIFT;
	return (features & ID_AA64PFR0_AMU_MASK) == 1U;
}

/*
 * Enable counters.  This function is meant to be invoked
 * by the context management library before exiting from EL3.
 */
void amu_enable(bool el2_unused)
{
	uint64_t v;

	if (!amu_supported())
		return;

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
	assert((idx >= 0) && (idx < AMU_GROUP0_NR_COUNTERS));

	return amu_group0_cnt_read_internal(idx);
}

/* Write the group 0 counter identified by the given `idx` with `val`. */
void amu_group0_cnt_write(int idx, uint64_t val)
{
	assert(amu_supported());
	assert((idx >= 0) && (idx < AMU_GROUP0_NR_COUNTERS));

	amu_group0_cnt_write_internal(idx, val);
	isb();
}

/* Read the group 1 counter identified by the given `idx`. */
uint64_t amu_group1_cnt_read(int idx)
{
	assert(amu_supported());
	assert((idx >= 0) && (idx < AMU_GROUP1_NR_COUNTERS));

	return amu_group1_cnt_read_internal(idx);
}

/* Write the group 1 counter identified by the given `idx` with `val`. */
void amu_group1_cnt_write(int idx, uint64_t val)
{
	assert(amu_supported());
	assert((idx >= 0) && (idx < AMU_GROUP1_NR_COUNTERS));

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
	assert((idx >= 0) && (idx < AMU_GROUP1_NR_COUNTERS));

	amu_group1_set_evtype_internal(idx, val);
	isb();
}

static void *amu_context_save(const void *arg)
{
	struct amu_ctx *ctx = &amu_ctxs[plat_my_core_pos()];
	int i;

	if (!amu_supported())
		return (void *)-1;

	/* Assert that group 0/1 counter configuration is what we expect */
	assert((read_amcntenset0_el0() == AMU_GROUP0_COUNTERS_MASK) &&
	       (read_amcntenset1_el0() == AMU_GROUP1_COUNTERS_MASK));

	assert(((sizeof(int) * 8) - __builtin_clz(AMU_GROUP1_COUNTERS_MASK))
		<= AMU_GROUP1_NR_COUNTERS);

	/*
	 * Disable group 0/1 counters to avoid other observers like SCP sampling
	 * counter values from the future via the memory mapped view.
	 */
	write_amcntenclr0_el0(AMU_GROUP0_COUNTERS_MASK);
	write_amcntenclr1_el0(AMU_GROUP1_COUNTERS_MASK);
	isb();

	/* Save group 0 counters */
	for (i = 0; i < AMU_GROUP0_NR_COUNTERS; i++)
		ctx->group0_cnts[i] = amu_group0_cnt_read(i);

	/* Save group 1 counters */
	for (i = 0; i < AMU_GROUP1_NR_COUNTERS; i++)
		ctx->group1_cnts[i] = amu_group1_cnt_read(i);

	return (void *)0;
}

static void *amu_context_restore(const void *arg)
{
	struct amu_ctx *ctx = &amu_ctxs[plat_my_core_pos()];
	int i;

	if (!amu_supported())
		return (void *)-1;

	/* Counters were disabled in `amu_context_save()` */
	assert((read_amcntenset0_el0() == 0U) && (read_amcntenset1_el0() == 0U));

	assert(((sizeof(int) * 8U) - __builtin_clz(AMU_GROUP1_COUNTERS_MASK))
		<= AMU_GROUP1_NR_COUNTERS);

	/* Restore group 0 counters */
	for (i = 0; i < AMU_GROUP0_NR_COUNTERS; i++)
		if ((AMU_GROUP0_COUNTERS_MASK & (1U << i)) != 0U)
			amu_group0_cnt_write(i, ctx->group0_cnts[i]);

	/* Restore group 1 counters */
	for (i = 0; i < AMU_GROUP1_NR_COUNTERS; i++)
		if ((AMU_GROUP1_COUNTERS_MASK & (1U << i)) != 0U)
			amu_group1_cnt_write(i, ctx->group1_cnts[i]);

	/* Restore group 0/1 counter configuration */
	write_amcntenset0_el0(AMU_GROUP0_COUNTERS_MASK);
	write_amcntenset1_el0(AMU_GROUP1_COUNTERS_MASK);

	return (void *)0;
}

SUBSCRIBE_TO_EVENT(psci_suspend_pwrdown_start, amu_context_save);
SUBSCRIBE_TO_EVENT(psci_suspend_pwrdown_finish, amu_context_restore);
