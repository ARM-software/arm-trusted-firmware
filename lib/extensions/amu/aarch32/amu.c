/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

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

	features = read_id_pfr0() >> ID_PFR0_AMU_SHIFT;
	return (features & ID_PFR0_AMU_MASK) == 1U;
}

void amu_enable(bool el2_unused)
{
	if (!amu_supported())
		return;

	if (el2_unused) {
		uint64_t v;
		/*
		 * Non-secure access from EL0 or EL1 to the Activity Monitor
		 * registers do not trap to EL2.
		 */
		v = read_hcptr();
		v &= ~TAM_BIT;
		write_hcptr(v);
	}

	/* Enable group 0 counters */
	write_amcntenset0(AMU_GROUP0_COUNTERS_MASK);

	/* Enable group 1 counters */
	write_amcntenset1(AMU_GROUP1_COUNTERS_MASK);
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

void amu_group1_set_evtype(int idx, unsigned int val)
{
	assert(amu_supported());
	assert((idx >= 0) && (idx < AMU_GROUP1_NR_COUNTERS));

	amu_group1_set_evtype_internal(idx, val);
	isb();
}

static void *amu_context_save(const void *arg)
{
	struct amu_ctx *ctx;
	int i;

	if (!amu_supported())
		return (void *)-1;

	ctx = &amu_ctxs[plat_my_core_pos()];

	/* Assert that group 0 counter configuration is what we expect */
	assert(read_amcntenset0() == AMU_GROUP0_COUNTERS_MASK &&
	       read_amcntenset1() == AMU_GROUP1_COUNTERS_MASK);

	/*
	 * Disable group 0 counters to avoid other observers like SCP sampling
	 * counter values from the future via the memory mapped view.
	 */
	write_amcntenclr0(AMU_GROUP0_COUNTERS_MASK);
	write_amcntenclr1(AMU_GROUP1_COUNTERS_MASK);
	isb();

	for (i = 0; i < AMU_GROUP0_NR_COUNTERS; i++)
		ctx->group0_cnts[i] = amu_group0_cnt_read(i);

	for (i = 0; i < AMU_GROUP1_NR_COUNTERS; i++)
		ctx->group1_cnts[i] = amu_group1_cnt_read(i);

	return (void *)0;
}

static void *amu_context_restore(const void *arg)
{
	struct amu_ctx *ctx;
	int i;

	if (!amu_supported())
		return (void *)-1;

	ctx = &amu_ctxs[plat_my_core_pos()];

	/* Counters were disabled in `amu_context_save()` */
	assert((read_amcntenset0() == 0U) && (read_amcntenset1() == 0U));

	/* Restore group 0 counters */
	for (i = 0; i < AMU_GROUP0_NR_COUNTERS; i++)
		amu_group0_cnt_write(i, ctx->group0_cnts[i]);
	for (i = 0; i < AMU_GROUP1_NR_COUNTERS; i++)
		amu_group1_cnt_write(i, ctx->group1_cnts[i]);

	/* Enable group 0 counters */
	write_amcntenset0(AMU_GROUP0_COUNTERS_MASK);

	/* Enable group 1 counters */
	write_amcntenset1(AMU_GROUP1_COUNTERS_MASK);
	return (void *)0;
}

SUBSCRIBE_TO_EVENT(psci_suspend_pwrdown_start, amu_context_save);
SUBSCRIBE_TO_EVENT(psci_suspend_pwrdown_finish, amu_context_restore);
