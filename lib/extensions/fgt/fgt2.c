/*
 * Copyright (c) 2024, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_features.h>
#include <arch_helpers.h>
#include <lib/extensions/fgt2.h>

void fgt2_enable(cpu_context_t *context)
{
	u_register_t reg;
	el3_state_t *state;

	state = get_el3state_ctx(context);

	/* Set the FGTEN2 bit in SCR_EL3 to enable access to HFGITR2_EL2,
	 * HFGRTR2_EL2, HFGWTR_EL2, HDFGRTR2_EL2, and HDFGWTR2_EL2.
	 */

	reg = read_ctx_reg(state, CTX_SCR_EL3);
	reg |= SCR_FGTEN2_BIT;
	write_ctx_reg(state, CTX_SCR_EL3, reg);
}

