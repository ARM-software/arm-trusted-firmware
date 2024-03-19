/*
 * Copyright (c) 2020-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <plat/arm/common/plat_arm.h>
#include <platform_def.h>

#define RDN2_TZC_CPER_REGION				\
	{NRD_CSS_SP_CPER_BUF_BASE, (NRD_CSS_SP_CPER_BUF_BASE +	\
	NRD_CSS_SP_CPER_BUF_SIZE) - 1, TZC_REGION_S_NONE,	\
	PLAT_ARM_TZC_NS_DEV_ACCESS}

static const arm_tzc_regions_info_t tzc_regions[] = {
	ARM_TZC_REGIONS_DEF,
#if ENABLE_FEAT_RAS && FFH_SUPPORT
	RDN2_TZC_CPER_REGION,
#endif
	{}
};

#if (NRD_PLATFORM_VARIANT == 2 && NRD_CHIP_COUNT > 1)
static const arm_tzc_regions_info_t tzc_regions_mc[][NRD_CHIP_COUNT - 1] = {
	{
		/* TZC memory regions for second chip */
		NRD_ROS_TZC_NS_REMOTE_REGIONS_DEF(1),
		{}
	},
#if NRD_CHIP_COUNT > 2
	{
		/* TZC memory regions for third chip */
		NRD_ROS_TZC_NS_REMOTE_REGIONS_DEF(2),
		{}
	},
#endif
#if NRD_CHIP_COUNT > 3
	{
		/* TZC memory regions for fourth chip */
		NRD_ROS_TZC_NS_REMOTE_REGIONS_DEF(3),
		{}
	},
#endif
};
#endif /* NRD_PLATFORM_VARIANT && NRD_CHIP_COUNT */

/* Initialize the secure environment */
void plat_arm_security_setup(void)
{
	unsigned int i;

	INFO("Configuring TrustZone Controller for Chip 0\n");

	for (i = 0; i < TZC400_COUNT; i++) {
		arm_tzc400_setup(TZC400_BASE(i), tzc_regions);
	}

#if (NRD_PLATFORM_VARIANT == 2 && NRD_CHIP_COUNT > 1)
	unsigned int j;

	for (i = 1; i < NRD_CHIP_COUNT; i++) {
		INFO("Configuring TrustZone Controller for Chip %u\n", i);

		for (j = 0; j < TZC400_COUNT; j++) {
			arm_tzc400_setup(NRD_REMOTE_CHIP_MEM_OFFSET(i)
				+ TZC400_BASE(j), tzc_regions_mc[i-1]);
		}
	}
#endif
}
