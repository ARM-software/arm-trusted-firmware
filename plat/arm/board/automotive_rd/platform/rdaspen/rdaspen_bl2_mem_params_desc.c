/*
 * Copyright (c) 2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/bl_common.h>
#include <common/desc_image_load.h>
#include <platform_def.h>

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
#if DEBUG
		.ep_info.args.arg3 = ARM_BL31_PLAT_PARAM_VAL,
#endif

		SET_STATIC_PARAM_HEAD(image_info, PARAM_EP,
			VERSION_2, image_info_t, IMAGE_ATTRIB_PLAT_SETUP),
		.image_info.image_base = BL31_BASE,
		.image_info.image_max_size = BL31_LIMIT - BL31_BASE,
#ifdef BL32_BASE
		.next_handoff_image_id = BL32_IMAGE_ID,
#else
		.next_handoff_image_id = BL33_IMAGE_ID,
#endif
	},
	/* Fill HW_CONFIG related information */
	{
		.image_id = HW_CONFIG_ID,
		SET_STATIC_PARAM_HEAD(ep_info, PARAM_IMAGE_BINARY,
			VERSION_2, entry_point_info_t,
			NON_SECURE | NON_EXECUTABLE),
		SET_STATIC_PARAM_HEAD(image_info, PARAM_IMAGE_BINARY,
			VERSION_2, image_info_t, IMAGE_ATTRIB_SKIP_LOADING),
		.next_handoff_image_id = INVALID_IMAGE_ID,
	},
#ifdef BL32_BASE
	/* Fill BL32 related information */
	{
		.image_id = BL32_IMAGE_ID,
		SET_STATIC_PARAM_HEAD(ep_info, PARAM_EP,
			VERSION_2, entry_point_info_t, SECURE | EXECUTABLE),
		.ep_info.pc = BL32_BASE,
		SET_STATIC_PARAM_HEAD(image_info, PARAM_EP,
			VERSION_2, image_info_t, 0),
		.image_info.image_base = BL32_BASE,
		.image_info.image_max_size = BL32_LIMIT - BL32_BASE,
		.next_handoff_image_id = BL33_IMAGE_ID,
	},

	/* Fill TOS_FW_CONFIG related information */
	{
		.image_id = TOS_FW_CONFIG_ID,
		SET_STATIC_PARAM_HEAD(ep_info, PARAM_IMAGE_BINARY,
			VERSION_2, entry_point_info_t, SECURE | NON_EXECUTABLE),
		SET_STATIC_PARAM_HEAD(image_info, PARAM_IMAGE_BINARY,
			VERSION_2, image_info_t, IMAGE_ATTRIB_SKIP_LOADING),
		.next_handoff_image_id = INVALID_IMAGE_ID,
	},
#endif
	/* Fill BL33 related information */
	{
		.image_id = BL33_IMAGE_ID,
		SET_STATIC_PARAM_HEAD(ep_info, PARAM_EP,
			VERSION_2, entry_point_info_t, NON_SECURE | EXECUTABLE),
		.ep_info.pc = PLAT_ARM_NS_IMAGE_BASE,

		SET_STATIC_PARAM_HEAD(image_info, PARAM_EP,
			VERSION_2, image_info_t, 0),
		.image_info.image_base = PLAT_ARM_NS_IMAGE_BASE,
		.image_info.image_max_size = ARM_DRAM1_BASE + ARM_DRAM1_SIZE
			- PLAT_ARM_NS_IMAGE_BASE,
		.next_handoff_image_id = INVALID_IMAGE_ID,
	},
};

REGISTER_BL_IMAGE_DESCS(bl2_mem_params_descs)
