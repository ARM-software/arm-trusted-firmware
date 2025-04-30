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
	 * MDCR_EL3.EnPMSN (ARM v8.7) and MDCR_EL3.EnPMS3: Do not trap access to
	 * PMSNEVFR_EL1 or PMSDSFR_EL1 register at NS-EL1 or NS-EL2 to EL3 if FEAT_SPEv1p2
	 * or FEAT_SPE_FDS are implemented. Setting these bits to 1 doesn't have any
	 * effect on it when the features aren't implemented.
	 */
	mdcr_el3_val |= MDCR_NSPB(MDCR_NSPB_EL1) | MDCR_EnPMSN_BIT | MDCR_EnPMS3_BIT;
	mdcr_el3_val &= ~(MDCR_NSPBE_BIT);
	write_ctx_reg(state, CTX_MDCR_EL3, mdcr_el3_val);
}

void spe_disable(cpu_context_t *ctx)
{
	el3_state_t *state = get_el3state_ctx(ctx);
	u_register_t mdcr_el3_val = read_ctx_reg(state, CTX_MDCR_EL3);

	/*
	 * MDCR_EL3.{NSPB,NSPBE} = 0b00, 0b0
	 *  Disable access of profiling buffer control registers from lower ELs
	 *  in any security state. Secure state owns the buffer.
	 *
	 * MDCR_EL3.EnPMSN (ARM v8.7) and MDCR_EL3.EnPMS3: Clear the bits to trap access
	 * of PMSNEVFR_EL1 and PMSDSFR_EL1 from EL2/EL1 to EL3.
	 */
	mdcr_el3_val &= ~(MDCR_NSPB(MDCR_NSPB_EL1) | MDCR_NSPBE_BIT | MDCR_EnPMSN_BIT |
			  MDCR_EnPMS3_BIT);
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
