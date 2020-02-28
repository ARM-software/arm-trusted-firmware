/*
 * Copyright (c) 2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <plat/arm/common/plat_arm.h>
#include <platform_def.h>

static const arm_tzc_regions_info_t tzc_regions[] = {
	ARM_TZC_REGIONS_DEF,
	{}
};

/* Initialize the secure environment */
void plat_arm_security_setup(void)
{
	int i;

	for (i = 0; i < TZC400_COUNT; i++)
		arm_tzc400_setup(TZC400_BASE(i), tzc_regions);
}
