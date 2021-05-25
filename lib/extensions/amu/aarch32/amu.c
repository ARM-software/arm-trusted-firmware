/*
 * Copyright (c) 2017-2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <cdefs.h>
#include <stdbool.h>

#include <arch.h>
#include <arch_helpers.h>

#include <lib/el3_runtime/pubsub_events.h>
#include <lib/extensions/amu.h>
#include <lib/extensions/amu_private.h>

#include <plat/common/platform.h>

static struct amu_ctx amu_ctxs[PLATFORM_CORE_COUNT];

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

static bool amu_supported(void)
{
	return read_id_pfr0_amu() >= ID_PFR0_AMU_V1;
}

static bool amu_v1p1_supported(void)
{
	return read_id_pfr0_amu() >= ID_PFR0_AMU_V1P1;
}

#if ENABLE_AMU_AUXILIARY_COUNTERS
static bool amu_group1_supported(void)
{
	return read_amcfgr_ncg() > 0U;
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

	if (el2_unused) {
		/*
		 * Non-secure access from EL0 or EL1 to the Activity Monitor
		 * registers do not trap to EL2.
		 */
		write_hcptr_tam(0U);
	}

	/* Enable group 0 counters */
	write_amcntenset0_px((UINT32_C(1) << read_amcgcr_cg0nc()) - 1U);

#if ENABLE_AMU_AUXILIARY_COUNTERS
	if (amu_group1_supported()) {
		/* Enable group 1 counters */
		write_amcntenset1_px(AMU_GROUP1_COUNTERS_MASK);
	}
#endif

	/* Initialize FEAT_AMUv1p1 features if present. */
	if (!amu_v1p1_supported()) {
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
	struct amu_ctx *ctx = &amu_ctxs[plat_my_core_pos()];
	unsigned int i;

	if (!amu_supported()) {
		return (void *)-1;
	}

	/* Assert that group 0/1 counter configuration is what we expect */
	assert(read_amcntenset0_px() ==
		((UINT32_C(1) << read_amcgcr_cg0nc()) - 1U));

#if ENABLE_AMU_AUXILIARY_COUNTERS
	if (amu_group1_supported()) {
		assert(read_amcntenset1_px() == AMU_GROUP1_COUNTERS_MASK);
	}
#endif
	/*
	 * Disable group 0/1 counters to avoid other observers like SCP sampling
	 * counter values from the future via the memory mapped view.
	 */
	write_amcntenclr0_px((UINT32_C(1) << read_amcgcr_cg0nc()) - 1U);

#if ENABLE_AMU_AUXILIARY_COUNTERS
	if (amu_group1_supported()) {
		write_amcntenclr1_px(AMU_GROUP1_COUNTERS_MASK);
	}
#endif

	isb();

	/* Save all group 0 counters */
	for (i = 0U; i < read_amcgcr_cg0nc(); i++) {
		ctx->group0_cnts[i] = amu_group0_cnt_read(i);
	}

#if ENABLE_AMU_AUXILIARY_COUNTERS
	if (amu_group1_supported()) {
		/* Save group 1 counters */
		for (i = 0U; i < read_amcgcr_cg1nc(); i++) {
			if ((AMU_GROUP1_COUNTERS_MASK & (1U << i)) != 0U) {
				ctx->group1_cnts[i] = amu_group1_cnt_read(i);
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

	/* Counters were disabled in `amu_context_save()` */
	assert(read_amcntenset0_px() == 0U);

#if ENABLE_AMU_AUXILIARY_COUNTERS
	if (amu_group1_supported()) {
		assert(read_amcntenset1_px() == 0U);
	}
#endif

	/* Restore all group 0 counters */
	for (i = 0U; i < read_amcgcr_cg0nc(); i++) {
		amu_group0_cnt_write(i, ctx->group0_cnts[i]);
	}

	/* Restore group 0 counter configuration */
	write_amcntenset0_px((UINT32_C(1) << read_amcgcr_cg0nc()) - 1U);

#if ENABLE_AMU_AUXILIARY_COUNTERS
	if (amu_group1_supported()) {
		/* Restore group 1 counters */
		for (i = 0U; i < read_amcgcr_cg1nc(); i++) {
			if ((AMU_GROUP1_COUNTERS_MASK & (1U << i)) != 0U) {
				amu_group1_cnt_write(i, ctx->group1_cnts[i]);
			}
		}

		/* Restore group 1 counter configuration */
		write_amcntenset1_px(AMU_GROUP1_COUNTERS_MASK);
	}
#endif

	return (void *)0;
}

SUBSCRIBE_TO_EVENT(psci_suspend_pwrdown_start, amu_context_save);
SUBSCRIBE_TO_EVENT(psci_suspend_pwrdown_finish, amu_context_restore);
