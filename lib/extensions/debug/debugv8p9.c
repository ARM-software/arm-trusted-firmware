/*
 * Copyright (c) 2024, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_features.h>
#include <arch_helpers.h>
#include <lib/extensions/debug_v8p9.h>

void debugv8p9_extended_bp_wp_enable(cpu_context_t *ctx)
{
	el3_state_t *state = get_el3state_ctx(ctx);
	u_register_t mdcr_el3_val = read_ctx_reg(state, CTX_MDCR_EL3);

	/* When FEAT_Debugv8p9 is implemented:
	 *
	 * MDCR_EL3.EBWE: Set to 0b1
	 * Enables use of additional breakpoints or watchpoints,
	 * and disables trap to EL3 on accesses to debug register.
	 */

	mdcr_el3_val |= MDCR_EBWE_BIT;
	write_ctx_reg(state, CTX_MDCR_EL3, mdcr_el3_val);
}
