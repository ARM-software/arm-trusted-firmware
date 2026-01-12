/*
 * Copyright (c) 2017-2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <cdefs.h>
#include <stdbool.h>

#include <arch.h>
#include <arch_features.h>
#include <arch_helpers.h>
#include <common/debug.h>
#include <lib/el3_runtime/pubsub_events.h>
#include <lib/extensions/amu.h>
#include <lib/per_cpu/per_cpu.h>

#include <plat/common/platform.h>

PER_CPU_DEFINE(amu_regs_t, amu_ctx);

static inline __unused uint32_t read_amcgcr_cg1nc(void)
{
	return (read_amcgcr() >> AMCGCR_CG1NC_SHIFT) &
		AMCGCR_CG1NC_MASK;
}

/*
 * Enable counters. This function is meant to be invoked by the context
 * management library before exiting from EL3.
 */
void amu_enable(bool el2_unused)
{
	if (el2_unused) {
		/*
		 * HCPTR.TAM: Set to zero so any accesses to the Activity
		 * Monitor registers do not trap to EL2.
		 */
		write_hcptr(read_hcptr() & ~TAM_BIT);
	}

	/* Architecture is currently pinned to 4 */
	assert((read_amcgcr() & AMCGCR_CG0NC_MASK) == CTX_AMU_GRP0_ALL);

	/* Enable all architected counters by default */
	write_amcntenset0(AMCNTENSET0_Pn_MASK);

	/* Bail out if FEAT_AMUv1p1 features are not present. */
	if (!is_feat_amuv1p1_supported()) {
		return;
	}

	if (is_feat_amu_aux_supported()) {
		unsigned int core_pos = plat_my_core_pos();

		/* Something went wrong if we're trying to write higher bits */
		assert((get_amu_aux_enables(core_pos) & ~AMCNTENSET1_Pn_MASK) == 0);
		write_amcntenset1(get_amu_aux_enables(core_pos));
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
	write_amcr(read_amcr() | 1U);
#else
	write_amcr(0);
#endif
}

static void amu_disable_counters(unsigned int core_pos)
{
	/* Disable all counters so we can write to them safely later */
	write_amcntenclr0(AMCNTENCLR0_Pn_MASK);
	if (is_feat_amu_aux_supported()) {
		write_amcntenclr1(get_amu_aux_enables(core_pos));
	}

	isb(); /* Ensure counters have been stopped */
}

static void *amu_context_save(const void *arg)
{
	if (!is_feat_amu_supported()) {
		return (void *)0;
	}

	unsigned int core_pos = *(unsigned int *)arg;
	amu_regs_t *ctx = PER_CPU_CUR(amu_ctx);

	/* disable counters so the save is a static snapshot for all counters */
	amu_disable_counters(core_pos);

	write_amu_grp0_ctx_reg(ctx, 0, read64_amevcntr00());
	write_amu_grp0_ctx_reg(ctx, 1, read64_amevcntr01());
	write_amu_grp0_ctx_reg(ctx, 2, read64_amevcntr02());
	write_amu_grp0_ctx_reg(ctx, 3, read64_amevcntr03());

	if (is_feat_amu_aux_supported()) {
		uint8_t num_counters = read_amcgcr_cg1nc();

		switch (num_counters) {
		case 0x10:
			write_amu_grp1_ctx_reg(ctx, 0xf, read64_amevcntr1f());
			__fallthrough;
		case 0x0f:
			write_amu_grp1_ctx_reg(ctx, 0xe, read64_amevcntr1e());
			__fallthrough;
		case 0x0e:
			write_amu_grp1_ctx_reg(ctx, 0xd, read64_amevcntr1d());
			__fallthrough;
		case 0x0d:
			write_amu_grp1_ctx_reg(ctx, 0xc, read64_amevcntr1c());
			__fallthrough;
		case 0x0c:
			write_amu_grp1_ctx_reg(ctx, 0xb, read64_amevcntr1b());
			__fallthrough;
		case 0x0b:
			write_amu_grp1_ctx_reg(ctx, 0xa, read64_amevcntr1a());
			__fallthrough;
		case 0x0a:
			write_amu_grp1_ctx_reg(ctx, 0x9, read64_amevcntr19());
			__fallthrough;
		case 0x09:
			write_amu_grp1_ctx_reg(ctx, 0x8, read64_amevcntr18());
			__fallthrough;
		case 0x08:
			write_amu_grp1_ctx_reg(ctx, 0x7, read64_amevcntr17());
			__fallthrough;
		case 0x07:
			write_amu_grp1_ctx_reg(ctx, 0x6, read64_amevcntr16());
			__fallthrough;
		case 0x06:
			write_amu_grp1_ctx_reg(ctx, 0x5, read64_amevcntr15());
			__fallthrough;
		case 0x05:
			write_amu_grp1_ctx_reg(ctx, 0x4, read64_amevcntr14());
			__fallthrough;
		case 0x04:
			write_amu_grp1_ctx_reg(ctx, 0x3, read64_amevcntr13());
			__fallthrough;
		case 0x03:
			write_amu_grp1_ctx_reg(ctx, 0x2, read64_amevcntr12());
			__fallthrough;
		case 0x02:
			write_amu_grp1_ctx_reg(ctx, 0x1, read64_amevcntr11());
			__fallthrough;
		case 0x01:
			write_amu_grp1_ctx_reg(ctx, 0x0, read64_amevcntr10());
			__fallthrough;
		case 0x00:
			break;
		default:
			assert(0); /* something is wrong */
		}
	}

	return (void *)0;
}

static void *amu_context_restore(const void *arg)
{
	if (!is_feat_amu_supported()) {
		return (void *)0;
	}

	unsigned int core_pos = *(unsigned int *)arg;
	amu_regs_t *ctx = PER_CPU_CUR(amu_ctx);

	/*
	 * Counters must be disabled to write them safely. All counters start
	 * disabled on an AMU reset but AMU reset doesn't have to happen with PE
	 * reset. So don't bother disabling them if they already are.
	 */
	if (read_amcntenclr0() != 0) {
		amu_disable_counters(core_pos);
	}

	write64_amevcntr00(read_amu_grp0_ctx_reg(ctx, 0));
	write64_amevcntr01(read_amu_grp0_ctx_reg(ctx, 1));
	write64_amevcntr02(read_amu_grp0_ctx_reg(ctx, 2));
	write64_amevcntr03(read_amu_grp0_ctx_reg(ctx, 3));

	if (is_feat_amu_aux_supported()) {
		uint8_t num_counters = read_amcgcr_cg1nc();

		switch (num_counters) {
		case 0x10:
			write64_amevcntr1f(read_amu_grp1_ctx_reg(ctx, 0xf));
			__fallthrough;
		case 0x0f:
			write64_amevcntr1e(read_amu_grp1_ctx_reg(ctx, 0xe));
			__fallthrough;
		case 0x0e:
			write64_amevcntr1d(read_amu_grp1_ctx_reg(ctx, 0xd));
			__fallthrough;
		case 0x0d:
			write64_amevcntr1c(read_amu_grp1_ctx_reg(ctx, 0xc));
			__fallthrough;
		case 0x0c:
			write64_amevcntr1b(read_amu_grp1_ctx_reg(ctx, 0xb));
			__fallthrough;
		case 0x0b:
			write64_amevcntr1a(read_amu_grp1_ctx_reg(ctx, 0xa));
			__fallthrough;
		case 0x0a:
			write64_amevcntr19(read_amu_grp1_ctx_reg(ctx, 0x9));
			__fallthrough;
		case 0x09:
			write64_amevcntr18(read_amu_grp1_ctx_reg(ctx, 0x8));
			__fallthrough;
		case 0x08:
			write64_amevcntr17(read_amu_grp1_ctx_reg(ctx, 0x7));
			__fallthrough;
		case 0x07:
			write64_amevcntr16(read_amu_grp1_ctx_reg(ctx, 0x6));
			__fallthrough;
		case 0x06:
			write64_amevcntr15(read_amu_grp1_ctx_reg(ctx, 0x5));
			__fallthrough;
		case 0x05:
			write64_amevcntr14(read_amu_grp1_ctx_reg(ctx, 0x4));
			__fallthrough;
		case 0x04:
			write64_amevcntr13(read_amu_grp1_ctx_reg(ctx, 0x3));
			__fallthrough;
		case 0x03:
			write64_amevcntr12(read_amu_grp1_ctx_reg(ctx, 0x2));
			__fallthrough;
		case 0x02:
			write64_amevcntr11(read_amu_grp1_ctx_reg(ctx, 0x1));
			__fallthrough;
		case 0x01:
			write64_amevcntr10(read_amu_grp1_ctx_reg(ctx, 0x0));
			__fallthrough;
		case 0x00:
			break;
		default:
			assert(0); /* something is wrong */
		}
	}


	/* now enable them again */
	write_amcntenset0(AMCNTENSET0_Pn_MASK);
	if (is_feat_amu_aux_supported()) {
		write_amcntenset1(get_amu_aux_enables(core_pos));
	}

	isb();
	return (void *)0;
}

SUBSCRIBE_TO_EVENT(psci_suspend_pwrdown_start, amu_context_save);
SUBSCRIBE_TO_EVENT(psci_suspend_pwrdown_finish, amu_context_restore);
