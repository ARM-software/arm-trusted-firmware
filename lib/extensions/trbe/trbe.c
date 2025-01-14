/*
 * Copyright (c) 2021-2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_features.h>
#include <arch_helpers.h>
#include <lib/extensions/trbe.h>

void trbe_enable(cpu_context_t *ctx)
{
	el3_state_t *state = get_el3state_ctx(ctx);
	u_register_t mdcr_el3_val = read_ctx_reg(state, CTX_MDCR_EL3);

	/*
	 * MDCR_EL3.NSTBE = 0b0
	 *  Trace Buffer owning Security state is Non-secure state. If FEAT_RME
	 *  is not implemented, this field is RES0.
	 *
	 * MDCR_EL3.NSTB = 0b11
	 *  Allow access of trace buffer control registers from NS-EL1 and
	 *  NS-EL2, tracing is prohibited in Secure and Realm state (if
	 *  implemented).
	 */
	mdcr_el3_val |= MDCR_NSTB(MDCR_NSTB_EL1);
	mdcr_el3_val &= ~(MDCR_NSTBE_BIT);
	write_ctx_reg(state, CTX_MDCR_EL3, mdcr_el3_val);
}

void trbe_disable(cpu_context_t *ctx)
{
	el3_state_t *state = get_el3state_ctx(ctx);
	u_register_t mdcr_el3_val = read_ctx_reg(state, CTX_MDCR_EL3);

	/*
	 * MDCR_EL3.{NSTBE,NSTB} = 0b0, 0b00
	 *  Disable access of trace buffer control registers from lower ELs in
	 *  any security state. Secure state owns the buffer.
	 */
	mdcr_el3_val &= ~(MDCR_NSTB(MDCR_NSTB_EL1));
	mdcr_el3_val &= ~(MDCR_NSTBE_BIT);
	write_ctx_reg(state, CTX_MDCR_EL3, mdcr_el3_val);
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
