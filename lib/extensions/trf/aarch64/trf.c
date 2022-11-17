/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_features.h>
#include <arch_helpers.h>
#include <lib/extensions/trf.h>

void trf_enable(void)
{
	uint64_t val;

	/*
	 * MDCR_EL3.TTRF = b0
	 * Allow access of trace filter control registers from NS-EL2
	 * and NS-EL1 when NS-EL2 is implemented but not used
	 */
	val = read_mdcr_el3();
	val &= ~MDCR_TTRF_BIT;
	write_mdcr_el3(val);
}
