/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <cpuamu.h>
#include <lib/el3_runtime/pubsub_events.h>
#include <plat/common/platform.h>

#define CPUAMU_NR_COUNTERS	5U

struct cpuamu_ctx {
	uint64_t cnts[CPUAMU_NR_COUNTERS];
	unsigned int mask;
};

static struct cpuamu_ctx cpuamu_ctxs[PLATFORM_CORE_COUNT];

int midr_match(unsigned int cpu_midr)
{
	unsigned int midr, midr_mask;

	midr = (unsigned int)read_midr();
	midr_mask = (MIDR_IMPL_MASK << MIDR_IMPL_SHIFT) |
		(MIDR_PN_MASK << MIDR_PN_SHIFT);
	return ((midr & midr_mask) == (cpu_midr & midr_mask));
}

void cpuamu_context_save(unsigned int nr_counters)
{
	struct cpuamu_ctx *ctx = &cpuamu_ctxs[plat_my_core_pos()];
	unsigned int i;

	assert(nr_counters <= CPUAMU_NR_COUNTERS);

	/* Save counter configuration */
	ctx->mask = cpuamu_read_cpuamcntenset_el0();

	/* Disable counters */
	cpuamu_write_cpuamcntenclr_el0(ctx->mask);
	isb();

	/* Save counters */
	for (i = 0; i < nr_counters; i++)
		ctx->cnts[i] = cpuamu_cnt_read(i);
}

void cpuamu_context_restore(unsigned int nr_counters)
{
	struct cpuamu_ctx *ctx = &cpuamu_ctxs[plat_my_core_pos()];
	unsigned int i;

	assert(nr_counters <= CPUAMU_NR_COUNTERS);

	/*
	 * Disable counters.  They were enabled early in the
	 * CPU reset function.
	 */
	cpuamu_write_cpuamcntenclr_el0(ctx->mask);
	isb();

	/* Restore counters */
	for (i = 0; i < nr_counters; i++)
		cpuamu_cnt_write(i, ctx->cnts[i]);
	isb();

	/* Restore counter configuration */
	cpuamu_write_cpuamcntenset_el0(ctx->mask);
}
