/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _SOC_TZASC_H_
#define _SOC_TZASC_H_

#include "tzc380.h"

#define MAX_NUM_TZC_REGION	3

/* List of MAX_NUM_TZC_REGION TZC regions' boundaries and configurations. */

static const struct tzc380_reg tzc380_reg_list[] = {
	{
		TZASC_REGION_SECURITY_NSRW,	/* .secure attr */
		0x0,			/* .enabled */
		0x0,			/* .lowaddr */
		0x0,			/* .highaddr */
		0x0,			/* .size */
		0x0,			/* .submask */
	},
	{
		TZASC_REGION_SECURITY_SRW,
		TZASC_REGION_ENABLED,
		0xFC000000,
		0x0,
		TZASC_REGION_SIZE_64MB,
		0x80,			/* Disable region 7 */
	},
	/* reserve 2M non-scure memory for OPTEE public memory */
	{
		TZASC_REGION_SECURITY_SRW,
		TZASC_REGION_ENABLED,
		0xFF800000,
		0x0,
		TZASC_REGION_SIZE_8MB,
		0xC0,			/* Disable region 6 & 7 */
	},

	{}
};

#endif /* _SOC_TZASC_H_ */
