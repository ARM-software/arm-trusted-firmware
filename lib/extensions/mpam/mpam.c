/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_helpers.h>
#include <mpam.h>
#include <stdbool.h>

bool mpam_supported(void)
{
	uint64_t features = read_id_aa64dfr0_el1() >> ID_AA64PFR0_MPAM_SHIFT;

	return ((features & ID_AA64PFR0_MPAM_MASK) != 0U);
}

void mpam_enable(int el2_unused)
{
	if (!mpam_supported())
		return;

	/*
	 * Enable MPAM, and disable trapping to EL3 when lower ELs access their
	 * own MPAM registers.
	 */
	write_mpam3_el3(MPAM3_EL3_MPAMEN_BIT);

	/*
	 * If EL2 is implemented but unused, disable trapping to EL2 when lower
	 * ELs access their own MPAM registers.
	 */
	if (el2_unused != 0) {
		write_mpam2_el2(0);

		if ((read_mpamidr_el1() & MPAMIDR_HAS_HCR_BIT) != 0U)
			write_mpamhcr_el2(0);
	}
}
