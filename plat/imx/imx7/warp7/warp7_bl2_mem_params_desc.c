/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <bl_common.h>
#include <desc_image_load.h>
#include <platform.h>
#include <platform_def.h>

static bl_mem_params_node_t bl2_mem_params_descs[] = {
	{
		.image_id = BL32_IMAGE_ID,

		SET_STATIC_PARAM_HEAD(ep_info, PARAM_EP, VERSION_2,
				      entry_point_info_t,
				      SECURE | EXECUTABLE | EP_FIRST_EXE),
		.ep_info.pc = BL32_BASE,

		SET_STATIC_PARAM_HEAD(image_info, PARAM_EP, VERSION_2,
				      image_info_t, 0),

		.image_info.image_base = WARP7_OPTEE_BASE,
		.image_info.image_max_size = WARP7_OPTEE_SIZE,

		.next_handoff_image_id = BL33_IMAGE_ID,
	},
	{
		.image_id = HW_CONFIG_ID,
		SET_STATIC_PARAM_HEAD(ep_info, PARAM_IMAGE_BINARY,
				      VERSION_2, entry_point_info_t, SECURE | NON_EXECUTABLE),
		SET_STATIC_PARAM_HEAD(image_info, PARAM_IMAGE_BINARY,
				      VERSION_2, image_info_t, 0),
		.image_info.image_base = WARP7_DTB_BASE,
		.image_info.image_max_size = WARP7_DTB_SIZE,
		.next_handoff_image_id = INVALID_IMAGE_ID,
	},
	{
		.image_id = BL32_EXTRA1_IMAGE_ID,

		SET_STATIC_PARAM_HEAD(ep_info, PARAM_EP, VERSION_2,
				      entry_point_info_t,
				      SECURE | NON_EXECUTABLE),

		SET_STATIC_PARAM_HEAD(image_info, PARAM_EP, VERSION_2,
				      image_info_t, IMAGE_ATTRIB_SKIP_LOADING),
		.image_info.image_base = WARP7_OPTEE_BASE,
		.image_info.image_max_size = WARP7_OPTEE_SIZE,

		.next_handoff_image_id = INVALID_IMAGE_ID,
	},
	{
		/* This is a zero sized image so we don't set base or size */
		.image_id = BL32_EXTRA2_IMAGE_ID,

		SET_STATIC_PARAM_HEAD(ep_info, PARAM_EP,
				      VERSION_2, entry_point_info_t,
				      SECURE | NON_EXECUTABLE),

		SET_STATIC_PARAM_HEAD(image_info, PARAM_EP,
				      VERSION_2, image_info_t,
				      IMAGE_ATTRIB_SKIP_LOADING),
		.next_handoff_image_id = INVALID_IMAGE_ID,
	},
	{
		.image_id = BL33_IMAGE_ID,
		SET_STATIC_PARAM_HEAD(ep_info, PARAM_EP, VERSION_2,
				      entry_point_info_t,
				      NON_SECURE | EXECUTABLE),
		# ifdef PRELOADED_BL33_BASE
			.ep_info.pc = PRELOADED_BL33_BASE,

			SET_STATIC_PARAM_HEAD(image_info, PARAM_EP,
					      VERSION_2, image_info_t,
					      IMAGE_ATTRIB_SKIP_LOADING),
		# else
			.ep_info.pc = BL33_BASE,

			SET_STATIC_PARAM_HEAD(image_info, PARAM_EP,
					      VERSION_2, image_info_t, 0),
			.image_info.image_base = WARP7_UBOOT_BASE,
			.image_info.image_max_size = WARP7_UBOOT_SIZE,
		# endif /* PRELOADED_BL33_BASE */

		.next_handoff_image_id = INVALID_IMAGE_ID,
	}
};

REGISTER_BL_IMAGE_DESCS(bl2_mem_params_descs);
