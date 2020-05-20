/*
 * Copyright (c) 2020, NVIDIA Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <arch_helpers.h>
#include <lib/mmio.h>
#include <plat/common/platform.h>
#include <platform_def.h>

u_register_t plat_get_stack_protector_canary(void)
{
	u_register_t seed;

	/*
	 * Ideally, a random number should be returned instead. As the
	 * platform does not have any random number generator, this is
	 * better than nothing, but not really secure.
	 */
	seed = mmio_read_32(TEGRA_MISC_BASE + HARDWARE_REVISION_OFFSET);
	seed <<= 32;
	seed |= mmio_read_32(TEGRA_TMRUS_BASE);

	return seed ^ read_cntpct_el0();
}
