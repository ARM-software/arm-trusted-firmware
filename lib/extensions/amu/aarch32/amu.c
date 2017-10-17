/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <amu.h>
#include <arch.h>
#include <arch_helpers.h>

void amu_enable(int el2_unused)
{
	uint64_t features;

	features = read_id_pfr0() >> ID_PFR0_AMU_SHIFT;
	if ((features & ID_PFR0_AMU_MASK) == 1) {
		if (el2_unused) {
			uint64_t v;

			/*
			 * Non-secure access from EL0 or EL1 to the Activity Monitor
			 * registers do not trap to EL2.
			 */
			v = read_hcptr();
			v &= ~TAM_BIT;
			write_hcptr(v);
		}

		/* Enable group 0 counters */
		write_amcntenset0(AMU_GROUP0_COUNTERS_MASK);
	}
}
