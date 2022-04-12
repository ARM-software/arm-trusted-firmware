/*
 * Copyright (c) 2021-2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/desc_image_load.h>

#include <platform_def.h>

/*******************************************************************************
 * Following descriptor provides BL image/ep information that gets used
 * by BL2 to load the images and also subset of this information is
 * passed to next BL image. The image loading sequence is managed by
 * populating the images in required loading order. The image execution
 * sequence is managed by populating the `next_handoff_image_id` with
 * the next executable image id.
 ******************************************************************************/
static bl_mem_params_node_t bl2_mem_params_descs[] = {

	/* Fill BL31 related information */
	{
		.image_id = BL31_IMAGE_ID,

		SET_STATIC_PARAM_HEAD(ep_info, PARAM_EP,
			VERSION_2, entry_point_info_t,
			SECURE | EXECUTABLE | EP_FIRST_EXE),
		.ep_info.pc = BL31_BASE,
		.ep_info.spsr = SPSR_64(MODE_EL3, MODE_SP_ELX,
			DISABLE_ALL_EXCEPTIONS),
			.ep_info.args.arg3 = ARM_BL31_PLAT_PARAM_VAL,
		SET_STATIC_PARAM_HEAD(image_info, PARAM_EP,
			VERSION_2, image_info_t, IMAGE_ATTRIB_PLAT_SETUP),
		.image_info.image_base = BL31_BASE,
		.image_info.image_max_size = BL31_LIMIT - BL31_BASE,

		.next_handoff_image_id = BL32_IMAGE_ID,
	},

	/* Fill BL32 related information */
	{
		.image_id = BL32_IMAGE_ID,

		SET_STATIC_PARAM_HEAD(ep_info, PARAM_EP,
			VERSION_2, entry_point_info_t, SECURE | EXECUTABLE),
		.ep_info.pc = BL32_BASE,
			.ep_info.args.arg0 = CORSTONE1000_TOS_FW_CONFIG_BASE,
		SET_STATIC_PARAM_HEAD(image_info, PARAM_EP,
			VERSION_2, image_info_t, 0),
		.image_info.image_base = BL32_BASE,
		.image_info.image_max_size = BL32_LIMIT - BL32_BASE,

		.next_handoff_image_id = BL33_IMAGE_ID,
	},

	/* Fill TOS_FW_CONFIG related information */
	{
		.image_id = TOS_FW_CONFIG_ID,
		.image_info.image_base = CORSTONE1000_TOS_FW_CONFIG_BASE,
		.image_info.image_max_size = (CORSTONE1000_TOS_FW_CONFIG_LIMIT -
					      CORSTONE1000_TOS_FW_CONFIG_BASE),
		SET_STATIC_PARAM_HEAD(ep_info, PARAM_IMAGE_BINARY,
			VERSION_2, entry_point_info_t, SECURE | NON_EXECUTABLE),
		SET_STATIC_PARAM_HEAD(image_info, PARAM_IMAGE_BINARY,
		VERSION_2, image_info_t, 0),
		.next_handoff_image_id = INVALID_IMAGE_ID,
	},

	/* Fill BL33 related information */
	{
		.image_id = BL33_IMAGE_ID,
		SET_STATIC_PARAM_HEAD(ep_info, PARAM_EP,
			VERSION_2, entry_point_info_t, NON_SECURE | EXECUTABLE),
		.ep_info.pc = BL33_BASE,

		SET_STATIC_PARAM_HEAD(image_info, PARAM_EP,
			VERSION_2, image_info_t, 0),
		.image_info.image_base = BL33_BASE,
		.image_info.image_max_size = BL33_LIMIT - BL33_BASE,

		.next_handoff_image_id = INVALID_IMAGE_ID,
	},
};

REGISTER_BL_IMAGE_DESCS(bl2_mem_params_descs)
