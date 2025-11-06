/*
 * Copyright (c) 2018-2020, The Linux Foundation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdint.h>

#include <drivers/qti/crypto/rng.h>
#include <platform.h>

#include <qtiseclib_interface.h>

u_register_t plat_get_stack_protector_canary(void)
{
	u_register_t random = 0x0;

	/*
	 * get random data , the below API doesn't return random = 0 on success
	 */
	qti_rng_get_data((uint8_t *) &random, sizeof(random));
	assert(random != 0x0);

	return random;
}
