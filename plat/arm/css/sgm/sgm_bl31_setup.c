/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <bl_common.h>
#include <debug.h>
#include <plat_arm.h>
#include <sgm_plat_config.h>

void bl31_early_platform_setup2(u_register_t arg0, u_register_t arg1,
				u_register_t arg2, u_register_t arg3)
{
	uint32_t plat_version;
	bl_params_node_t *bl_params;

	bl_params = ((bl_params_t *)arg0)->head;

	/* Initialize the platform configuration structure */
	plat_config_init();

	while (bl_params) {
		if (bl_params->image_id == BL33_IMAGE_ID) {
			plat_version = mmio_read_32(SSC_VERSION);
			bl_params->ep_info->args.arg2 = plat_version;
			break;
		}

		bl_params = bl_params->next_params_info;
	}

	arm_bl31_early_platform_setup((void *)arg0, arg1, arg2, (void *)arg3);
}
