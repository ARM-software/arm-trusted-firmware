/*
 * Copyright (c) 2018-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>

#include <arch.h>
#include <arch_features.h>
#include <arch_helpers.h>
#include <lib/extensions/mpam.h>

void mpam_enable_per_world(per_world_context_t *per_world_ctx)
{
	u_register_t mpam3_el3;

	/*
	 * Enable MPAM, and disable trapping to EL3 when lower ELs access their
	 * own MPAM registers
	 */
	mpam3_el3 = per_world_ctx->ctx_mpam3_el3;
	mpam3_el3 = (mpam3_el3 | MPAM3_EL3_MPAMEN_BIT) &
				~(MPAM3_EL3_TRAPLOWER_BIT);

	per_world_ctx->ctx_mpam3_el3 = mpam3_el3;
}

/*
 * If EL2 is implemented but unused, disable trapping to EL2 when lower ELs
 * access their own MPAM registers.
 */
void mpam_init_el2_unused(void)
{
	write_mpam2_el2(0ULL);

	if ((read_mpamidr_el1() & MPAMIDR_HAS_HCR_BIT) != 0U) {
		write_mpamhcr_el2(0ULL);
	}

}
