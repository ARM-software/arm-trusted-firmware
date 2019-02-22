/*
 * Copyright 2018-2019 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef PLAT_TZC380_H
#define PLAT_TZC380_H

#include <drivers/arm/tzc380.h>

#define TZC380_REGION_GUARD_SIZE	(66 * 1024 * 1024)

/* Four Region:
 *	Region 0: Default region marked as Non-Secure.
 *	Region 1: Secure Region on DRAM 1 for  2MB out of  2MB,
 *			excluding 0 sub-region(=256KB).
 *	Region 2: Secure Region on DRAM 1 for 54MB out of 64MB,
 *			excluding 1 sub-rgion(=8MB) of 8MB
 *	Region 3: Secure Region on DRAM 1 for  6MB out of  8MB,
 *			excluding 2 sub-rgion(=1MB) of 2MB
 *
 *      For TZC-380 MAX_NUM_TZC_REGION will remain = 4.
 *
 * Note: No need to confifure Non-Secure region as it falls in region-0.
 */
#define MAX_NUM_TZC_REGION	4

struct tzc380_reg {
	unsigned int secure;
	unsigned int enabled;
	uintptr_t addr;
	unsigned int size;
	unsigned int sub_mask;
};


/* List of MAX_NUM_TZC_REGION TZC regions' boundaries and configurations. */

static struct tzc380_reg tzc380_reg_list[] = {
	{
		TZC_ATTR_SP_NS_RW,	/* .secure attr */
		0x0,			/* .enabled */
		0x0,			/* .addr */
		0x0,			/* .size */
		0x0,			/* .submask */
	},
	{
		TZC_ATTR_SP_S_RW,	/* .secure attr */
		TZC_ATTR_REGION_ENABLE,
		0x0,
		TZC_REGION_SIZE_2M,
		0x0,			/* Disable region 7 */
	},
	{
		TZC_ATTR_SP_S_RW,	/* .secure attr */
		TZC_ATTR_REGION_ENABLE,
		0x0,
		TZC_REGION_SIZE_64M,
		0x80,			/* Disable region 7 */
	},
	/* reserve 2M non-scure memory for OPTEE public memory */
	{
		TZC_ATTR_SP_S_RW,	/* .secure attr */
		TZC_ATTR_REGION_ENABLE,
		0x0,
		TZC_REGION_SIZE_8M,
		0xC0,			/* Disable region 6 & 7 */
	},

	{}
};

#endif /* PLAT_TZC380_H */
