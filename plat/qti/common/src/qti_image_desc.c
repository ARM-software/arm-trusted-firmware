/*
 * Copyright (c) 2025, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <arch.h>
#include <common/desc_image_load.h>
#include <common/tbbr/tbbr_img_def.h>

#include <platform_def.h>

static struct bl_mem_params_node qti_image_descs[] = {
	{
		.image_id = BL31_IMAGE_ID,

		SET_STATIC_PARAM_HEAD(image_info, PARAM_EP,
				      VERSION_2, image_info_t, 0),
		.image_info.image_base = BL31_BASE,
		.image_info.image_max_size = BL31_SIZE,

		SET_STATIC_PARAM_HEAD(ep_info, PARAM_EP,
				      VERSION_2, entry_point_info_t,
				      SECURE | EXECUTABLE | EP_FIRST_EXE),
		.ep_info.pc = BL31_BASE,
		.ep_info.spsr = SPSR_64(MODE_EL3, MODE_SP_ELX,
					DISABLE_ALL_EXCEPTIONS),

		.next_handoff_image_id = BL32_IMAGE_ID,
	},
#if defined(SPD_spmd)
	/*
	 * SPMC core manifest: loaded by BL2 from the FIP as TOS_FW_CONFIG.
	 * BL31/SPMD reads it via BL32 ep_info.args.arg0 in spmd_setup().
	 * Without this entry BL2 never loads the DTB and SPMD stays in
	 * SPMC_STATE_RESET, returning FFA_ERROR_NOT_SUPPORTED to every call.
	 */
	{
		.image_id = TOS_FW_CONFIG_ID,

		SET_STATIC_PARAM_HEAD(image_info, PARAM_EP,
				      VERSION_2, image_info_t, 0),
		.image_info.image_base = TOS_FW_CONFIG_BASE,
		.image_info.image_max_size = TOS_FW_CONFIG_SIZE,

		SET_STATIC_PARAM_HEAD(ep_info, PARAM_EP,
				      VERSION_2, entry_point_info_t,
				      NON_EXECUTABLE),
		.ep_info.pc = TOS_FW_CONFIG_BASE,

		.next_handoff_image_id = INVALID_IMAGE_ID,
	},
#endif
	{
		.image_id = BL32_IMAGE_ID,

		SET_STATIC_PARAM_HEAD(image_info, PARAM_EP,
				      VERSION_2, image_info_t, 0),
		.image_info.image_base = BL32_BASE,
		.image_info.image_max_size = BL32_SIZE,

		SET_STATIC_PARAM_HEAD(ep_info, PARAM_EP,
				      VERSION_2, entry_point_info_t,
				      SECURE | EXECUTABLE),
		.ep_info.pc = BL32_BASE,
		.ep_info.spsr = SPSR_64(MODE_EL3, MODE_SP_ELX,
					DISABLE_ALL_EXCEPTIONS),
		.next_handoff_image_id = BL33_IMAGE_ID,
	},
	{
		.image_id = BL33_IMAGE_ID,

		SET_STATIC_PARAM_HEAD(image_info, PARAM_EP,
				      VERSION_2, image_info_t, 0),
		.image_info.image_base = BL33_BASE,
		.image_info.image_max_size = BL33_SIZE,

		SET_STATIC_PARAM_HEAD(ep_info, PARAM_EP,
				      VERSION_2, entry_point_info_t,
				      NON_SECURE | EXECUTABLE),
		.ep_info.pc = BL33_BASE,
		.ep_info.spsr = SPSR_64(MODE_EL2, MODE_SP_ELX,
					DISABLE_ALL_EXCEPTIONS),

		.next_handoff_image_id = INVALID_IMAGE_ID,
	},
};
REGISTER_BL_IMAGE_DESCS(qti_image_descs)

struct image_info *qti_get_image_info(unsigned int image_id)
{
	struct bl_mem_params_node *desc;

	desc = get_bl_mem_params_node(image_id);
	assert(desc);
	return &desc->image_info;
}
