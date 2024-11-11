/*
 * Copyright (c) 2024, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_features.h>
#include <arch_helpers.h>
#include <lib/extensions/fpmr.h>

void fpmr_enable_per_world(per_world_context_t *per_world_ctx)
{
	u_register_t reg;

	/* Disable Floating point Trap in CPTR_EL3. */
	reg = per_world_ctx->ctx_cptr_el3;
	reg &= ~TFP_BIT;
	per_world_ctx->ctx_cptr_el3 = reg;
}
