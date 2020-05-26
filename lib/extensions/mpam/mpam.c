/*
 * Copyright (c) 2018-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>

#include <arch.h>
#include <arch_features.h>
#include <arch_helpers.h>
#include <lib/extensions/mpam.h>

void mpam_enable(bool el2_unused)
{
	/* Check if MPAM is implemented */
	if (get_mpam_version() == 0U) {
		return;
	}

	/*
	 * Enable MPAM, and disable trapping to EL3 when lower ELs access their
	 * own MPAM registers.
	 */
	write_mpam3_el3(MPAM3_EL3_MPAMEN_BIT);

	/*
	 * If EL2 is implemented but unused, disable trapping to EL2 when lower
	 * ELs access their own MPAM registers.
	 * If EL2 is implemented and used, enable trapping to EL2.
	 */
	if (el2_unused) {
		write_mpam2_el2(0ULL);

		if ((read_mpamidr_el1() & MPAMIDR_HAS_HCR_BIT) != 0U) {
			write_mpamhcr_el2(0ULL);
		}
	} else {
		write_mpam2_el2(MPAM2_EL2_TRAPMPAM0EL1 |
				MPAM2_EL2_TRAPMPAM1EL1);

		if ((read_mpamidr_el1() & MPAMIDR_HAS_HCR_BIT) != 0U) {
			write_mpamhcr_el2(MPAMHCR_EL2_TRAP_MPAMIDR_EL1);
		}
	}
}
