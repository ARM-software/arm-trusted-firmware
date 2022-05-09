/*
 * Copyright (c) 2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_features.h>
#include <arch_helpers.h>

void brbe_enable(void)
{
	uint64_t val;

	if (is_feat_brbe_present()) {
		/*
		 * MDCR_EL3.SBRBE = 0b01
		 *
		 * Allows BRBE usage in non-secure world and prohibited in
		 * secure world.
		 */
		val = read_mdcr_el3();
		val &= ~(MDCR_SBRBE_MASK << MDCR_SBRBE_SHIFT);
		val |= (0x1UL << MDCR_SBRBE_SHIFT);
		write_mdcr_el3(val);
	}
}
