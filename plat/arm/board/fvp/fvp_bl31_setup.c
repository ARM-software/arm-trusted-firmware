/*
 * Copyright (c) 2013-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <drivers/arm/smmu_v3.h>
#include <plat/arm/common/arm_config.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>

#include "fvp_private.h"

void __init bl31_early_platform_setup2(u_register_t arg0,
		u_register_t arg1, u_register_t arg2, u_register_t arg3)
{
	arm_bl31_early_platform_setup((void *)arg0, arg1, arg2, (void *)arg3);

	/* Initialize the platform config for future decision making */
	fvp_config_setup();

	/*
	 * Initialize the correct interconnect for this cluster during cold
	 * boot. No need for locks as no other CPU is active.
	 */
	fvp_interconnect_init();

	/*
	 * Enable coherency in interconnect for the primary CPU's cluster.
	 * Earlier bootloader stages might already do this (e.g. Trusted
	 * Firmware's BL1 does it) but we can't assume so. There is no harm in
	 * executing this code twice anyway.
	 * FVP PSCI code will enable coherency for other clusters.
	 */
	fvp_interconnect_enable();

	/* On FVP RevC, initialize SMMUv3 */
	if ((arm_config.flags & ARM_CONFIG_FVP_HAS_SMMUV3) != 0U)
		smmuv3_init(PLAT_FVP_SMMUV3_BASE);
}
