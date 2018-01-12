/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <cortex_a75.h>
#include <pubsub_events.h>
#include <platform.h>

struct amu_ctx {
	uint64_t cnts[CORTEX_A75_AMU_NR_COUNTERS];
	uint16_t mask;
};

static struct amu_ctx amu_ctxs[PLATFORM_CORE_COUNT];

static void *cortex_a75_context_save(const void *arg)
{
	struct amu_ctx *ctx = &amu_ctxs[plat_my_core_pos()];
	unsigned int midr;
	unsigned int midr_mask;
	int i;

	midr = read_midr();
	midr_mask = (MIDR_IMPL_MASK << MIDR_IMPL_SHIFT) |
		(MIDR_PN_MASK << MIDR_PN_SHIFT);
	if ((midr & midr_mask) != (CORTEX_A75_MIDR & midr_mask))
		return 0;

	/* Save counter configuration */
	ctx->mask = cortex_a75_amu_read_cpuamcntenset_el0();

	/* Ensure counters are disabled */
	cortex_a75_amu_write_cpuamcntenclr_el0(ctx->mask);
	isb();

	/* Save counters */
	for (i = 0; i < CORTEX_A75_AMU_NR_COUNTERS; i++)
		ctx->cnts[i] = cortex_a75_amu_cnt_read(i);

	return 0;
}

static void *cortex_a75_context_restore(const void *arg)
{
	struct amu_ctx *ctx = &amu_ctxs[plat_my_core_pos()];
	unsigned int midr;
	unsigned int midr_mask;
	int i;

	midr = read_midr();
	midr_mask = (MIDR_IMPL_MASK << MIDR_IMPL_SHIFT) |
		(MIDR_PN_MASK << MIDR_PN_SHIFT);
	if ((midr & midr_mask) != (CORTEX_A75_MIDR & midr_mask))
		return 0;

	ctx = &amu_ctxs[plat_my_core_pos()];

	/* Counters were disabled in `cortex_a75_context_save()` */
	assert(cortex_a75_amu_read_cpuamcntenset_el0() == 0);

	/* Restore counters */
	for (i = 0; i < CORTEX_A75_AMU_NR_COUNTERS; i++)
		cortex_a75_amu_cnt_write(i, ctx->cnts[i]);
	isb();

	/* Restore counter configuration */
	cortex_a75_amu_write_cpuamcntenset_el0(ctx->mask);

	return 0;
}

SUBSCRIBE_TO_EVENT(psci_suspend_pwrdown_start, cortex_a75_context_save);
SUBSCRIBE_TO_EVENT(psci_suspend_pwrdown_finish, cortex_a75_context_restore);
