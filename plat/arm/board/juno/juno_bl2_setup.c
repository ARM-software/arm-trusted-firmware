/*
 * Copyright (c) 2016-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <common/bl_common.h>
#include <common/desc_image_load.h>
#include <plat/arm/common/plat_arm.h>

#if JUNO_AARCH32_EL3_RUNTIME
/*******************************************************************************
 * This function changes the spsr for BL32 image to bypass
 * the check in BL1 AArch64 exception handler. This is needed in the aarch32
 * boot flow as the core comes up in aarch64 and to enter the BL32 image a warm
 * reset in aarch32 state is required.
 ******************************************************************************/
int arm_bl2_plat_handle_post_image_load(unsigned int image_id)
{
	int err = arm_bl2_handle_post_image_load(image_id);

	if (!err && (image_id == BL32_IMAGE_ID)) {
		bl_mem_params_node_t *bl_mem_params = get_bl_mem_params_node(image_id);
		assert(bl_mem_params);
		bl_mem_params->ep_info.spsr = SPSR_64(MODE_EL3, MODE_SP_ELX,
			DISABLE_ALL_EXCEPTIONS);
	}

	return err;
}
#endif /* JUNO_AARCH32_EL3_RUNTIME */
