/*
 * Copyright (c) 2017-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <platform_def.h>

#include <arch.h>
#include <common/desc_image_load.h>

#include "uniphier.h"

#define UNIPHIER_BL33_OFFSET		0x04000000UL
#define UNIPHIER_BL33_MAX_SIZE		0x00800000UL

#define UNIPHIER_SCP_OFFSET		0x04800000UL
#define UNIPHIER_SCP_MAX_SIZE		0x00020000UL

static struct bl_mem_params_node uniphier_image_descs[] = {
	{
		.image_id = SCP_BL2_IMAGE_ID,

		SET_STATIC_PARAM_HEAD(image_info, PARAM_EP,
				      VERSION_2, image_info_t, 0),
		.image_info.image_base = UNIPHIER_SCP_OFFSET,
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
		.image_info.image_base = UNIPHIER_BL31_OFFSET,
		.image_info.image_max_size = UNIPHIER_BL31_MAX_SIZE,

		SET_STATIC_PARAM_HEAD(ep_info, PARAM_EP,
				      VERSION_2, entry_point_info_t,
				      SECURE | EXECUTABLE | EP_FIRST_EXE),
		.ep_info.pc = UNIPHIER_BL31_OFFSET,
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
		.image_info.image_base = UNIPHIER_BL32_OFFSET,
		.image_info.image_max_size = UNIPHIER_BL32_MAX_SIZE,

		SET_STATIC_PARAM_HEAD(ep_info, PARAM_EP,
				      VERSION_2, entry_point_info_t,
				      SECURE | EXECUTABLE),
		.ep_info.pc = UNIPHIER_BL32_OFFSET,
		.ep_info.spsr = SPSR_64(MODE_EL3, MODE_SP_ELX,
					DISABLE_ALL_EXCEPTIONS),

		.next_handoff_image_id = BL33_IMAGE_ID,
	},
#endif
	{
		.image_id = BL33_IMAGE_ID,

		SET_STATIC_PARAM_HEAD(image_info, PARAM_EP,
				      VERSION_2, image_info_t, 0),
		.image_info.image_base = UNIPHIER_BL33_OFFSET,
		.image_info.image_max_size = UNIPHIER_BL33_MAX_SIZE,

		SET_STATIC_PARAM_HEAD(ep_info, PARAM_EP,
				      VERSION_2, entry_point_info_t,
				      NON_SECURE | EXECUTABLE),
		.ep_info.pc = UNIPHIER_BL33_OFFSET,
		.ep_info.spsr = SPSR_64(MODE_EL2, MODE_SP_ELX,
					DISABLE_ALL_EXCEPTIONS),

		.next_handoff_image_id = INVALID_IMAGE_ID,
	},
};
REGISTER_BL_IMAGE_DESCS(uniphier_image_descs)

/*
 * image_info.image_base and ep_info.pc are the offset from the memory base.
 * When ENABLE_PIE is set, we never know the real memory base at link-time.
 * Fix-up the addresses by adding the run-time detected base.
 */
void uniphier_init_image_descs(uintptr_t mem_base)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(uniphier_image_descs); i++) {
		uniphier_image_descs[i].image_info.image_base += mem_base;
		uniphier_image_descs[i].ep_info.pc += mem_base;
	}
}

struct image_info *uniphier_get_image_info(unsigned int image_id)
{
	struct bl_mem_params_node *desc;

	desc = get_bl_mem_params_node(image_id);
	assert(desc);
	return &desc->image_info;
}
