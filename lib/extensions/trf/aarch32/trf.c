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
	uint32_t features;

	features = read_id_dfr0() >> ID_DFR0_TRACEFILT_SHIFT;
	return ((features & ID_DFR0_TRACEFILT_MASK) ==
		ID_DFR0_TRACEFILT_SUPPORTED);
}

void trf_enable(void)
{
	uint32_t val;

	if (trf_supported()) {
		/*
		 * Allow access of trace filter control registers from
		 * non-monitor mode
		 */
		val = read_sdcr();
		val &= ~SDCR_TTRF_BIT;
		write_sdcr(val);
	}
}
