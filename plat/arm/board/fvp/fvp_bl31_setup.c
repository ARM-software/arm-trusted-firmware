/*
 * Copyright (c) 2013-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arm_config.h>
#include <plat_arm.h>
#include <smmu_v3.h>
#include "fvp_private.h"

#if LOAD_IMAGE_V2
void bl31_early_platform_setup(void *from_bl2,
				void *plat_params_from_bl2)
#else
void bl31_early_platform_setup(bl31_params_t *from_bl2,
				void *plat_params_from_bl2)
#endif
{
	arm_bl31_early_platform_setup(from_bl2, plat_params_from_bl2);

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

	/* On FVP RevC, intialize SMMUv3 */
	if (arm_config.flags & ARM_CONFIG_FVP_HAS_SMMUV3)
		smmuv3_init(PLAT_FVP_SMMUV3_BASE);
}
