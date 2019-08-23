/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include "gxbb_private.h"

static int32_t modules_initialized = -1;

/*******************************************************************************
 * Unknown commands related to something thermal-related
 ******************************************************************************/
void gxbb_thermal_unknown(void)
{
	uint16_t ret;

	if (modules_initialized == -1) {
		scpi_efuse_read(&ret, 0, 2);
		modules_initialized = ret;
	}

	scpi_unknown_thermal(10, 2,  /* thermal */
			     13, 1); /* thermalver */
}
