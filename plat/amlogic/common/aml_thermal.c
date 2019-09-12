/*
 * Copyright (c) 2018-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include "aml_private.h"

static int32_t modules_initialized = -1;

/*******************************************************************************
 * Unknown commands related to something thermal-related
 ******************************************************************************/
void aml_thermal_unknown(void)
{
	uint16_t ret;

	if (modules_initialized == -1) {
		aml_scpi_efuse_read(&ret, 0, 2);
		modules_initialized = ret;
	}

	aml_scpi_unknown_thermal(10, 2,  /* thermal */
				 13, 1); /* thermalver */
}
