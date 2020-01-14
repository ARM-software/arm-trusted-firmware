/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/desc_image_load.h>

/*******************************************************************************
 * This function flushes the data structures so that they are visible
 * in memory for the next BL image.
 ******************************************************************************/
void plat_flush_next_bl_params(void)
{
	flush_bl_params_desc();
}

/*******************************************************************************
 * This function returns the list of loadable images.
 ******************************************************************************/
bl_load_info_t *plat_get_bl_image_load_info(void)
{
	return get_bl_load_info_from_mem_params_desc();
}

/*******************************************************************************
 * This function returns the list of executable images.
 ******************************************************************************/
bl_params_t *plat_get_next_bl_params(void)
{
	unsigned int count;
	unsigned int img_id = 0U;
	unsigned int link_index = 0U;
	bl_params_node_t *bl_exec_node = NULL;
	bl_mem_params_node_t *desc_ptr;

	/* If there is no image to start with, return NULL */
	if (bl_mem_params_desc_num == 0U)
		return NULL;

	/* Clean next_params_info in BL image node */
	for (count = 0U; count < bl_mem_params_desc_num; count++) {

		desc_ptr = &bl_mem_params_desc_ptr[link_index];
		bl_exec_node = &desc_ptr->params_node_mem;
		bl_exec_node->next_params_info = NULL;

		/* If no next hand-off image then break out */
		img_id = desc_ptr->next_handoff_image_id;
		if (img_id == INVALID_IMAGE_ID)
			break;

		/* Get the index for the next hand-off image */
		link_index = get_bl_params_node_index(img_id);
	}

	return get_next_bl_params_from_mem_params_desc();
}
