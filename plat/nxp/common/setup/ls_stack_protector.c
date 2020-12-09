/*
 * Copyright 2018-2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <stdint.h>

#include <arch_helpers.h>

#include <plat/common/platform.h>

#define RANDOM_CANARY_VALUE ((u_register_t) 3288484550995823360ULL)

u_register_t plat_get_stack_protector_canary(void)
{
	/*
	 * TBD: Generate Random Number from NXP CAAM Block.
	 */
	return RANDOM_CANARY_VALUE ^ read_cntpct_el0();
}
