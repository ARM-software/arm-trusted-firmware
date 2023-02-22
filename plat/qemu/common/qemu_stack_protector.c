/*
 * Copyright (c) 2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <arch_helpers.h>
#include <arch_features.h>
#include <plat/common/platform.h>

#define RANDOM_CANARY_VALUE ((u_register_t) 3288484550995823360ULL)

u_register_t plat_get_stack_protector_canary(void)
{
	/* Use the RNDR instruction if the CPU supports it */
	if (is_feat_rng_supported()) {
		return read_rndr();
	}

	/*
	 * Ideally, a random number should be returned above. If a random
	 * number generator is not supported, return instead a
	 * combination of a timer's value and a compile-time constant.
	 * This is better than nothing but not necessarily really secure.
	 */
	return RANDOM_CANARY_VALUE ^ read_cntpct_el0();
}

