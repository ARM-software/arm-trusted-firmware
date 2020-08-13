/*
 * Copyright (c) 2017-2020, ARM Limited and Contributors. All rights reserved.
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

static struct amu_ctx amu_ctxs[PLATFORM_CORE_COUNT];

/* Check if AMUv1 for Armv8.4 or 8.6 is implemented */
bool amu_supported(void)
{
	uint32_t features = read_id_pfr0() >> ID_PFR0_AMU_SHIFT;

	features &= ID_PFR0_AMU_MASK;
	return ((features == 1U) || (features == 2U));
}

#if AMU_GROUP1_NR_COUNTERS
/* Check if group 1 counters is implemented */
bool amu_group1_supported(void)
{
	uint32_t features = read_amcfgr() >> AMCFGR_NCG_SHIFT;

	return (features & AMCFGR_NCG_MASK) == 1U;
}
#endif

/*
 * Enable counters. This function is meant to be invoked
 * by the context management library before exiting from EL3.
 */
void amu_enable(bool el2_unused)
{
	if (!amu_supported()) {
		return;
	}

#if AMU_GROUP1_NR_COUNTERS
	/* Check and set presence of group 1 counters */
	if (!amu_group1_supported()) {
		ERROR("AMU Counter Group 1 is not implemented\n");
		panic();
	}

	/* Check number of group 1 counters */
	uint32_t cnt_num = (read_amcgcr() >> AMCGCR_CG1NC_SHIFT) &
				AMCGCR_CG1NC_MASK;
	VERBOSE("%s%u. %s%u\n",
		"Number of AMU Group 1 Counters ", cnt_num,
		"Requested number ", AMU_GROUP1_NR_COUNTERS);

	if (cnt_num < AMU_GROUP1_NR_COUNTERS) {
		ERROR("%s%u is less than %s%u\n",
		"Number of AMU Group 1 Counters ", cnt_num,
		"Requested number ", AMU_GROUP1_NR_COUNTERS);
		panic();
	}
#endif

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

#if AMU_GROUP1_NR_COUNTERS
	/* Enable group 1 counters */
	write_amcntenset1(AMU_GROUP1_COUNTERS_MASK);
#endif
}

/* Read the group 0 counter identified by the given `idx`. */
uint64_t amu_group0_cnt_read(unsigned int idx)
{
	assert(amu_supported());
	assert(idx < AMU_GROUP0_NR_COUNTERS);

	return amu_group0_cnt_read_internal(idx);
}

/* Write the group 0 counter identified by the given `idx` with `val` */
void amu_group0_cnt_write(unsigned  int idx, uint64_t val)
{
	assert(amu_supported());
	assert(idx < AMU_GROUP0_NR_COUNTERS);

	amu_group0_cnt_write_internal(idx, val);
	isb();
}

#if AMU_GROUP1_NR_COUNTERS
/* Read the group 1 counter identified by the given `idx` */
uint64_t amu_group1_cnt_read(unsigned  int idx)
{
	assert(amu_supported());
	assert(amu_group1_supported());
	assert(idx < AMU_GROUP1_NR_COUNTERS);

	return amu_group1_cnt_read_internal(idx);
}

/* Write the group 1 counter identified by the given `idx` with `val` */
void amu_group1_cnt_write(unsigned  int idx, uint64_t val)
{
	assert(amu_supported());
	assert(amu_group1_supported());
	assert(idx < AMU_GROUP1_NR_COUNTERS);

	amu_group1_cnt_write_internal(idx, val);
	isb();
}

/*
 * Program the event type register for the given `idx` with
 * the event number `val`
 */
void amu_group1_set_evtype(unsigned int idx, unsigned int val)
{
	assert(amu_supported());
	assert(amu_group1_supported());
	assert(idx < AMU_GROUP1_NR_COUNTERS);

	amu_group1_set_evtype_internal(idx, val);
	isb();
}
#endif	/* AMU_GROUP1_NR_COUNTERS */

static void *amu_context_save(const void *arg)
{
	struct amu_ctx *ctx = &amu_ctxs[plat_my_core_pos()];
	unsigned int i;

	if (!amu_supported()) {
		return (void *)-1;
	}

#if AMU_GROUP1_NR_COUNTERS
	if (!amu_group1_supported()) {
		return (void *)-1;
	}
#endif
	/* Assert that group 0/1 counter configuration is what we expect */
	assert(read_amcntenset0_el0() == AMU_GROUP0_COUNTERS_MASK);

#if AMU_GROUP1_NR_COUNTERS
	assert(read_amcntenset1_el0() == AMU_GROUP1_COUNTERS_MASK);
#endif
	/*
	 * Disable group 0/1 counters to avoid other observers like SCP sampling
	 * counter values from the future via the memory mapped view.
	 */
	write_amcntenclr0(AMU_GROUP0_COUNTERS_MASK);

#if AMU_GROUP1_NR_COUNTERS
	write_amcntenclr1(AMU_GROUP1_COUNTERS_MASK);
#endif
	isb();

	/* Save all group 0 counters */
	for (i = 0U; i < AMU_GROUP0_NR_COUNTERS; i++) {
		ctx->group0_cnts[i] = amu_group0_cnt_read(i);
	}

#if AMU_GROUP1_NR_COUNTERS
	/* Save group 1 counters */
	for (i = 0U; i < AMU_GROUP1_NR_COUNTERS; i++) {
		if ((AMU_GROUP1_COUNTERS_MASK & (1U << i)) != 0U) {
			ctx->group1_cnts[i] = amu_group1_cnt_read(i);
		}
	}
#endif
	return (void *)0;
}

static void *amu_context_restore(const void *arg)
{
	struct amu_ctx *ctx = &amu_ctxs[plat_my_core_pos()];
	unsigned int i;

	if (!amu_supported()) {
		return (void *)-1;
	}

#if AMU_GROUP1_NR_COUNTERS
	if (!amu_group1_supported()) {
		return (void *)-1;
	}
#endif
	/* Counters were disabled in `amu_context_save()` */
	assert(read_amcntenset0_el0() == 0U);

#if AMU_GROUP1_NR_COUNTERS
	assert(read_amcntenset1_el0() == 0U);
#endif

	/* Restore all group 0 counters */
	for (i = 0U; i < AMU_GROUP0_NR_COUNTERS; i++) {
		amu_group0_cnt_write(i, ctx->group0_cnts[i]);
	}

	/* Restore group 0 counter configuration */
	write_amcntenset0(AMU_GROUP0_COUNTERS_MASK);

#if AMU_GROUP1_NR_COUNTERS
	/* Restore group 1 counters */
	for (i = 0U; i < AMU_GROUP1_NR_COUNTERS; i++) {
		if ((AMU_GROUP1_COUNTERS_MASK & (1U << i)) != 0U) {
			amu_group1_cnt_write(i, ctx->group1_cnts[i]);
		}
	}

	/* Restore group 1 counter configuration */
	write_amcntenset1(AMU_GROUP1_COUNTERS_MASK);
#endif

	return (void *)0;
}

SUBSCRIBE_TO_EVENT(psci_suspend_pwrdown_start, amu_context_save);
SUBSCRIBE_TO_EVENT(psci_suspend_pwrdown_finish, amu_context_restore);
