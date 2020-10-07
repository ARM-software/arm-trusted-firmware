/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <common/debug.h>
#include <lib/utils.h>
#include <plat/common/plat_trng.h>
#include <platform_def.h>

u_register_t plat_get_stack_protector_canary(void)
{
	uint64_t entropy;

	if (!plat_get_entropy(&entropy)) {
		ERROR("Not enough entropy to initialize canary value\n");
		panic();
	}

	if (sizeof(entropy) == sizeof(u_register_t)) {
		return entropy;
	}

	return (entropy & 0xffffffffULL) ^ (entropy >> 32);
}
