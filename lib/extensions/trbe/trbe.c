/*
 * Copyright (c) 2021-2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_features.h>
#include <arch_helpers.h>
#include <lib/extensions/trbe.h>


/*
 * TRBE is an unusual feature. Its enable is split into two:
 *  - (NSTBE, NSTB[0]) - the security state bits - determines which security
 *    state owns the trace buffer.
 *  - NSTB[1] - the enable bit - determines if the security state that owns the
 *    buffer may access TRBE registers.
 *
 * There is a secondary id register TRBIDR_EL1 that is more granular than
 * ID_AA64DFR0_EL1. When a security state owns the buffer, TRBIDR_EL1.P will
 * report that TRBE programming is allowed. This means that the usual assumption
 * that leaving all bits to a default of zero will disable the feature may not
 * work correctly. To correctly disable TRBE, the current security state must NOT
 * own the buffer, irrespective of the enable bit. Then, to play nicely with
 * SMCCC_ARCH_FEATURE_AVAILABILITY, the enable bit should correspond to the
 * enable status. The feature is architected this way to allow for lazy context
 * switching of the buffer - a world can be made owner of the buffer (with
 * TRBIDR_EL1.P reporting full access) without giving it access to the registers
 * (by trapping to EL3). Then context switching can be deferred until a world
 * tries to use TRBE at which point access can be given and the trapping
 * instruction repeated.
 *
 * This can be simplified to the following rules:
 * 1. To enable TRBE for world X:
 *    * world X owns the buffer ((NSTBE, NSTB[0]) == SCR_EL3.{NSE, NS})
 *    * trapping disabled (NSTB[0] == 1)
 * 2. To disable TRBE for world X:
 *    * world X does not own the buffer ((NSTBE, NSTB[0]) != SCR_EL3.{NSE, NS})
 *    * trapping enabled (NSTB[0] == 0)
 */
void trbe_enable_ns(cpu_context_t *ctx)
{
	el3_state_t *state = get_el3state_ctx(ctx);
	u_register_t mdcr_el3_val = read_ctx_reg(state, CTX_MDCR_EL3);

	mdcr_el3_val |= MDCR_NSTB_EN_BIT | MDCR_NSTB_SS_BIT;
	mdcr_el3_val &= ~(MDCR_NSTBE_BIT);

	write_ctx_reg(state, CTX_MDCR_EL3, mdcr_el3_val);
}

static void trbe_disable_all(cpu_context_t *ctx, bool ns)
{
	el3_state_t *state = get_el3state_ctx(ctx);
	u_register_t mdcr_el3_val = read_ctx_reg(state, CTX_MDCR_EL3);

	mdcr_el3_val &= ~MDCR_NSTB_EN_BIT;
	mdcr_el3_val &= ~MDCR_NSTBE_BIT;

	/* make NS owner, except when NS is running */
	if (ns) {
		mdcr_el3_val &= ~MDCR_NSTB_SS_BIT;
	} else {
		mdcr_el3_val |= MDCR_NSTB_SS_BIT;
	}

	write_ctx_reg(state, CTX_MDCR_EL3, mdcr_el3_val);
}


void trbe_disable_ns(cpu_context_t *ctx)
{
	trbe_disable_all(ctx, true);
}

void trbe_disable_secure(cpu_context_t *ctx)
{
	trbe_disable_all(ctx, false);
}

void trbe_disable_realm(cpu_context_t *ctx)
{
	trbe_disable_all(ctx, false);
}


void trbe_init_el2_unused(void)
{
	/*
	 * MDCR_EL2.E2TB: Set to zero so that the trace Buffer
	 *  owning exception level is NS-EL1 and, tracing is
	 *  prohibited at NS-EL2. These bits are RES0 when
	 *  FEAT_TRBE is not implemented.
	 */
	write_mdcr_el2(read_mdcr_el2() & ~MDCR_EL2_E2TB(MDCR_EL2_E2TB_EL1));
}
