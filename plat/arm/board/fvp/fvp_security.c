/*
 * Copyright (c) 2014-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <plat/arm/common/arm_config.h>
#include <plat/arm/common/plat_arm.h>
#include <platform_def.h>

/*
 * We assume that all security programming is done by the primary core.
 */
void plat_arm_security_setup(void)
{
	/*
	 * The Base FVP has a TrustZone address space controller, the Foundation
	 * FVP does not. Trying to program the device on the foundation FVP will
	 * cause an abort.
	 *
	 * If the platform had additional peripheral specific security
	 * configurations, those would be configured here.
	 */

	const arm_tzc_regions_info_t fvp_tzc_regions[] = {
		ARM_TZC_REGIONS_DEF,
#if !SPM_MM && !ENABLE_RME
		{FVP_DRAM3_BASE, FVP_DRAM3_END,
		 ARM_TZC_NS_DRAM_S_ACCESS, PLAT_ARM_TZC_NS_DEV_ACCESS},
		{FVP_DRAM4_BASE, FVP_DRAM4_END,
		 ARM_TZC_NS_DRAM_S_ACCESS, PLAT_ARM_TZC_NS_DEV_ACCESS},
		{FVP_DRAM5_BASE, FVP_DRAM5_END,
		 ARM_TZC_NS_DRAM_S_ACCESS, PLAT_ARM_TZC_NS_DEV_ACCESS},
		{FVP_DRAM6_BASE, FVP_DRAM6_END,
		 ARM_TZC_NS_DRAM_S_ACCESS, PLAT_ARM_TZC_NS_DEV_ACCESS},
#endif
		{0}
	};

	if ((get_arm_config()->flags & ARM_CONFIG_HAS_TZC) != 0U)
		arm_tzc400_setup(PLAT_ARM_TZC_BASE, fvp_tzc_regions);
}
