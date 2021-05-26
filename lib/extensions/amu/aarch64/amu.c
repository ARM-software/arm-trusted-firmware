/*
 * Copyright (c) 2017-2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <cdefs.h>
#include <stdbool.h>

#include <arch.h>
#include <arch_features.h>
#include <arch_helpers.h>

#include <lib/el3_runtime/pubsub_events.h>
#include <lib/extensions/amu.h>
#include <lib/extensions/amu_private.h>

#include <plat/common/platform.h>

static struct amu_ctx amu_ctxs[PLATFORM_CORE_COUNT];

static inline __unused uint64_t read_id_aa64pfr0_el1_amu(void)
{
	return (read_id_aa64pfr0_el1() >> ID_AA64PFR0_AMU_SHIFT) &
		ID_AA64PFR0_AMU_MASK;
}

static inline __unused uint64_t read_hcr_el2_amvoffen(void)
{
	return (read_hcr_el2() & HCR_AMVOFFEN_BIT) >>
		HCR_AMVOFFEN_SHIFT;
}

static inline __unused void write_cptr_el2_tam(uint64_t value)
{
	write_cptr_el2((read_cptr_el2() & ~CPTR_EL2_TAM_BIT) |
		((value << CPTR_EL2_TAM_SHIFT) & CPTR_EL2_TAM_BIT));
}

static inline __unused void write_cptr_el3_tam(cpu_context_t *ctx, uint64_t tam)
{
	uint64_t value = read_ctx_reg(get_el3state_ctx(ctx), CTX_CPTR_EL3);

	value &= ~TAM_BIT;
	value |= (tam << TAM_SHIFT) & TAM_BIT;

	write_ctx_reg(get_el3state_ctx(ctx), CTX_CPTR_EL3, value);
}

static inline __unused void write_hcr_el2_amvoffen(uint64_t value)
{
	write_hcr_el2((read_hcr_el2() & ~HCR_AMVOFFEN_BIT) |
		((value << HCR_AMVOFFEN_SHIFT) & HCR_AMVOFFEN_BIT));
}

static inline __unused void write_amcr_el0_cg1rz(uint64_t value)
{
	write_amcr_el0((read_amcr_el0() & ~AMCR_CG1RZ_BIT) |
		((value << AMCR_CG1RZ_SHIFT) & AMCR_CG1RZ_BIT));
}

static inline __unused uint64_t read_amcfgr_el0_ncg(void)
{
	return (read_amcfgr_el0() >> AMCFGR_EL0_NCG_SHIFT) &
		AMCFGR_EL0_NCG_MASK;
}

static inline __unused uint64_t read_amcg1idr_el0_voff(void)
{
	return (read_amcg1idr_el0() >> AMCG1IDR_VOFF_SHIFT) &
		AMCG1IDR_VOFF_MASK;
}

static inline __unused uint64_t read_amcgcr_el0_cg1nc(void)
{
	return (read_amcgcr_el0() >> AMCGCR_EL0_CG1NC_SHIFT) &
		AMCGCR_EL0_CG1NC_MASK;
}

static inline __unused uint64_t read_amcntenset0_el0_px(void)
{
	return (read_amcntenset0_el0() >> AMCNTENSET0_EL0_Pn_SHIFT) &
		AMCNTENSET0_EL0_Pn_MASK;
}

static inline __unused uint64_t read_amcntenset1_el0_px(void)
{
	return (read_amcntenset1_el0() >> AMCNTENSET1_EL0_Pn_SHIFT) &
		AMCNTENSET1_EL0_Pn_MASK;
}

static inline __unused void write_amcntenset0_el0_px(uint64_t px)
{
	uint64_t value = read_amcntenset0_el0();

	value &= ~AMCNTENSET0_EL0_Pn_MASK;
	value |= (px << AMCNTENSET0_EL0_Pn_SHIFT) & AMCNTENSET0_EL0_Pn_MASK;

	write_amcntenset0_el0(value);
}

static inline __unused void write_amcntenset1_el0_px(uint64_t px)
{
	uint64_t value = read_amcntenset1_el0();

	value &= ~AMCNTENSET1_EL0_Pn_MASK;
	value |= (px << AMCNTENSET1_EL0_Pn_SHIFT) & AMCNTENSET1_EL0_Pn_MASK;

	write_amcntenset1_el0(value);
}

static inline __unused void write_amcntenclr0_el0_px(uint64_t px)
{
	uint64_t value = read_amcntenclr0_el0();

	value &= ~AMCNTENCLR0_EL0_Pn_MASK;
	value |= (px << AMCNTENCLR0_EL0_Pn_SHIFT) & AMCNTENCLR0_EL0_Pn_MASK;

	write_amcntenclr0_el0(value);
}

static inline __unused void write_amcntenclr1_el0_px(uint64_t px)
{
	uint64_t value = read_amcntenclr1_el0();

	value &= ~AMCNTENCLR1_EL0_Pn_MASK;
	value |= (px << AMCNTENCLR1_EL0_Pn_SHIFT) & AMCNTENCLR1_EL0_Pn_MASK;

	write_amcntenclr1_el0(value);
}

static bool amu_supported(void)
{
	return read_id_aa64pfr0_el1_amu() >= ID_AA64PFR0_AMU_V1;
}

static bool amu_v1p1_supported(void)
{
	return read_id_aa64pfr0_el1_amu() >= ID_AA64PFR0_AMU_V1P1;
}

#if ENABLE_AMU_AUXILIARY_COUNTERS
static bool amu_group1_supported(void)
{
	return read_amcfgr_el0_ncg() > 0U;
}
#endif

/*
 * Enable counters. This function is meant to be invoked
 * by the context management library before exiting from EL3.
 */
void amu_enable(bool el2_unused, cpu_context_t *ctx)
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
	uint64_t cnt_num = read_amcgcr_el0_cg1nc();
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
		write_cptr_el2_tam(0U);
	}

	/*
	 * Retrieve and update the CPTR_EL3 value from the context mentioned
	 * in 'ctx'. Set CPTR_EL3.TAM to zero so that any accesses to
	 * the Activity Monitor registers do not trap to EL3.
	 */
	write_cptr_el3_tam(ctx, 0U);

	/* Enable group 0 counters */
	write_amcntenset0_el0_px(AMU_GROUP0_COUNTERS_MASK);

#if AMU_GROUP1_NR_COUNTERS
	/* Enable group 1 counters */
	write_amcntenset1_el0_px(AMU_GROUP1_COUNTERS_MASK);
#endif

	/* Initialize FEAT_AMUv1p1 features if present. */
	if (!amu_v1p1_supported()) {
		return;
	}

	if (el2_unused) {
		/* Make sure virtual offsets are disabled if EL2 not used. */
		write_hcr_el2_amvoffen(0U);
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
	write_amcr_el0_cg1rz(1U);
#else
	write_amcr_el0_cg1rz(0U);
#endif
}

/* Read the group 0 counter identified by the given `idx`. */
static uint64_t amu_group0_cnt_read(unsigned int idx)
{
	assert(amu_supported());
	assert(idx < AMU_GROUP0_NR_COUNTERS);

	return amu_group0_cnt_read_internal(idx);
}

/* Write the group 0 counter identified by the given `idx` with `val` */
static void amu_group0_cnt_write(unsigned  int idx, uint64_t val)
{
	assert(amu_supported());
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
static uint64_t amu_group0_voffset_read(unsigned int idx)
{
	assert(amu_v1p1_supported());
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
static void amu_group0_voffset_write(unsigned int idx, uint64_t val)
{
	assert(amu_v1p1_supported());
	assert(idx < AMU_GROUP0_NR_COUNTERS);
	assert(idx != 1U);

	amu_group0_voffset_write_internal(idx, val);
	isb();
}

#if AMU_GROUP1_NR_COUNTERS
/* Read the group 1 counter identified by the given `idx` */
static uint64_t amu_group1_cnt_read(unsigned int idx)
{
	assert(amu_supported());
	assert(amu_group1_supported());
	assert(idx < AMU_GROUP1_NR_COUNTERS);

	return amu_group1_cnt_read_internal(idx);
}

/* Write the group 1 counter identified by the given `idx` with `val` */
static void amu_group1_cnt_write(unsigned int idx, uint64_t val)
{
	assert(amu_supported());
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
static uint64_t amu_group1_voffset_read(unsigned int idx)
{
	assert(amu_v1p1_supported());
	assert(amu_group1_supported());
	assert(idx < AMU_GROUP1_NR_COUNTERS);
	assert((read_amcg1idr_el0_voff() & (UINT64_C(1) << idx)) != 0U);

	return amu_group1_voffset_read_internal(idx);
}

/*
 * Write the group 1 offset register for a given index.
 *
 * Using this function requires FEAT_AMUv1p1 support.
 */
static void amu_group1_voffset_write(unsigned int idx, uint64_t val)
{
	assert(amu_v1p1_supported());
	assert(amu_group1_supported());
	assert(idx < AMU_GROUP1_NR_COUNTERS);
	assert((read_amcg1idr_el0_voff() & (UINT64_C(1) << idx)) != 0U);

	amu_group1_voffset_write_internal(idx, val);
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
	assert(read_amcntenset0_el0_px() == AMU_GROUP0_COUNTERS_MASK);

#if AMU_GROUP1_NR_COUNTERS
	assert(read_amcntenset1_el0_px() == AMU_GROUP1_COUNTERS_MASK);
#endif
	/*
	 * Disable group 0/1 counters to avoid other observers like SCP sampling
	 * counter values from the future via the memory mapped view.
	 */
	write_amcntenclr0_el0_px(AMU_GROUP0_COUNTERS_MASK);

#if AMU_GROUP1_NR_COUNTERS
	write_amcntenclr1_el0_px(AMU_GROUP1_COUNTERS_MASK);
#endif
	isb();

	/* Save all group 0 counters */
	for (i = 0U; i < AMU_GROUP0_NR_COUNTERS; i++) {
		ctx->group0_cnts[i] = amu_group0_cnt_read(i);
	}

	/* Save group 0 virtual offsets if supported and enabled. */
	if (amu_v1p1_supported() && (read_hcr_el2_amvoffen() != 0U)) {
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
	if (amu_v1p1_supported() && (read_hcr_el2_amvoffen() != 0U)) {
		uint64_t amcg1idr = read_amcg1idr_el0_voff() &
			AMU_GROUP1_COUNTERS_MASK;

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

	if (!amu_supported()) {
		return (void *)-1;
	}

#if AMU_GROUP1_NR_COUNTERS
	if (!amu_group1_supported()) {
		return (void *)-1;
	}
#endif
	/* Counters were disabled in `amu_context_save()` */
	assert(read_amcntenset0_el0_px() == 0U);

#if AMU_GROUP1_NR_COUNTERS
	assert(read_amcntenset1_el0_px() == 0U);
#endif

	/* Restore all group 0 counters */
	for (i = 0U; i < AMU_GROUP0_NR_COUNTERS; i++) {
		amu_group0_cnt_write(i, ctx->group0_cnts[i]);
	}

	/* Restore group 0 virtual offsets if supported and enabled. */
	if (amu_v1p1_supported() && (read_hcr_el2_amvoffen() != 0U)) {
		/* Not using a loop because count is fixed and index 1 DNE. */
		amu_group0_voffset_write(0U, ctx->group0_voffsets[0U]);
		amu_group0_voffset_write(2U, ctx->group0_voffsets[1U]);
		amu_group0_voffset_write(3U, ctx->group0_voffsets[2U]);
	}

	/* Restore group 0 counter configuration */
	write_amcntenset0_el0_px(AMU_GROUP0_COUNTERS_MASK);

#if AMU_GROUP1_NR_COUNTERS
	/* Restore group 1 counters */
	for (i = 0U; i < AMU_GROUP1_NR_COUNTERS; i++) {
		if ((AMU_GROUP1_COUNTERS_MASK & (1UL << i)) != 0U) {
			amu_group1_cnt_write(i, ctx->group1_cnts[i]);
		}
	}

	/* Restore group 1 virtual offsets if supported and enabled. */
	if (amu_v1p1_supported() && (read_hcr_el2_amvoffen() != 0U)) {
		uint64_t amcg1idr = read_amcg1idr_el0_voff() &
			AMU_GROUP1_COUNTERS_MASK;

		for (i = 0U; i < AMU_GROUP1_NR_COUNTERS; i++) {
			if (((amcg1idr >> i) & 1ULL) != 0ULL) {
				amu_group1_voffset_write(i,
					ctx->group1_voffsets[i]);
			}
		}
	}

	/* Restore group 1 counter configuration */
	write_amcntenset1_el0_px(AMU_GROUP1_COUNTERS_MASK);
#endif

	return (void *)0;
}

SUBSCRIBE_TO_EVENT(psci_suspend_pwrdown_start, amu_context_save);
SUBSCRIBE_TO_EVENT(psci_suspend_pwrdown_finish, amu_context_restore);
