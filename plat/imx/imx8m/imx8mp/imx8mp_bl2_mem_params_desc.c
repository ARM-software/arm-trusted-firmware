/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <common/desc_image_load.h>
#include <plat/common/platform.h>
#include <platform_def.h>

static bl_mem_params_node_t bl2_mem_params_descs[] = {
	{
		.image_id = BL31_IMAGE_ID,
		SET_STATIC_PARAM_HEAD(ep_info, PARAM_EP, VERSION_2,
				      entry_point_info_t,
				      SECURE | EXECUTABLE | EP_FIRST_EXE),
		.ep_info.pc = BL31_BASE,
		.ep_info.spsr = SPSR_64(MODE_EL3, MODE_SP_ELX,
					DISABLE_ALL_EXCEPTIONS),
		SET_STATIC_PARAM_HEAD(image_info, PARAM_EP, VERSION_2, image_info_t,
				      IMAGE_ATTRIB_PLAT_SETUP),
		.image_info.image_base = BL31_BASE,
		.image_info.image_max_size = BL31_LIMIT - BL31_BASE,
		.next_handoff_image_id = INVALID_IMAGE_ID,
	},
	{
		.image_id = BL32_IMAGE_ID,

		SET_STATIC_PARAM_HEAD(ep_info, PARAM_EP, VERSION_2,
				      entry_point_info_t,
				      SECURE | EXECUTABLE),
		.ep_info.pc = BL32_BASE,

		SET_STATIC_PARAM_HEAD(image_info, PARAM_EP, VERSION_2,
				      image_info_t, 0),

		.image_info.image_base = BL32_BASE,
		.image_info.image_max_size = BL32_SIZE,

		.next_handoff_image_id = BL33_IMAGE_ID,
	},
	{
		.image_id = BL32_EXTRA1_IMAGE_ID,

		SET_STATIC_PARAM_HEAD(ep_info, PARAM_EP, VERSION_2,
				      entry_point_info_t,
				      SECURE | NON_EXECUTABLE),

		SET_STATIC_PARAM_HEAD(image_info, PARAM_EP, VERSION_2,
				      image_info_t, IMAGE_ATTRIB_SKIP_LOADING),
		.image_info.image_base = BL32_BASE,
		.image_info.image_max_size =  BL32_SIZE,

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
			.ep_info.pc = PLAT_NS_IMAGE_OFFSET,

			SET_STATIC_PARAM_HEAD(image_info, PARAM_EP,
					      VERSION_2, image_info_t,
					      IMAGE_ATTRIB_SKIP_LOADING),
		# else
			.ep_info.pc = PLAT_NS_IMAGE_OFFSET,

			SET_STATIC_PARAM_HEAD(image_info, PARAM_EP,
					      VERSION_2, image_info_t, 0),
			.image_info.image_base = PLAT_NS_IMAGE_OFFSET,
			.image_info.image_max_size = PLAT_NS_IMAGE_SIZE,
		# endif /* PRELOADED_BL33_BASE */

		.next_handoff_image_id = INVALID_IMAGE_ID,
	}
};

REGISTER_BL_IMAGE_DESCS(bl2_mem_params_descs);
