/*
 * Copyright (c) 2024, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <plat/arm/common/plat_arm.h>
#include <platform_def.h>

#if (TARGET_PLATFORM <= 2)
static const arm_tzc_regions_info_t tzc_regions[] = {
	TC_TZC_REGIONS_DEF,
	{}
};
#endif

/* Initialize the secure environment */
void plat_arm_security_setup(void)
{
#if (TARGET_PLATFORM <= 2)
	unsigned int i;

	for (i = 0U; i < TZC400_COUNT; i++) {
		arm_tzc400_setup(TZC400_BASE(i), tzc_regions);
	}
#endif
}
