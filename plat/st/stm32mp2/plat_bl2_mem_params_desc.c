/*
 * Copyright (c) 2023-2024, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/bl_common.h>
#include <common/desc_image_load.h>
#include <plat/common/platform.h>

#include <platform_def.h>

#if STM32MP_BL33_EL1
#define BL33_MODE MODE_EL1
#else
#define BL33_MODE MODE_EL2
#endif

/*******************************************************************************
 * Following descriptor provides BL image/ep information that gets used
 * by BL2 to load the images and also subset of this information is
 * passed to next BL image. The image loading sequence is managed by
 * populating the images in required loading order. The image execution
 * sequence is managed by populating the `next_handoff_image_id` with
 * the next executable image id.
 ******************************************************************************/
static bl_mem_params_node_t bl2_mem_params_descs[] = {
#if STM32MP_DDR_FIP_IO_STORAGE
	/* Fill FW_DDR related information if it exists */
	{
		.image_id = DDR_FW_ID,
		SET_STATIC_PARAM_HEAD(ep_info, PARAM_IMAGE_BINARY,
				      VERSION_2, entry_point_info_t,
				      SECURE | NON_EXECUTABLE),
		SET_STATIC_PARAM_HEAD(image_info, PARAM_IMAGE_BINARY,
				      VERSION_2, image_info_t,
				      0),

		.image_info.image_base = STM32MP_DDR_FW_BASE,
		.image_info.image_max_size = STM32MP_DDR_FW_MAX_SIZE,

		.next_handoff_image_id = INVALID_IMAGE_ID,
	},
#endif

	/* Fill FW_CONFIG related information if it exists */
	{
		.image_id = FW_CONFIG_ID,
		SET_STATIC_PARAM_HEAD(ep_info, PARAM_IMAGE_BINARY,
				      VERSION_2, entry_point_info_t,
				      SECURE | NON_EXECUTABLE),
		SET_STATIC_PARAM_HEAD(image_info, PARAM_IMAGE_BINARY,
				      VERSION_2, image_info_t,
				      IMAGE_ATTRIB_PLAT_SETUP),

		.image_info.image_base = STM32MP_FW_CONFIG_BASE,
		.image_info.image_max_size = STM32MP_FW_CONFIG_MAX_SIZE,

		.next_handoff_image_id = INVALID_IMAGE_ID,
	},

	/* Fill BL31 related information */
	{
		.image_id = BL31_IMAGE_ID,

		SET_STATIC_PARAM_HEAD(ep_info, PARAM_EP,
				      VERSION_2, entry_point_info_t,
				      SECURE | EXECUTABLE | EP_FIRST_EXE),

		.ep_info.spsr = SPSR_64(MODE_EL3, MODE_SP_ELX, DISABLE_ALL_EXCEPTIONS),

		SET_STATIC_PARAM_HEAD(image_info, PARAM_EP,
				      VERSION_2, image_info_t,
				      IMAGE_ATTRIB_SKIP_LOADING),

		.next_handoff_image_id = BL32_IMAGE_ID,
	},

	/* Fill SoC FW config related information */
	{
		.image_id = SOC_FW_CONFIG_ID,

		SET_STATIC_PARAM_HEAD(ep_info, PARAM_IMAGE_BINARY,
				      VERSION_2, entry_point_info_t,
				      SECURE | NON_EXECUTABLE),

		SET_STATIC_PARAM_HEAD(image_info, PARAM_IMAGE_BINARY,
				      VERSION_2, image_info_t,
				      IMAGE_ATTRIB_SKIP_LOADING),

		.next_handoff_image_id = INVALID_IMAGE_ID,
	},

	/* Fill BL32 related information */
	{
		.image_id = BL32_IMAGE_ID,

		SET_STATIC_PARAM_HEAD(ep_info, PARAM_EP,
				      VERSION_2, entry_point_info_t,
				      SECURE | EXECUTABLE),

		SET_STATIC_PARAM_HEAD(image_info, PARAM_EP,
				      VERSION_2, image_info_t,
				      IMAGE_ATTRIB_SKIP_LOADING),

		.next_handoff_image_id = BL33_IMAGE_ID,
	},

	/* Fill BL32 external 1 image related information */
	{
		.image_id = BL32_EXTRA1_IMAGE_ID,

		SET_STATIC_PARAM_HEAD(ep_info, PARAM_EP,
				      VERSION_2, entry_point_info_t,
				      SECURE | NON_EXECUTABLE),

		SET_STATIC_PARAM_HEAD(image_info, PARAM_EP,
				      VERSION_2, image_info_t,
				      IMAGE_ATTRIB_SKIP_LOADING),

		.next_handoff_image_id = INVALID_IMAGE_ID,
	},

	/* Fill HW_CONFIG related information if it exists */
	{
		.image_id = HW_CONFIG_ID,

		SET_STATIC_PARAM_HEAD(ep_info, PARAM_IMAGE_BINARY,
				      VERSION_2, entry_point_info_t,
				      NON_SECURE | NON_EXECUTABLE),

		SET_STATIC_PARAM_HEAD(image_info, PARAM_IMAGE_BINARY,
				      VERSION_2, image_info_t,
				      IMAGE_ATTRIB_SKIP_LOADING),

		.next_handoff_image_id = INVALID_IMAGE_ID,
	},

	/* Fill BL33 related information */
	{
		.image_id = BL33_IMAGE_ID,

		SET_STATIC_PARAM_HEAD(ep_info, PARAM_EP,
				      VERSION_2, entry_point_info_t,
				      NON_SECURE | EXECUTABLE),

		.ep_info.spsr = SPSR_64(BL33_MODE, MODE_SP_ELX, DISABLE_ALL_EXCEPTIONS),

		SET_STATIC_PARAM_HEAD(image_info, PARAM_EP,
				      VERSION_2, image_info_t,
				      IMAGE_ATTRIB_SKIP_LOADING),

		.next_handoff_image_id = INVALID_IMAGE_ID,
	}
};

REGISTER_BL_IMAGE_DESCS(bl2_mem_params_descs)
