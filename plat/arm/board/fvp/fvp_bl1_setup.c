/*
 * Copyright (c) 2013-2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <plat_arm.h>
#include <tbbr_img_def.h>
#include "fvp_private.h"


/*******************************************************************************
 * Perform any BL1 specific platform actions.
 ******************************************************************************/
void bl1_early_platform_setup(void)
{
	arm_bl1_early_platform_setup();

	/* Initialize the platform config for future decision making */
	fvp_config_setup();

	/*
	 * Initialize Interconnect for this cluster during cold boot.
	 * No need for locks as no other CPU is active.
	 */
	fvp_interconnect_init();
	/*
	 * Enable coherency in Interconnect for the primary CPU's cluster.
	 */
	fvp_interconnect_enable();
}

/*******************************************************************************
 * The following function checks if Firmware update is needed,
 * by checking if TOC in FIP image is valid or not.
 ******************************************************************************/
unsigned int bl1_plat_get_next_image_id(void)
{
	if (!arm_io_is_toc_valid())
		return NS_BL1U_IMAGE_ID;

	return BL2_IMAGE_ID;
}

