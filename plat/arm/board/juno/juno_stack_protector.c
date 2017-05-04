/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <debug.h>
#include <utils.h>
#include "juno_decl.h"
#include "juno_def.h"

u_register_t plat_get_stack_protector_canary(void)
{
	u_register_t c[TRNG_NBYTES / sizeof(u_register_t)];
	u_register_t ret = 0;
	size_t i;

	if (juno_getentropy(c, sizeof(c)) != 0) {
		ERROR("Not enough entropy to initialize canary value\n");
		panic();
	}

	/*
	 * On Juno we get 128-bits of entropy in one round.
	 * Fuse the values together to form the canary.
	 */
	for (i = 0; i < ARRAY_SIZE(c); i++)
		ret ^= c[i];
	return ret;
}
