/*
 * Copyright (c) 2017-2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdbool.h>

#include <arch.h>
#include <arch_features.h>
#include <arch_helpers.h>

#include <lib/el3_runtime/pubsub_events.h>
#include <lib/extensions/amu.h>
#include <lib/extensions/amu_private.h>

#include <plat/common/platform.h>

static struct amu_ctx amu_ctxs[PLATFORM_CORE_COUNT];

/*
 * Get AMU version value from aa64pfr0.
 * Return values
 *   ID_AA64PFR0_AMU_V1: FEAT_AMUv1 supported (introduced in ARM v8.4)
 *   ID_AA64PFR0_AMU_V1P1: FEAT_AMUv1p1 supported (introduced in ARM v8.6)
 *   ID_AA64PFR0_AMU_NOT_SUPPORTED: not supported
 */
unsigned int amu_get_version(void)
{
	return (unsigned int)(read_id_aa64pfr0_el1() >> ID_AA64PFR0_AMU_SHIFT) &
		ID_AA64PFR0_AMU_MASK;
}

#if AMU_GROUP1_NR_COUNTERS
/* Check if group 1 counters is implemented */
bool amu_group1_supported(void)
{
	uint64_t features = read_amcfgr_el0() >> AMCFGR_EL0_NCG_SHIFT;

	return (features & AMCFGR_EL0_NCG_MASK) == 1U;
}
#endif

/*
 * Enable counters. This function is meant to be invoked
 * by the context management library before exiting from EL3.
 */
void amu_enable(bool el2_unused)
{
	uint64_t v;
	unsigned int amu_version = amu_get_version();

	if (amu_version == ID_AA64PFR0_AMU_NOT_SUPPORTED) {
		return;
	}

#if AMU_GROUP1_NR_COUNTERS
	/* Check and set presence of group 1 counters */
	if (!amu_group1_supported()) {
		ERROR("AMU Counter Group 1 is not implemented\n");
		panic();
	}

	/* Check number of group 1 counters */
	uint64_t cnt_num = (read_amcgcr_el0() >> AMCGCR_EL0_CG1NC_SHIFT) &
				AMCGCR_EL0_CG1NC_MASK;
	VERBOSE("%s%llu. %s%u\n",
		"Number of AMU Group 1 Counters ", cnt_num,
		"Requested number ", AMU_GROUP1_NR_COUNTERS);

	if (cnt_num < AMU_GROUP1_NR_COUNTERS) {
		ERROR("%s%llu is less than %s%u\n",
		"Number of AMU Group 1 Counters ", cnt_num,
		"Requested number ", AMU_GROUP1_NR_COUNTERS);
		panic();
	}
#endif

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

#if AMU_GROUP1_NR_COUNTERS
	/* Enable group 1 counters */
	write_amcntenset1_el0(AMU_GROUP1_COUNTERS_MASK);
#endif

	/* Initialize FEAT_AMUv1p1 features if present. */
	if (amu_version < ID_AA64PFR0_AMU_V1P1) {
		return;
	}

	if (el2_unused) {
		/* Make sure virtual offsets are disabled if EL2 not used. */
		write_hcr_el2(read_hcr_el2() & ~HCR_AMVOFFEN_BIT);
	}

#if AMU_RESTRICT_COUNTERS
	/*
	 * FEAT_AMUv1p1 adds a register field to restrict access to group 1
	 * counters at all but the highest implemented EL.  This is controlled
	 * with the AMU_RESTRICT_COUNTERS compile time flag, when set, system
	 * register reads at lower ELs return zero.  Reads from the memory
	 * mapped view are unaffected.
	 */
	VERBOSE("AMU group 1 counter access restricted.\n");
	write_amcr_el0(read_amcr_el0() | AMCR_CG1RZ_BIT);
#else
	write_amcr_el0(read_amcr_el0() & ~AMCR_CG1RZ_BIT);
#endif
}

/* Read the group 0 counter identified by the given `idx`. */
uint64_t amu_group0_cnt_read(unsigned int idx)
{
	assert(amu_get_version() != ID_AA64PFR0_AMU_NOT_SUPPORTED);
	assert(idx < AMU_GROUP0_NR_COUNTERS);

	return amu_group0_cnt_read_internal(idx);
}

/* Write the group 0 counter identified by the given `idx` with `val` */
void amu_group0_cnt_write(unsigned  int idx, uint64_t val)
{
	assert(amu_get_version() != ID_AA64PFR0_AMU_NOT_SUPPORTED);
	assert(idx < AMU_GROUP0_NR_COUNTERS);

	amu_group0_cnt_write_internal(idx, val);
	isb();
}

/*
 * Read the group 0 offset register for a given index. Index must be 0, 2,
 * or 3, the register for 1 does not exist.
 *
 * Using this function requires FEAT_AMUv1p1 support.
 */
uint64_t amu_group0_voffset_read(unsigned int idx)
{
	assert(amu_get_version() >= ID_AA64PFR0_AMU_V1P1);
	assert(idx < AMU_GROUP0_NR_COUNTERS);
	assert(idx != 1U);

	return amu_group0_voffset_read_internal(idx);
}

/*
 * Write the group 0 offset register for a given index. Index must be 0, 2, or
 * 3, the register for 1 does not exist.
 *
 * Using this function requires FEAT_AMUv1p1 support.
 */
void amu_group0_voffset_write(unsigned int idx, uint64_t val)
{
	assert(amu_get_version() >= ID_AA64PFR0_AMU_V1P1);
	assert(idx < AMU_GROUP0_NR_COUNTERS);
	assert(idx != 1U);

	amu_group0_voffset_write_internal(idx, val);
	isb();
}

#if AMU_GROUP1_NR_COUNTERS
/* Read the group 1 counter identified by the given `idx` */
uint64_t amu_group1_cnt_read(unsigned int idx)
{
	assert(amu_get_version() != ID_AA64PFR0_AMU_NOT_SUPPORTED);
	assert(amu_group1_supported());
	assert(idx < AMU_GROUP1_NR_COUNTERS);

	return amu_group1_cnt_read_internal(idx);
}

/* Write the group 1 counter identified by the given `idx` with `val` */
void amu_group1_cnt_write(unsigned int idx, uint64_t val)
{
	assert(amu_get_version() != ID_AA64PFR0_AMU_NOT_SUPPORTED);
	assert(amu_group1_supported());
	assert(idx < AMU_GROUP1_NR_COUNTERS);

	amu_group1_cnt_write_internal(idx, val);
	isb();
}

/*
 * Read the group 1 offset register for a given index.
 *
 * Using this function requires FEAT_AMUv1p1 support.
 */
uint64_t amu_group1_voffset_read(unsigned int idx)
{
	assert(amu_get_version() >= ID_AA64PFR0_AMU_V1P1);
	assert(amu_group1_supported());
	assert(idx < AMU_GROUP1_NR_COUNTERS);
	assert(((read_amcg1idr_el0() >> AMCG1IDR_VOFF_SHIFT) &
		(1ULL << idx)) != 0ULL);

	return amu_group1_voffset_read_internal(idx);
}

/*
 * Write the group 1 offset register for a given index.
 *
 * Using this function requires FEAT_AMUv1p1 support.
 */
void amu_group1_voffset_write(unsigned int idx, uint64_t val)
{
	assert(amu_get_version() >= ID_AA64PFR0_AMU_V1P1);
	assert(amu_group1_supported());
	assert(idx < AMU_GROUP1_NR_COUNTERS);
	assert(((read_amcg1idr_el0() >> AMCG1IDR_VOFF_SHIFT) &
		(1ULL << idx)) != 0ULL);

	amu_group1_voffset_write_internal(idx, val);
	isb();
}

/*
 * Program the event type register for the given `idx` with
 * the event number `val`
 */
void amu_group1_set_evtype(unsigned int idx, unsigned int val)
{
	assert(amu_get_version() != ID_AA64PFR0_AMU_NOT_SUPPORTED);
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

	if (amu_get_version() == ID_AA64PFR0_AMU_NOT_SUPPORTED) {
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
	write_amcntenclr0_el0(AMU_GROUP0_COUNTERS_MASK);

#if AMU_GROUP1_NR_COUNTERS
	write_amcntenclr1_el0(AMU_GROUP1_COUNTERS_MASK);
#endif
	isb();

	/* Save all group 0 counters */
	for (i = 0U; i < AMU_GROUP0_NR_COUNTERS; i++) {
		ctx->group0_cnts[i] = amu_group0_cnt_read(i);
	}

	/* Save group 0 virtual offsets if supported and enabled. */
	if ((amu_get_version() >= ID_AA64PFR0_AMU_V1P1) &&
			((read_hcr_el2() & HCR_AMVOFFEN_BIT) != 0ULL)) {
		/* Not using a loop because count is fixed and index 1 DNE. */
		ctx->group0_voffsets[0U] = amu_group0_voffset_read(0U);
		ctx->group0_voffsets[1U] = amu_group0_voffset_read(2U);
		ctx->group0_voffsets[2U] = amu_group0_voffset_read(3U);
	}

#if AMU_GROUP1_NR_COUNTERS
	/* Save group 1 counters */
	for (i = 0U; i < AMU_GROUP1_NR_COUNTERS; i++) {
		if ((AMU_GROUP1_COUNTERS_MASK & (1UL << i)) != 0U) {
			ctx->group1_cnts[i] = amu_group1_cnt_read(i);
		}
	}

	/* Save group 1 virtual offsets if supported and enabled. */
	if ((amu_get_version() >= ID_AA64PFR0_AMU_V1P1) &&
			((read_hcr_el2() & HCR_AMVOFFEN_BIT) != 0ULL)) {
		u_register_t amcg1idr = read_amcg1idr_el0() >>
			AMCG1IDR_VOFF_SHIFT;
		amcg1idr = amcg1idr & AMU_GROUP1_COUNTERS_MASK;

		for (i = 0U; i < AMU_GROUP1_NR_COUNTERS; i++) {
			if (((amcg1idr >> i) & 1ULL) != 0ULL) {
				ctx->group1_voffsets[i] =
					amu_group1_voffset_read(i);
			}
		}
	}
#endif
	return (void *)0;
}

static void *amu_context_restore(const void *arg)
{
	struct amu_ctx *ctx = &amu_ctxs[plat_my_core_pos()];
	unsigned int i;

	if (amu_get_version() == ID_AA64PFR0_AMU_NOT_SUPPORTED) {
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

	/* Restore group 0 virtual offsets if supported and enabled. */
	if ((amu_get_version() >= ID_AA64PFR0_AMU_V1P1) &&
			((read_hcr_el2() & HCR_AMVOFFEN_BIT) != 0ULL)) {
		/* Not using a loop because count is fixed and index 1 DNE. */
		amu_group0_voffset_write(0U, ctx->group0_voffsets[0U]);
		amu_group0_voffset_write(2U, ctx->group0_voffsets[1U]);
		amu_group0_voffset_write(3U, ctx->group0_voffsets[2U]);
	}

	/* Restore group 0 counter configuration */
	write_amcntenset0_el0(AMU_GROUP0_COUNTERS_MASK);

#if AMU_GROUP1_NR_COUNTERS
	/* Restore group 1 counters */
	for (i = 0U; i < AMU_GROUP1_NR_COUNTERS; i++) {
		if ((AMU_GROUP1_COUNTERS_MASK & (1UL << i)) != 0U) {
			amu_group1_cnt_write(i, ctx->group1_cnts[i]);
		}
	}

	/* Restore group 1 virtual offsets if supported and enabled. */
	if ((amu_get_version() >= ID_AA64PFR0_AMU_V1P1) &&
			((read_hcr_el2() & HCR_AMVOFFEN_BIT) != 0ULL)) {
		u_register_t amcg1idr = read_amcg1idr_el0() >>
			AMCG1IDR_VOFF_SHIFT;
		amcg1idr = amcg1idr & AMU_GROUP1_COUNTERS_MASK;

		for (i = 0U; i < AMU_GROUP1_NR_COUNTERS; i++) {
			if (((amcg1idr >> i) & 1ULL) != 0ULL) {
				amu_group1_voffset_write(i,
					ctx->group1_voffsets[i]);
			}
		}
	}

	/* Restore group 1 counter configuration */
	write_amcntenset1_el0(AMU_GROUP1_COUNTERS_MASK);
#endif

	return (void *)0;
}

SUBSCRIBE_TO_EVENT(psci_suspend_pwrdown_start, amu_context_save);
SUBSCRIBE_TO_EVENT(psci_suspend_pwrdown_finish, amu_context_restore);
