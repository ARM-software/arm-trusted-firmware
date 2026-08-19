/*
 * Copyright (c) 2026, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <bl31/sync_handle.h>
#include <context.h>
#include <plat/common/plat_trng.h>

int plat_handle_rng_trap(u_register_t *data, bool rndrrs)
{
	uint64_t entropy;

	if (!plat_get_entropy(&entropy)) {
		ERROR("Failed to get entropy\n");
		return TRAP_RET_UNHANDLED;
	}

	*data = entropy;

	return TRAP_RET_CONTINUE;
}
