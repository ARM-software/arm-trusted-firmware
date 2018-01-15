/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <amu.h>
#include <arch.h>
#include <arch_helpers.h>
#include <platform.h>
#include <pubsub_events.h>

#define AMU_GROUP0_NR_COUNTERS	4

struct amu_ctx {
	uint64_t group0_cnts[AMU_GROUP0_NR_COUNTERS];
};

static struct amu_ctx amu_ctxs[PLATFORM_CORE_COUNT];

void amu_enable(int el2_unused)
{
	uint64_t features;

	features = read_id_pfr0() >> ID_PFR0_AMU_SHIFT;
	if ((features & ID_PFR0_AMU_MASK) != 1)
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
}

static void *amu_context_save(const void *arg)
{
	struct amu_ctx *ctx;
	uint64_t features;

	features = read_id_pfr0() >> ID_PFR0_AMU_SHIFT;
	if ((features & ID_PFR0_AMU_MASK) != 1)
		return (void *)-1;

	ctx = &amu_ctxs[plat_my_core_pos()];

	/* Assert that group 0 counter configuration is what we expect */
	assert(read_amcntenset0() == AMU_GROUP0_COUNTERS_MASK);

	/*
	 * Disable group 0 counters to avoid other observers like SCP sampling
	 * counter values from the future via the memory mapped view.
	 */
	write_amcntenclr0(AMU_GROUP0_COUNTERS_MASK);
	isb();

	ctx->group0_cnts[0] = read64_amevcntr00();
	ctx->group0_cnts[1] = read64_amevcntr01();
	ctx->group0_cnts[2] = read64_amevcntr02();
	ctx->group0_cnts[3] = read64_amevcntr03();

	return 0;
}

static void *amu_context_restore(const void *arg)
{
	struct amu_ctx *ctx;
	uint64_t features;

	features = read_id_pfr0() >> ID_PFR0_AMU_SHIFT;
	if ((features & ID_PFR0_AMU_MASK) != 1)
		return (void *)-1;

	ctx = &amu_ctxs[plat_my_core_pos()];

	/* Counters were disabled in `amu_context_save()` */
	assert(read_amcntenset0() == 0);

	/* Restore group 0 counters */
	if (AMU_GROUP0_COUNTERS_MASK & (1U << 0))
		write64_amevcntr00(ctx->group0_cnts[0]);
	if (AMU_GROUP0_COUNTERS_MASK & (1U << 1))
		write64_amevcntr01(ctx->group0_cnts[1]);
	if (AMU_GROUP0_COUNTERS_MASK & (1U << 2))
		write64_amevcntr02(ctx->group0_cnts[2]);
	if (AMU_GROUP0_COUNTERS_MASK & (1U << 3))
		write64_amevcntr03(ctx->group0_cnts[3]);
	isb();

	/* Enable group 0 counters */
	write_amcntenset0(AMU_GROUP0_COUNTERS_MASK);

	return 0;
}

SUBSCRIBE_TO_EVENT(psci_suspend_pwrdown_start, amu_context_save);
SUBSCRIBE_TO_EVENT(psci_suspend_pwrdown_finish, amu_context_restore);
