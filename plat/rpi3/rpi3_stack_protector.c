/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <lib/utils.h>

#include "rpi3_private.h"

/* Get 128 bits of entropy and fuse the values together to form the canary. */
#define TRNG_NBYTES	16U

u_register_t plat_get_stack_protector_canary(void)
{
	size_t i;
	u_register_t buf[TRNG_NBYTES / sizeof(u_register_t)];
	u_register_t ret = 0U;

	rpi3_rng_read(buf, sizeof(buf));

	for (i = 0U; i < ARRAY_SIZE(buf); i++)
		ret ^= buf[i];

	return ret;
}
