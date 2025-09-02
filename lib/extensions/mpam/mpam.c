/*
 * Copyright (c) 2018-2025, Arm Limited and Contributors. All rights reserved.
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

void mpam_init_el3(void)
{
	if (is_feat_mpam_pe_bw_ctrl_supported()) {
		u_register_t mpambw3_el3;

		/*
		 * Disables EL3 trapping of MPAM PE-side bandwidth
		 * control registers - MPAMBW2_EL2, MPAMBWCAP_EL2,
		 * MPAMBW1_EL1, MPAMBW0_EL1, MPAMBWSM_EL1,
		 * MPAMBWIDR_EL1.
		 */
		mpambw3_el3 = read_mpambw3_el3()	|
			MPAMBW3_EL3_NTRAPLOWER_BIT;

		/*
		 * Disable use of MPAM PE side bandwidth controls in EL3
		 * since we don't use it in EL3.
		 */
		mpambw3_el3 &= ~(MPAMBW3_EL3_HW_SCALE_ENABLE_BIT	|
				MPAMBW3_EL3_ENABLED_BIT			|
				MPAMBW3_EL3_HARDLIM_BIT);

		write_mpambw3_el3(mpambw3_el3);
	}
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

	if (is_feat_mpam_pe_bw_ctrl_supported()) {
		u_register_t mpambw2_el2;

		/*
		 * Configure MPAMBW2_EL2 to allow EL1 direct access to
		 * specific MPAM bandwidth registers (MPAMBWIDR_EL1,
		 * MPAMBW0_EL1, MPAMBW1_EL1, MPAMBWSM_EL1)
		 * by disabling traps for these accesses.
		 */
		mpambw2_el2 = read_mpambw2_el2()			|
				MPAMBW2_EL2_NTRAP_MPAMBWIDR_EL1_BIT	|
				MPAMBW2_EL2_NTRAP_MPAMBW0_EL1_BIT	|
				MPAMBW2_EL2_NTRAP_MPAMBW1_EL1_BIT	|
				MPAMBW2_EL2_NTRAP_MPAMBWSM_EL1_BIT;
		/*
		 * MPAM PE side bandwidth controls are disabled in EL2
		 * since NS-EL2 is unused.
		 */
		mpambw2_el2 &= (MPAMBW2_EL2_HW_SCALE_ENABLE_BIT		|
				MPAMBW2_EL2_ENABLED_BIT			|
				MPAMBW2_EL2_HARDLIM_BIT);

		write_mpambw2_el2(mpambw2_el2);
	}
}
