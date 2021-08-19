/*
 * Copyright (c) 2017-2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <cdefs.h>
#include <stdbool.h>

#include "../amu_private.h"
#include <arch.h>
#include <arch_helpers.h>
#include <common/debug.h>
#include <lib/el3_runtime/pubsub_events.h>
#include <lib/extensions/amu.h>

#include <plat/common/platform.h>

struct amu_ctx {
	uint64_t group0_cnts[AMU_GROUP0_MAX_COUNTERS];
#if ENABLE_AMU_AUXILIARY_COUNTERS
	uint64_t group1_cnts[AMU_GROUP1_MAX_COUNTERS];
#endif

	uint16_t group0_enable;
#if ENABLE_AMU_AUXILIARY_COUNTERS
	uint16_t group1_enable;
#endif
};

static struct amu_ctx amu_ctxs_[PLATFORM_CORE_COUNT];

CASSERT((sizeof(amu_ctxs_[0].group0_enable) * CHAR_BIT) <= AMU_GROUP0_MAX_COUNTERS,
	amu_ctx_group0_enable_cannot_represent_all_group0_counters);

#if ENABLE_AMU_AUXILIARY_COUNTERS
CASSERT((sizeof(amu_ctxs_[0].group1_enable) * CHAR_BIT) <= AMU_GROUP1_MAX_COUNTERS,
	amu_ctx_group1_enable_cannot_represent_all_group1_counters);
#endif

static inline __unused uint32_t read_id_pfr0_amu(void)
{
	return (read_id_pfr0() >> ID_PFR0_AMU_SHIFT) &
		ID_PFR0_AMU_MASK;
}

static inline __unused void write_hcptr_tam(uint32_t value)
{
	write_hcptr((read_hcptr() & ~TAM_BIT) |
		((value << TAM_SHIFT) & TAM_BIT));
}

static inline __unused void write_amcr_cg1rz(uint32_t value)
{
	write_amcr((read_amcr() & ~AMCR_CG1RZ_BIT) |
		((value << AMCR_CG1RZ_SHIFT) & AMCR_CG1RZ_BIT));
}

static inline __unused uint32_t read_amcfgr_ncg(void)
{
	return (read_amcfgr() >> AMCFGR_NCG_SHIFT) &
		AMCFGR_NCG_MASK;
}

static inline __unused uint32_t read_amcgcr_cg0nc(void)
{
	return (read_amcgcr() >> AMCGCR_CG0NC_SHIFT) &
		AMCGCR_CG0NC_MASK;
}

static inline __unused uint32_t read_amcgcr_cg1nc(void)
{
	return (read_amcgcr() >> AMCGCR_CG1NC_SHIFT) &
		AMCGCR_CG1NC_MASK;
}

static inline __unused uint32_t read_amcntenset0_px(void)
{
	return (read_amcntenset0() >> AMCNTENSET0_Pn_SHIFT) &
		AMCNTENSET0_Pn_MASK;
}

static inline __unused uint32_t read_amcntenset1_px(void)
{
	return (read_amcntenset1() >> AMCNTENSET1_Pn_SHIFT) &
		AMCNTENSET1_Pn_MASK;
}

static inline __unused void write_amcntenset0_px(uint32_t px)
{
	uint32_t value = read_amcntenset0();

	value &= ~AMCNTENSET0_Pn_MASK;
	value |= (px << AMCNTENSET0_Pn_SHIFT) &
		AMCNTENSET0_Pn_MASK;

	write_amcntenset0(value);
}

static inline __unused void write_amcntenset1_px(uint32_t px)
{
	uint32_t value = read_amcntenset1();

	value &= ~AMCNTENSET1_Pn_MASK;
	value |= (px << AMCNTENSET1_Pn_SHIFT) &
		AMCNTENSET1_Pn_MASK;

	write_amcntenset1(value);
}

static inline __unused void write_amcntenclr0_px(uint32_t px)
{
	uint32_t value = read_amcntenclr0();

	value &= ~AMCNTENCLR0_Pn_MASK;
	value |= (px << AMCNTENCLR0_Pn_SHIFT) & AMCNTENCLR0_Pn_MASK;

	write_amcntenclr0(value);
}

static inline __unused void write_amcntenclr1_px(uint32_t px)
{
	uint32_t value = read_amcntenclr1();

	value &= ~AMCNTENCLR1_Pn_MASK;
	value |= (px << AMCNTENCLR1_Pn_SHIFT) & AMCNTENCLR1_Pn_MASK;

	write_amcntenclr1(value);
}

static __unused bool amu_supported(void)
{
	return read_id_pfr0_amu() >= ID_PFR0_AMU_V1;
}

#if ENABLE_AMU_AUXILIARY_COUNTERS
static __unused bool amu_group1_supported(void)
{
	return read_amcfgr_ncg() > 0U;
}
#endif

/*
 * Enable counters. This function is meant to be invoked by the context
 * management library before exiting from EL3.
 */
void amu_enable(bool el2_unused)
{
	uint32_t id_pfr0_amu;		/* AMU version */

	uint32_t amcfgr_ncg;		/* Number of counter groups */
	uint32_t amcgcr_cg0nc;		/* Number of group 0 counters */

	uint32_t amcntenset0_px = 0x0;	/* Group 0 enable mask */
	uint32_t amcntenset1_px = 0x0;	/* Group 1 enable mask */

	id_pfr0_amu = read_id_pfr0_amu();
	if (id_pfr0_amu == ID_PFR0_AMU_NOT_SUPPORTED) {
		/*
		 * If the AMU is unsupported, nothing needs to be done.
		 */

		return;
	}

	if (el2_unused) {
		/*
		 * HCPTR.TAM: Set to zero so any accesses to the Activity
		 * Monitor registers do not trap to EL2.
		 */
		write_hcptr_tam(0U);
	}

	/*
	 * Retrieve the number of architected counters. All of these counters
	 * are enabled by default.
	 */

	amcgcr_cg0nc = read_amcgcr_cg0nc();
	amcntenset0_px = (UINT32_C(1) << (amcgcr_cg0nc)) - 1U;

	assert(amcgcr_cg0nc <= AMU_AMCGCR_CG0NC_MAX);

	/*
	 * The platform may opt to enable specific auxiliary counters. This can
	 * be done via the common FCONF getter, or via the platform-implemented
	 * function.
	 */

#if ENABLE_AMU_AUXILIARY_COUNTERS
	const struct amu_topology *topology;

#if ENABLE_AMU_FCONF
	topology = FCONF_GET_PROPERTY(amu, config, topology);
#else
	topology = plat_amu_topology();
#endif /* ENABLE_AMU_FCONF */

	if (topology != NULL) {
		unsigned int core_pos = plat_my_core_pos();

		amcntenset1_el0_px = topology->cores[core_pos].enable;
	} else {
		ERROR("AMU: failed to generate AMU topology\n");
	}
#endif /* ENABLE_AMU_AUXILIARY_COUNTERS */

	/*
	 * Enable the requested counters.
	 */

	write_amcntenset0_px(amcntenset0_px);

	amcfgr_ncg = read_amcfgr_ncg();
	if (amcfgr_ncg > 0U) {
		write_amcntenset1_px(amcntenset1_px);

#if !ENABLE_AMU_AUXILIARY_COUNTERS
		VERBOSE("AMU: auxiliary counters detected but support is disabled\n");
#endif
	}

	/* Initialize FEAT_AMUv1p1 features if present. */
	if (id_pfr0_amu < ID_PFR0_AMU_V1P1) {
		return;
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
	write_amcr_cg1rz(1U);
#else
	write_amcr_cg1rz(0U);
#endif
}

/* Read the group 0 counter identified by the given `idx`. */
static uint64_t amu_group0_cnt_read(unsigned int idx)
{
	assert(amu_supported());
	assert(idx < read_amcgcr_cg0nc());

	return amu_group0_cnt_read_internal(idx);
}

/* Write the group 0 counter identified by the given `idx` with `val` */
static void amu_group0_cnt_write(unsigned  int idx, uint64_t val)
{
	assert(amu_supported());
	assert(idx < read_amcgcr_cg0nc());

	amu_group0_cnt_write_internal(idx, val);
	isb();
}

#if ENABLE_AMU_AUXILIARY_COUNTERS
/* Read the group 1 counter identified by the given `idx` */
static uint64_t amu_group1_cnt_read(unsigned  int idx)
{
	assert(amu_supported());
	assert(amu_group1_supported());
	assert(idx < read_amcgcr_cg1nc());

	return amu_group1_cnt_read_internal(idx);
}

/* Write the group 1 counter identified by the given `idx` with `val` */
static void amu_group1_cnt_write(unsigned  int idx, uint64_t val)
{
	assert(amu_supported());
	assert(amu_group1_supported());
	assert(idx < read_amcgcr_cg1nc());

	amu_group1_cnt_write_internal(idx, val);
	isb();
}
#endif

static void *amu_context_save(const void *arg)
{
	uint32_t i;

	unsigned int core_pos;
	struct amu_ctx *ctx;

	uint32_t id_pfr0_amu;	/* AMU version */
	uint32_t amcgcr_cg0nc;	/* Number of group 0 counters */

#if ENABLE_AMU_AUXILIARY_COUNTERS
	uint32_t amcfgr_ncg;	/* Number of counter groups */
	uint32_t amcgcr_cg1nc;	/* Number of group 1 counters */
#endif

	id_pfr0_amu = read_id_pfr0_amu();
	if (id_pfr0_amu == ID_PFR0_AMU_NOT_SUPPORTED) {
		return (void *)0;
	}

	core_pos = plat_my_core_pos();
	ctx = &amu_ctxs_[core_pos];

	amcgcr_cg0nc = read_amcgcr_cg0nc();

#if ENABLE_AMU_AUXILIARY_COUNTERS
	amcfgr_ncg = read_amcfgr_ncg();
	amcgcr_cg1nc = (amcfgr_ncg > 0U) ? read_amcgcr_cg1nc() : 0U;
#endif

	/*
	 * Disable all AMU counters.
	 */

	ctx->group0_enable = read_amcntenset0_px();
	write_amcntenclr0_px(ctx->group0_enable);

#if ENABLE_AMU_AUXILIARY_COUNTERS
	if (amcfgr_ncg > 0U) {
		ctx->group1_enable = read_amcntenset1_px();
		write_amcntenclr1_px(ctx->group1_enable);
	}
#endif

	/*
	 * Save the counters to the local context.
	 */

	isb(); /* Ensure counters have been stopped */

	for (i = 0U; i < amcgcr_cg0nc; i++) {
		ctx->group0_cnts[i] = amu_group0_cnt_read(i);
	}

#if ENABLE_AMU_AUXILIARY_COUNTERS
	for (i = 0U; i < amcgcr_cg1nc; i++) {
		ctx->group1_cnts[i] = amu_group1_cnt_read(i);
	}
#endif

	return (void *)0;
}

static void *amu_context_restore(const void *arg)
{
	uint32_t i;

	unsigned int core_pos;
	struct amu_ctx *ctx;

	uint32_t id_pfr0_amu;	/* AMU version */

	uint32_t amcfgr_ncg;	/* Number of counter groups */
	uint32_t amcgcr_cg0nc;	/* Number of group 0 counters */

#if ENABLE_AMU_AUXILIARY_COUNTERS
	uint32_t amcgcr_cg1nc;	/* Number of group 1 counters */
#endif

	id_pfr0_amu = read_id_pfr0_amu();
	if (id_pfr0_amu == ID_PFR0_AMU_NOT_SUPPORTED) {
		return (void *)0;
	}

	core_pos = plat_my_core_pos();
	ctx = &amu_ctxs_[core_pos];

	amcfgr_ncg = read_amcfgr_ncg();
	amcgcr_cg0nc = read_amcgcr_cg0nc();

#if ENABLE_AMU_AUXILIARY_COUNTERS
	amcgcr_cg1nc = (amcfgr_ncg > 0U) ? read_amcgcr_cg1nc() : 0U;
#endif

	/*
	 * Sanity check that all counters were disabled when the context was
	 * previously saved.
	 */

	assert(read_amcntenset0_px() == 0U);

	if (amcfgr_ncg > 0U) {
		assert(read_amcntenset1_px() == 0U);
	}

	/*
	 * Restore the counter values from the local context.
	 */

	for (i = 0U; i < amcgcr_cg0nc; i++) {
		amu_group0_cnt_write(i, ctx->group0_cnts[i]);
	}

#if ENABLE_AMU_AUXILIARY_COUNTERS
	for (i = 0U; i < amcgcr_cg1nc; i++) {
		amu_group1_cnt_write(i, ctx->group1_cnts[i]);
	}
#endif

	/*
	 * Re-enable counters that were disabled during context save.
	 */

	write_amcntenset0_px(ctx->group0_enable);

#if ENABLE_AMU_AUXILIARY_COUNTERS
	if (amcfgr_ncg > 0U) {
		write_amcntenset1_px(ctx->group1_enable);
	}
#endif

	return (void *)0;
}

SUBSCRIBE_TO_EVENT(psci_suspend_pwrdown_start, amu_context_save);
SUBSCRIBE_TO_EVENT(psci_suspend_pwrdown_finish, amu_context_restore);
