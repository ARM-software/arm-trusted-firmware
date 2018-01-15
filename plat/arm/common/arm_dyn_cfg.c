/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <debug.h>
#include <desc_image_load.h>
#include <platform.h>
#include <platform_def.h>
#include <string.h>
#include <tbbr_img_def.h>

#if LOAD_IMAGE_V2

/*
 * Helper function to load TB_FW_CONFIG and populate the load information to
 * arg0 of BL2 entrypoint info.
 */
void arm_load_tb_fw_config(void)
{
	int err;
	uintptr_t config_base = 0;
	image_desc_t *image_desc;

	image_desc_t arm_tb_fw_info = {
		.image_id = TB_FW_CONFIG_ID,
		SET_STATIC_PARAM_HEAD(image_info, PARAM_IMAGE_BINARY,
				VERSION_2, image_info_t, 0),
		.image_info.image_base = ARM_TB_FW_CONFIG_BASE,
		.image_info.image_max_size = ARM_TB_FW_CONFIG_LIMIT - ARM_TB_FW_CONFIG_BASE,
	};

	VERBOSE("BL1: Loading TB_FW_CONFIG\n");
	err = load_auth_image(TB_FW_CONFIG_ID, &arm_tb_fw_info.image_info);
	if (err) {
		/* Return if TB_FW_CONFIG is not loaded */
		VERBOSE("Failed to load TB_FW_CONFIG\n");
		return;
	}

	config_base = arm_tb_fw_info.image_info.image_base;

	/* The BL2 ep_info arg0 is modified to point to TB_FW_CONFIG */
	image_desc = bl1_plat_get_image_desc(BL2_IMAGE_ID);
	assert(image_desc);
	image_desc->ep_info.args.arg0 = config_base;

	INFO("BL1: TB_FW_CONFIG loaded at address = %p\n",
			(void *) config_base);
}

#endif /* LOAD_IMAGE_V2 */
