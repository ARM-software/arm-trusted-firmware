/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>

#include <arch.h>
#include <arch_helpers.h>
#include <lib/extensions/trf.h>

static bool trf_supported(void)
{
	uint64_t features;

	features = read_id_aa64dfr0_el1() >> ID_AA64DFR0_TRACEFILT_SHIFT;
	return ((features & ID_AA64DFR0_TRACEFILT_MASK) ==
		ID_AA64DFR0_TRACEFILT_SUPPORTED);
}

void trf_enable(void)
{
	uint64_t val;

	if (trf_supported()) {
		/*
		 * MDCR_EL3.TTRF = b0
		 * Allow access of trace filter control registers from NS-EL2
		 * and NS-EL1 when NS-EL2 is implemented but not used
		 */
		val = read_mdcr_el3();
		val &= ~MDCR_TTRF_BIT;
		write_mdcr_el3(val);
	}
}
