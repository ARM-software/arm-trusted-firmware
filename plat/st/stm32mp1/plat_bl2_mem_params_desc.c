/*
 * Copyright (c) 2016-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>

#include <common/bl_common.h>
#include <common/desc_image_load.h>
#include <plat/common/platform.h>

/*******************************************************************************
 * Following descriptor provides BL image/ep information that gets used
 * by BL2 to load the images and also subset of this information is
 * passed to next BL image. The image loading sequence is managed by
 * populating the images in required loading order. The image execution
 * sequence is managed by populating the `next_handoff_image_id` with
 * the next executable image id.
 ******************************************************************************/
static bl_mem_params_node_t bl2_mem_params_descs[] = {
	/* Fill BL32 related information */
	{
		.image_id = BL32_IMAGE_ID,

		SET_STATIC_PARAM_HEAD(ep_info, PARAM_EP,
				      VERSION_2, entry_point_info_t,
				      SECURE | EXECUTABLE | EP_FIRST_EXE),

#if !defined(AARCH32_SP_OPTEE)
		.ep_info.pc = BL32_BASE,
#endif
		.ep_info.spsr = SPSR_MODE32(MODE32_svc, SPSR_T_ARM,
					    SPSR_E_LITTLE,
					    DISABLE_ALL_EXCEPTIONS),

		SET_STATIC_PARAM_HEAD(image_info, PARAM_EP,
				      VERSION_2, image_info_t,
				      IMAGE_ATTRIB_PLAT_SETUP),
#if defined(AARCH32_SP_OPTEE)
		/* optee header is loaded in SYSRAM above BL2 */
		.image_info.image_base = STM32MP_OPTEE_BASE,
		.image_info.image_max_size = STM32MP_OPTEE_SIZE,
#else
		.image_info.image_base = BL32_BASE,
		.image_info.image_max_size = BL32_LIMIT - BL32_BASE,
#endif
		.next_handoff_image_id = BL33_IMAGE_ID,
	},

#if defined(AARCH32_SP_OPTEE)
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
	/* Fill BL32 external 2 image related information */
	{
		.image_id = BL32_EXTRA2_IMAGE_ID,

		SET_STATIC_PARAM_HEAD(ep_info, PARAM_EP,
				      VERSION_2, entry_point_info_t,
				      SECURE | NON_EXECUTABLE),

		SET_STATIC_PARAM_HEAD(image_info, PARAM_EP,
				      VERSION_2, image_info_t,
				      IMAGE_ATTRIB_SKIP_LOADING),

		.next_handoff_image_id = INVALID_IMAGE_ID,
	},
#endif /* AARCH32_SP_OPTEE */

	/* Fill BL33 related information */
	{
		.image_id = BL33_IMAGE_ID,

		SET_STATIC_PARAM_HEAD(ep_info, PARAM_EP,
				      VERSION_2, entry_point_info_t,
				      NON_SECURE | EXECUTABLE),

		.ep_info.pc = PLAT_STM32MP_NS_IMAGE_OFFSET,
		.ep_info.spsr = SPSR_MODE32(MODE32_svc, SPSR_T_ARM,
					    SPSR_E_LITTLE,
					    DISABLE_ALL_EXCEPTIONS),

		SET_STATIC_PARAM_HEAD(image_info, PARAM_EP,
				      VERSION_2, image_info_t, 0),

		.image_info.image_base = PLAT_STM32MP_NS_IMAGE_OFFSET,
		.image_info.image_max_size = STM32MP_DDR_MAX_SIZE -
			(PLAT_STM32MP_NS_IMAGE_OFFSET - STM32MP_DDR_BASE),

		.next_handoff_image_id = INVALID_IMAGE_ID,
	}
};

REGISTER_BL_IMAGE_DESCS(bl2_mem_params_descs)
