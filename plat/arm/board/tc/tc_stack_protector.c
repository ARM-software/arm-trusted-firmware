/*
 * Copyright (c) 2024, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <arch_helpers.h>
#include <plat/common/platform.h>

#define RANDOM_CANARY_VALUE ((u_register_t) 3288484550995823360ULL)

u_register_t plat_get_stack_protector_canary(void)
{
	/*
	 * On the Total Compute platform, it can generate RNG via MHU channel
	 * and communicate with RSE. But the stack protector canary function
	 * is needed prior to MHU channel gets ready.
	 *
	 * Since now MHU module cannot distinguish if MHU channel has been
	 * initialized or not, if it arbitrarily tries to send message, it will
	 * cause panic. For this reason, this function cannot rollback to
	 * dummy random number based on the MHU failure.
	 *
	 * For above reasons, simply return a value of the combination of a
	 * timer's value and a compile-time constant.
	 */
	return RANDOM_CANARY_VALUE ^ read_cntpct_el0();
}
