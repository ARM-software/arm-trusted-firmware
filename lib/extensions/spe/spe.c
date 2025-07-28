/*
 * Copyright (c) 2017-2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>

#include <arch.h>
#include <arch_features.h>
#include <arch_helpers.h>
#include <lib/extensions/spe.h>

#include <plat/common/platform.h>

/*
 * SPE is an unusual feature. Its enable is split into two:
 *  - (NSPBE, NSPB[0]) - the security state bits - determines which security
 *    state owns the profiling buffer.
 *  - NSPB[1] - the enable bit - determines if the security state that owns the
 *    buffer may access SPE registers.
 *
 * There is a secondary id register PMBIDR_EL1 that is more granular than
 * ID_AA64DFR0_EL1. When a security state owns the buffer, PMBIDR_EL1.P will
 * report that SPE programming is allowed. This means that the usual assumption
 * that leaving all bits to a default of zero will disable the feature may not
 * work correctly. To correctly disable SPE, the current security state must NOT
 * own the buffer, irrespective of the enable bit. Then, to play nicely with
 * SMCCC_ARCH_FEATURE_AVAILABILITY, the enable bit should correspond to the
 * enable status. The feature is architected this way to allow for lazy context
 * switching of the buffer - a world can be made owner of the buffer (with
 * PMBIDR_EL1.P reporting full access) without giving it access to the registers
 * (by trapping to EL3). Then context switching can be deferred until a world
 * tries to use SPE at which point access can be given and the trapping
 * instruction repeated.
 *
 * This can be simplified to the following rules:
 * 1. To enable SPE for world X:
 *    * world X owns the buffer ((NSPBE, NSPB[0]) == SCR_EL3.{NSE, NS})
 *    * trapping disabled (NSPB[0] == 1)
 * 2. To disable SPE for world X:
 *    * world X does not own the buffer ((NSPBE, NSPB[0]) != SCR_EL3.{NSE, NS})
 *    * trapping enabled (NSPB[0] == 0)
 */

/*
 * MDCR_EL3.EnPMSN (ARM v8.7) and MDCR_EL3.EnPMS3: Do not trap access to
 * PMSNEVFR_EL1 or PMSDSFR_EL1 register at NS-EL1 or NS-EL2 to EL3 if
 * FEAT_SPEv1p2 or FEAT_SPE_FDS are implemented. Setting these bits to 1 doesn't
 * have any effect on it when the features aren't implemented.
 */
void spe_enable_ns(cpu_context_t *ctx)
{
	el3_state_t *state = get_el3state_ctx(ctx);
	u_register_t mdcr_el3_val = read_ctx_reg(state, CTX_MDCR_EL3);

	mdcr_el3_val |= MDCR_NSPB_EN_BIT | MDCR_NSPB_SS_BIT | MDCR_EnPMSN_BIT | MDCR_EnPMS3_BIT;
	mdcr_el3_val &= ~(MDCR_NSPBE_BIT);

	write_ctx_reg(state, CTX_MDCR_EL3, mdcr_el3_val);
}

/*
 * MDCR_EL3.EnPMSN (ARM v8.7) and MDCR_EL3.EnPMS3: Clear the bits to trap access
 * of PMSNEVFR_EL1 and PMSDSFR_EL1 from EL2/EL1 to EL3.
 */
static void spe_disable_others(cpu_context_t *ctx)
{
	el3_state_t *state = get_el3state_ctx(ctx);
	u_register_t mdcr_el3_val = read_ctx_reg(state, CTX_MDCR_EL3);

	mdcr_el3_val |= MDCR_NSPB_SS_BIT;
	mdcr_el3_val &= ~(MDCR_NSPB_EN_BIT | MDCR_NSPBE_BIT | MDCR_EnPMSN_BIT |
			  MDCR_EnPMS3_BIT);
	write_ctx_reg(state, CTX_MDCR_EL3, mdcr_el3_val);
}

void spe_disable_secure(cpu_context_t *ctx)
{
	spe_disable_others(ctx);
}

void spe_disable_realm(cpu_context_t *ctx)
{
	spe_disable_others(ctx);
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
