/*
 * Copyright (c) 2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <plat/common/platform.h>

int plat_set_drtm_error(uint64_t error_code)
{
	/* TODO: Set DRTM error in NV-storage */
	return 0;
}

int plat_get_drtm_error(uint64_t *error_code)
{
	/* TODO: Get DRTM error from NV-storage */
	*error_code = 0;
	return 0;
}
