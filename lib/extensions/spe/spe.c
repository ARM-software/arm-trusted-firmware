/*
 * Copyright (c) 2017-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>

#include <arch.h>
#include <arch_features.h>
#include <arch_helpers.h>
#include <lib/el3_runtime/pubsub.h>
#include <lib/extensions/spe.h>

#include <plat/common/platform.h>

typedef struct spe_ctx {
	u_register_t pmblimitr_el1;
} spe_ctx_t;

static struct spe_ctx spe_ctxs[PLATFORM_CORE_COUNT];

static inline void psb_csync(void)
{
	/*
	 * The assembler does not yet understand the psb csync mnemonic
	 * so use the equivalent hint instruction.
	 */
	__asm__ volatile("hint #17");
}

void spe_enable(cpu_context_t *ctx)
{
	el3_state_t *state = get_el3state_ctx(ctx);
	u_register_t mdcr_el3_val = read_ctx_reg(state, CTX_MDCR_EL3);

	/*
	 * MDCR_EL3.NSPB (ARM v8.2): SPE enabled in Non-secure state
	 * and disabled in secure state. Accesses to SPE registers at
	 * S-EL1 generate trap exceptions to EL3.
	 *
	 * MDCR_EL3.NSPBE: Profiling Buffer uses Non-secure Virtual Addresses.
	 * When FEAT_RME is not implemented, this field is RES0.
	 *
	 * MDCR_EL3.EnPMSN (ARM v8.7): Do not trap access to PMSNEVFR_EL1
	 * register at NS-EL1 or NS-EL2 to EL3 if FEAT_SPEv1p2 is implemented.
	 * Setting this bit to 1 doesn't have any effect on it when
	 * FEAT_SPEv1p2 not implemented.
	 */
	mdcr_el3_val |= MDCR_NSPB(MDCR_NSPB_EL1) | MDCR_EnPMSN_BIT;
	mdcr_el3_val &= ~(MDCR_NSPBE_BIT);
	write_ctx_reg(state, CTX_MDCR_EL3, mdcr_el3_val);
}

void spe_disable(cpu_context_t *ctx)
{
	el3_state_t *state = get_el3state_ctx(ctx);
	u_register_t mdcr_el3_val = read_ctx_reg(state, CTX_MDCR_EL3);

	/*
	 * MDCR_EL3.NSPB: Clear these bits to disable SPE feature, as it was enabled
	 * for Non-secure state only. After clearing these bits Secure state owns
	 * the Profiling Buffer and accesses to Statistical Profiling and Profiling
	 * Buffer control registers at EL2 and EL1 generate Trap exceptions to EL3
	 *
	 * MDCR_EL3.NSPBE: Don't care as it was cleared during spe_enable and setting
	 * this to 1 does not make sense as NSPBE{1} and NSPB{0b0x} is RESERVED.
	 *
	 * MDCR_EL3.EnPMSN (ARM v8.7): Clear the bit to trap access of PMSNEVFR_EL1
	 * from EL2/EL1 to EL3.
	 */
	mdcr_el3_val &= ~(MDCR_NSPB(MDCR_NSPB_EL1) | MDCR_EnPMSN_BIT);
	write_ctx_reg(state, CTX_MDCR_EL3, mdcr_el3_val);
}

void spe_init_el2_unused(void)
{
	uint64_t v;

	/*
	 * MDCR_EL2.TPMS (ARM v8.2): Do not trap statistical
	 * profiling controls to EL2.
	 *
	 * MDCR_EL2.E2PB (ARM v8.2): SPE enabled in Non-secure
	 * state. Accesses to profiling buffer controls at
	 * Non-secure EL1 are not trapped to EL2.
	 */
	v = read_mdcr_el2();
	v &= ~MDCR_EL2_TPMS;
	v |= MDCR_EL2_E2PB(MDCR_EL2_E2PB_EL1);
	write_mdcr_el2(v);
}

void spe_stop(void)
{
	uint64_t v;

	/* Drain buffered data */
	psb_csync();
	dsbnsh();

	/* Disable profiling buffer */
	v = read_pmblimitr_el1();
	v &= ~(1ULL << 0);
	write_pmblimitr_el1(v);
	isb();
}

static void *spe_drain_buffers_hook(const void *arg)
{
	if (!is_feat_spe_supported())
		return (void *)-1;

	/* Drain buffered data */
	psb_csync();
	dsbnsh();

	return (void *)0;
}

static void *spe_context_save(const void *arg)
{
	unsigned int core_pos;
	struct spe_ctx *ctx;

	if (is_feat_spe_supported()) {
		core_pos = plat_my_core_pos();
		ctx = &spe_ctxs[core_pos];
		ctx->pmblimitr_el1 = read_pmblimitr_el1();
	}

	return NULL;
}

static void *spe_context_restore(const void *arg)
{
	unsigned int core_pos;
	struct spe_ctx *ctx;

	if (is_feat_spe_supported()) {
		core_pos = plat_my_core_pos();
		ctx = &spe_ctxs[core_pos];
		write_pmblimitr_el1(ctx->pmblimitr_el1);
	}

	return NULL;
}

SUBSCRIBE_TO_EVENT(cm_entering_secure_world, spe_drain_buffers_hook);

SUBSCRIBE_TO_EVENT(psci_suspend_pwrdown_start, spe_context_save);
SUBSCRIBE_TO_EVENT(psci_suspend_pwrdown_finish, spe_context_restore);
