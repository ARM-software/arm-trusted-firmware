/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <common/debug.h>
#include <lib/utils.h>
#include <platform_def.h>

#include "juno_decl.h"

u_register_t plat_get_stack_protector_canary(void)
{
	uint64_t entropy;

	if (!juno_getentropy(&entropy)) {
		ERROR("Not enough entropy to initialize canary value\n");
		panic();
	}

	if (sizeof(entropy) == sizeof(u_register_t)) {
		return entropy;
	}

	return (entropy & 0xffffffffULL) ^ (entropy >> 32);
}
