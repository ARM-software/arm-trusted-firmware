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

	features = read_id_aa64pfr0_el1() >> ID_AA64PFR0_AMU_SHIFT;
	if ((features & ID_AA64PFR0_AMU_MASK) == 1) {
		uint64_t v;

		if (el2_unused) {
			/*
			 * CPTR_EL2.TAM: Set to zero so any accesses to
			 * the Activity Monitor registers do not trap to EL2.
			 */
			v = read_cptr_el2();
			v &= ~CPTR_EL2_TAM_BIT;
			write_cptr_el2(v);
		}

		/*
		 * CPTR_EL3.TAM: Set to zero so that any accesses to
		 * the Activity Monitor registers do not trap to EL3.
		 */
		v = read_cptr_el3();
		v &= ~TAM_BIT;
		write_cptr_el3(v);

		/* Enable group 0 counters */
		write_amcntenset0_el0(AMU_GROUP0_COUNTERS_MASK);
		/* Enable group 1 counters */
		write_amcntenset1_el0(AMU_GROUP1_COUNTERS_MASK);
	}
}
