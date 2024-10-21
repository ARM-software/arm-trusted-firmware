/*
 * Copyright (c) 2022-2024, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_features.h>
#include <arch_helpers.h>
#include <lib/extensions/brbe.h>

void brbe_enable(cpu_context_t *ctx)
{
	el3_state_t *state = get_el3state_ctx(ctx);
	u_register_t mdcr_el3_val = read_ctx_reg(state, CTX_MDCR_EL3);

	/*
	 * MDCR_EL3.SBRBE = 0b01
	 * Allows BRBE usage in non-secure world and prohibited in
	 * secure world.
	 *
	 * MDCR_EL3.{E3BREW, E3BREC} = 0b00
	 * Branch recording at EL3 is disabled
	 */
	mdcr_el3_val &= ~((MDCR_SBRBE_MASK << MDCR_SBRBE_SHIFT) | MDCR_E3BREW | MDCR_E3BREC);
	mdcr_el3_val |= (0x1UL << MDCR_SBRBE_SHIFT);
	write_ctx_reg(state, CTX_MDCR_EL3, mdcr_el3_val);
}
