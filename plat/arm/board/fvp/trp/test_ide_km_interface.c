/*
 * Copyright (c) 2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <plat/common/platform.h>
#include <services/trp/platform_trp.h>

#include <platform_def.h>

/*
 * Helper function for ecam address and root port ID
 *
 */
uint64_t trp_get_test_rootport(uint64_t *ecam_address, uint64_t *rp_id)
{

	*ecam_address = 0xE001C000;
	*rp_id = 0x001C0001;

	return 0;
}

