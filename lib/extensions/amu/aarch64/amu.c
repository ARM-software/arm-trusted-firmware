/*
 * Copyright (c) 2017-2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <cdefs.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>

#include <arch.h>
#include <arch_features.h>
#include <arch_helpers.h>
#include <common/debug.h>
#include <lib/el3_runtime/pubsub_events.h>
#include <lib/extensions/amu.h>
#include <lib/utils_def.h>
#include <platform_def.h>

amu_regs_t amu_ctx[PLATFORM_CORE_COUNT];

static inline uint8_t read_amcgcr_el0_cg1nc(void)
{
	return (read_amcgcr_el0() >> AMCGCR_EL0_CG1NC_SHIFT) &
		AMCGCR_EL0_CG1NC_MASK;
}

void amu_enable(cpu_context_t *ctx)
{
	/* Initialize FEAT_AMUv1p1 features if present. */
	if (is_feat_amuv1p1_supported()) {
		el3_state_t *state = get_el3state_ctx(ctx);
		u_register_t reg;

		/*
		 * Set SCR_EL3.AMVOFFEN to one so that accesses to virtual
		 * offset registers at EL2 do not trap to EL3
		 */
		reg = read_ctx_reg(state, CTX_SCR_EL3);
		reg |= SCR_AMVOFFEN_BIT;
		write_ctx_reg(state, CTX_SCR_EL3, reg);
	}
}

void amu_enable_per_world(per_world_context_t *per_world_ctx)
{
	/*
	 * Set CPTR_EL3.TAM to zero so that any accesses to the Activity Monitor
	 * registers do not trap to EL3.
	 */
	uint64_t cptr_el3 = per_world_ctx->ctx_cptr_el3;

	cptr_el3 &= ~TAM_BIT;
	per_world_ctx->ctx_cptr_el3 = cptr_el3;
}

void amu_init_el3(unsigned int core_pos)
{
	/* architecture is currently pinned to 4 */
	assert((read_amcgcr_el0() & AMCGCR_EL0_CG0NC_MASK) == CTX_AMU_GRP0_ALL);

	/* Enable all architected counters by default */
	write_amcntenset0_el0(AMCNTENSET0_EL0_Pn_MASK);
	if (is_feat_amu_aux_supported()) {
		/* something went wrong if we're trying to write higher bits */
		assert((get_amu_aux_enables(core_pos) & ~AMCNTENSET1_EL0_Pn_MASK) == 0);
		write_amcntenset1_el0(get_amu_aux_enables(core_pos));
	}

	if (is_feat_amuv1p1_supported()) {
#if AMU_RESTRICT_COUNTERS
		/*
		 * FEAT_AMUv1p1 adds a register field to restrict access to
		 * group 1 counters at all but the highest implemented EL. This
		 * is controlled with the `AMU_RESTRICT_COUNTERS` compile time
		 * flag, when set, system register reads at lower ELs return
		 * zero. Reads from the memory mapped view are unaffected.
		 */
		VERBOSE("AMU group 1 counter access restricted.\n");
		write_amcr_el0(AMCR_CG1RZ_BIT);
#else
		/* HDBG = 0 in both cases */
		write_amcr_el0(0);
#endif
	}
}

void amu_init_el2_unused(void)
{
	/*
	 * CPTR_EL2.TAM: Set to zero so any accesses to the Activity Monitor
	 *  registers do not trap to EL2.
	 */
	write_cptr_el2(read_cptr_el2() & ~CPTR_EL2_TAM_BIT);

	if (is_feat_amuv1p1_supported()) {
		/* Make sure virtual offsets are disabled */
		write_hcr_el2(read_hcr_el2() & ~HCR_AMVOFFEN_BIT);
	}
}

static void *amu_context_save(const void *arg)
{
	if (!is_feat_amu_supported()) {
		return (void *)0;
	}

	unsigned int core_pos = *(unsigned int *)arg;
	amu_regs_t *ctx = &amu_ctx[core_pos];

	/* disable all counters so we can write them safely later */
	write_amcntenclr0_el0(AMCNTENCLR0_EL0_Pn_MASK);
	if (is_feat_amu_aux_supported()) {
		write_amcntenclr1_el0(get_amu_aux_enables(core_pos));
	}

	isb();

	write_amu_grp0_ctx_reg(ctx, 0, read_amevcntr00_el0());
	write_amu_grp0_ctx_reg(ctx, 1, read_amevcntr01_el0());
	write_amu_grp0_ctx_reg(ctx, 2, read_amevcntr02_el0());
	write_amu_grp0_ctx_reg(ctx, 3, read_amevcntr03_el0());

	if (is_feat_amu_aux_supported()) {
		uint8_t num_counters = read_amcgcr_el0_cg1nc();

		switch (num_counters) {
		case 0x10:
			write_amu_grp1_ctx_reg(ctx, 0xf, read_amevcntr1f_el0());
			__fallthrough;
		case 0x0f:
			write_amu_grp1_ctx_reg(ctx, 0xe, read_amevcntr1e_el0());
			__fallthrough;
		case 0x0e:
			write_amu_grp1_ctx_reg(ctx, 0xd, read_amevcntr1d_el0());
			__fallthrough;
		case 0x0d:
			write_amu_grp1_ctx_reg(ctx, 0xc, read_amevcntr1c_el0());
			__fallthrough;
		case 0x0c:
			write_amu_grp1_ctx_reg(ctx, 0xb, read_amevcntr1b_el0());
			__fallthrough;
		case 0x0b:
			write_amu_grp1_ctx_reg(ctx, 0xa, read_amevcntr1a_el0());
			__fallthrough;
		case 0x0a:
			write_amu_grp1_ctx_reg(ctx, 0x9, read_amevcntr19_el0());
			__fallthrough;
		case 0x09:
			write_amu_grp1_ctx_reg(ctx, 0x8, read_amevcntr18_el0());
			__fallthrough;
		case 0x08:
			write_amu_grp1_ctx_reg(ctx, 0x7, read_amevcntr17_el0());
			__fallthrough;
		case 0x07:
			write_amu_grp1_ctx_reg(ctx, 0x6, read_amevcntr16_el0());
			__fallthrough;
		case 0x06:
			write_amu_grp1_ctx_reg(ctx, 0x5, read_amevcntr15_el0());
			__fallthrough;
		case 0x05:
			write_amu_grp1_ctx_reg(ctx, 0x4, read_amevcntr14_el0());
			__fallthrough;
		case 0x04:
			write_amu_grp1_ctx_reg(ctx, 0x3, read_amevcntr13_el0());
			__fallthrough;
		case 0x03:
			write_amu_grp1_ctx_reg(ctx, 0x2, read_amevcntr12_el0());
			__fallthrough;
		case 0x02:
			write_amu_grp1_ctx_reg(ctx, 0x1, read_amevcntr11_el0());
			__fallthrough;
		case 0x01:
			write_amu_grp1_ctx_reg(ctx, 0x0, read_amevcntr10_el0());
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
	amu_regs_t *ctx = &amu_ctx[core_pos];

	write_amevcntr00_el0(read_amu_grp0_ctx_reg(ctx, 0));
	write_amevcntr01_el0(read_amu_grp0_ctx_reg(ctx, 1));
	write_amevcntr02_el0(read_amu_grp0_ctx_reg(ctx, 2));
	write_amevcntr03_el0(read_amu_grp0_ctx_reg(ctx, 3));

	if (is_feat_amu_aux_supported()) {
		uint8_t num_counters = read_amcgcr_el0_cg1nc();

		switch (num_counters) {
		case 0x10:
			write_amevcntr1f_el0(read_amu_grp1_ctx_reg(ctx, 0xf));
			__fallthrough;
		case 0x0f:
			write_amevcntr1e_el0(read_amu_grp1_ctx_reg(ctx, 0xe));
			__fallthrough;
		case 0x0e:
			write_amevcntr1d_el0(read_amu_grp1_ctx_reg(ctx, 0xd));
			__fallthrough;
		case 0x0d:
			write_amevcntr1c_el0(read_amu_grp1_ctx_reg(ctx, 0xc));
			__fallthrough;
		case 0x0c:
			write_amevcntr1b_el0(read_amu_grp1_ctx_reg(ctx, 0xb));
			__fallthrough;
		case 0x0b:
			write_amevcntr1a_el0(read_amu_grp1_ctx_reg(ctx, 0xa));
			__fallthrough;
		case 0x0a:
			write_amevcntr19_el0(read_amu_grp1_ctx_reg(ctx, 0x9));
			__fallthrough;
		case 0x09:
			write_amevcntr18_el0(read_amu_grp1_ctx_reg(ctx, 0x8));
			__fallthrough;
		case 0x08:
			write_amevcntr17_el0(read_amu_grp1_ctx_reg(ctx, 0x7));
			__fallthrough;
		case 0x07:
			write_amevcntr16_el0(read_amu_grp1_ctx_reg(ctx, 0x6));
			__fallthrough;
		case 0x06:
			write_amevcntr15_el0(read_amu_grp1_ctx_reg(ctx, 0x5));
			__fallthrough;
		case 0x05:
			write_amevcntr14_el0(read_amu_grp1_ctx_reg(ctx, 0x4));
			__fallthrough;
		case 0x04:
			write_amevcntr13_el0(read_amu_grp1_ctx_reg(ctx, 0x3));
			__fallthrough;
		case 0x03:
			write_amevcntr12_el0(read_amu_grp1_ctx_reg(ctx, 0x2));
			__fallthrough;
		case 0x02:
			write_amevcntr11_el0(read_amu_grp1_ctx_reg(ctx, 0x1));
			__fallthrough;
		case 0x01:
			write_amevcntr10_el0(read_amu_grp1_ctx_reg(ctx, 0x0));
			__fallthrough;
		case 0x00:
			break;
		default:
			assert(0); /* something is wrong */
		}
	}


	/* now enable them again */
	write_amcntenset0_el0(AMCNTENSET0_EL0_Pn_MASK);
	if (is_feat_amu_aux_supported()) {
		write_amcntenset1_el0(get_amu_aux_enables(core_pos));
	}

	isb();
	return (void *)0;
}

SUBSCRIBE_TO_EVENT(psci_suspend_pwrdown_start, amu_context_save);
SUBSCRIBE_TO_EVENT(psci_suspend_pwrdown_finish, amu_context_restore);
