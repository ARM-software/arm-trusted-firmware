/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <assert.h>
#include <desc_image_load.h>
#include <platform_def.h>

#include "uniphier.h"

static struct bl_mem_params_node uniphier_image_descs[] = {
	{
		.image_id = SCP_BL2_IMAGE_ID,

		SET_STATIC_PARAM_HEAD(image_info, PARAM_EP,
				      VERSION_2, image_info_t, 0),
		.image_info.image_base = UNIPHIER_SCP_BASE,
		.image_info.image_max_size = UNIPHIER_SCP_MAX_SIZE,

		SET_STATIC_PARAM_HEAD(ep_info, PARAM_EP,
				      VERSION_2, entry_point_info_t,
				      NON_SECURE | NON_EXECUTABLE),

		.next_handoff_image_id = INVALID_IMAGE_ID,
	},
	{
		.image_id = BL31_IMAGE_ID,

		SET_STATIC_PARAM_HEAD(image_info, PARAM_EP,
				      VERSION_2, image_info_t, 0),
		.image_info.image_base = BL31_BASE,
		.image_info.image_max_size = BL31_LIMIT - BL31_BASE,

		SET_STATIC_PARAM_HEAD(ep_info, PARAM_EP,
				      VERSION_2, entry_point_info_t,
				      SECURE | EXECUTABLE | EP_FIRST_EXE),
		.ep_info.pc = BL31_BASE,
		.ep_info.spsr = SPSR_64(MODE_EL3, MODE_SP_ELX,
					DISABLE_ALL_EXCEPTIONS),

#ifdef UNIPHIER_LOAD_BL32
		.next_handoff_image_id = BL32_IMAGE_ID,
#else
		.next_handoff_image_id = BL33_IMAGE_ID,
#endif
	},
#ifdef UNIPHIER_LOAD_BL32
	{
		.image_id = BL32_IMAGE_ID,

		SET_STATIC_PARAM_HEAD(image_info, PARAM_EP,
				      VERSION_2, image_info_t, 0),
		.image_info.image_base = BL32_BASE,
		.image_info.image_max_size = BL32_LIMIT - BL32_BASE,

		SET_STATIC_PARAM_HEAD(ep_info, PARAM_EP,
				      VERSION_2, entry_point_info_t,
				      SECURE | EXECUTABLE),
		.ep_info.pc = BL32_BASE,
		.ep_info.spsr = SPSR_64(MODE_EL3, MODE_SP_ELX,
					DISABLE_ALL_EXCEPTIONS),

		.next_handoff_image_id = BL33_IMAGE_ID,
	},
#endif
	{
		.image_id = BL33_IMAGE_ID,

		SET_STATIC_PARAM_HEAD(image_info, PARAM_EP,
				      VERSION_2, image_info_t, 0),
		.image_info.image_base = UNIPHIER_BL33_BASE,
		.image_info.image_max_size = UNIPHIER_BL33_MAX_SIZE,

		SET_STATIC_PARAM_HEAD(ep_info, PARAM_EP,
				      VERSION_2, entry_point_info_t,
				      NON_SECURE | EXECUTABLE),
		.ep_info.pc = UNIPHIER_BL33_BASE,
		.ep_info.spsr = SPSR_64(MODE_EL1, MODE_SP_ELX,
					DISABLE_ALL_EXCEPTIONS),

		.next_handoff_image_id = INVALID_IMAGE_ID,
	},
};
REGISTER_BL_IMAGE_DESCS(uniphier_image_descs)

/* SCP is optional.  Allow run-time fixup of the descriptor array. */
void uniphier_image_descs_fixup(void)
{
	struct bl_mem_params_node *desc;

	desc = get_bl_mem_params_node(SCP_BL2_IMAGE_ID);
	assert(desc != NULL);
	desc->image_info.h.attr |= IMAGE_ATTRIB_SKIP_LOADING;
}
