/*
 * Copyright (c) 2024, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_features.h>
#include <arch_helpers.h>
#include <lib/extensions/tcr2.h>

void tcr2_enable(cpu_context_t *ctx)
{
	u_register_t reg;
	el3_state_t *state;

	state = get_el3state_ctx(ctx);

	/* Set the TCR2EN bit in SCR_EL3 to enable access to TCR2_EL1,
	 * and TCR2_EL2 registers .
	 */

	reg = read_ctx_reg(state, CTX_SCR_EL3);
	reg |= SCR_TCR2EN_BIT;
	write_ctx_reg(state, CTX_SCR_EL3, reg);
}

void tcr2_disable(cpu_context_t *ctx)
{
	u_register_t reg;
	el3_state_t *state;

	state = get_el3state_ctx(ctx);

	/* Clear the TCR2EN bit in SCR_EL3 to disable access to TCR2_EL1,
	 * and TCR2_EL2 registers .
	 */

	reg = read_ctx_reg(state, CTX_SCR_EL3);
	reg &= ~SCR_TCR2EN_BIT;
	write_ctx_reg(state, CTX_SCR_EL3, reg);
}
