/*
 * Copyright (c) 2021-2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_features.h>
#include <arch_helpers.h>
#include <lib/extensions/trbe.h>

void trbe_enable(void)
{
	uint64_t val;

	if (is_feat_trbe_present()) {
		/*
		 * MDCR_EL3.NSTB = 0b11
		 * Allow access of trace buffer control registers from NS-EL1
		 * and NS-EL2, tracing is prohibited in Secure and Realm state
		 * (if implemented).
		 */
		val = read_mdcr_el3();
		val |= MDCR_NSTB(MDCR_NSTB_EL1);
		write_mdcr_el3(val);
	}
}

void trbe_disable(void)
{
	uint64_t val;

	if (is_feat_trbe_present()) {
		/*
		 * MDCR_EL3.NSTB = 0b11
		 * Disable access of trace buffer control registers from NS-EL1
		 * and NS-EL2, tracing is prohibited in Secure and Realm state
		 * (if implemented).
		 */
		val = read_mdcr_el3();
		val &= ~MDCR_NSTB(MDCR_NSTB_EL1);
		write_mdcr_el3(val);
	}
}
