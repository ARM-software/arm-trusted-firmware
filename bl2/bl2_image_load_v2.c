/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_helpers.h>
#include <assert.h>
#include <auth_mod.h>
#include <bl_common.h>
#include <debug.h>
#include <desc_image_load.h>
#include <platform.h>
#include <platform_def.h>
#include <stdint.h>


/*******************************************************************************
 * This function loads SCP_BL2/BL3x images and returns the ep_info for
 * the next executable image.
 ******************************************************************************/
entry_point_info_t *bl2_load_images(void)
{
	bl_params_t *bl2_to_next_bl_params;
	bl_load_info_t *bl2_load_info;
	const bl_load_info_node_t *bl2_node_info;
	int plat_setup_done = 0;
	int err;

	/*
	 * Get information about the images to load.
	 */
	bl2_load_info = plat_get_bl_image_load_info();
	assert(bl2_load_info);
	assert(bl2_load_info->head);
	assert(bl2_load_info->h.type == PARAM_BL_LOAD_INFO);
	assert(bl2_load_info->h.version >= VERSION_2);
	bl2_node_info = bl2_load_info->head;

	while (bl2_node_info) {
		/*
		 * Perform platform setup before loading the image,
		 * if indicated in the image attributes AND if NOT
		 * already done before.
		 */
		if (bl2_node_info->image_info->h.attr & IMAGE_ATTRIB_PLAT_SETUP) {
			if (plat_setup_done) {
				WARN("BL2: Platform setup already done!!\n");
			} else {
				INFO("BL2: Doing platform setup\n");
				bl2_platform_setup();
				plat_setup_done = 1;
			}
		}

		if (!(bl2_node_info->image_info->h.attr & IMAGE_ATTRIB_SKIP_LOADING)) {
			INFO("BL2: Loading image id %d\n", bl2_node_info->image_id);
			err = load_auth_image(bl2_node_info->image_id,
				bl2_node_info->image_info);
			if (err) {
				ERROR("BL2: Failed to load image (%i)\n", err);
				plat_error_handler(err);
			}
		} else {
			INFO("BL2: Skip loading image id %d\n", bl2_node_info->image_id);
		}

		/* Allow platform to handle image information. */
		err = bl2_plat_handle_post_image_load(bl2_node_info->image_id);
		if (err) {
			ERROR("BL2: Failure in post image load handling (%i)\n", err);
			plat_error_handler(err);
		}

		/* Go to next image */
		bl2_node_info = bl2_node_info->next_load_info;
	}

	/*
	 * Get information to pass to the next image.
	 */
	bl2_to_next_bl_params = plat_get_next_bl_params();
	assert(bl2_to_next_bl_params);
	assert(bl2_to_next_bl_params->head);
	assert(bl2_to_next_bl_params->h.type == PARAM_BL_PARAMS);
	assert(bl2_to_next_bl_params->h.version >= VERSION_2);
	assert(bl2_to_next_bl_params->head->ep_info);

	/* Populate arg0 for the next BL image */
	bl2_to_next_bl_params->head->ep_info->args.arg0 = (u_register_t)bl2_to_next_bl_params;

	/* Flush the parameters to be passed to next image */
	plat_flush_next_bl_params();

	return bl2_to_next_bl_params->head->ep_info;
}
