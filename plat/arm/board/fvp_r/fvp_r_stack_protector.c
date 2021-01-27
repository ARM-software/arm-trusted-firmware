/*
 * Copyright (c) 2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <fvp_r_arch_helpers.h>
#include <plat/common/platform.h>

#define RANDOM_CANARY_VALUE ((u_register_t) 8092347823957523895ULL)

u_register_t plat_get_stack_protector_canary(void)
{
	/*
	 * Ideally, a random number should be returned instead of the
	 * combination of a timer's value and a compile-time constant. As the
	 * FVP_R does not have any random number generator, this is better than
	 * nothing but not necessarily really secure.
	 */
	return RANDOM_CANARY_VALUE ^ read_cntpct_el0();
}

