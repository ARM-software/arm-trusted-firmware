/*
 * Copyright (c) 2016-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/desc_image_load.h>
#include <common/tbbr/tbbr_img_def.h>

static bl_load_info_t bl_load_info;
static bl_params_t next_bl_params;


/*******************************************************************************
 * This function flushes the data structures so that they are visible
 * in memory for the next BL image.
 ******************************************************************************/
void flush_bl_params_desc(void)
{
	flush_bl_params_desc_args(bl_mem_params_desc_ptr,
		bl_mem_params_desc_num,
		&next_bl_params);
}

/*******************************************************************************
 * This function flushes the data structures specified as arguments so that they
 * are visible in memory for the next BL image.
 ******************************************************************************/
void flush_bl_params_desc_args(bl_mem_params_node_t *mem_params_desc_ptr,
	unsigned int mem_params_desc_num,
	bl_params_t *next_bl_params_ptr)
{
	assert(mem_params_desc_ptr != NULL);
	assert(mem_params_desc_num != 0U);
	assert(next_bl_params_ptr != NULL);

	flush_dcache_range((uintptr_t)mem_params_desc_ptr,
		sizeof(*mem_params_desc_ptr) * mem_params_desc_num);

	flush_dcache_range((uintptr_t)next_bl_params_ptr,
			sizeof(*next_bl_params_ptr));
}

/*******************************************************************************
 * This function returns the index for given image_id, within the
 * image descriptor array provided by bl_image_info_descs_ptr, if the
 * image is found else it returns -1.
 ******************************************************************************/
int get_bl_params_node_index(unsigned int image_id)
{
	unsigned int index;
	assert(image_id != INVALID_IMAGE_ID);

	for (index = 0U; index < bl_mem_params_desc_num; index++) {
		if (bl_mem_params_desc_ptr[index].image_id == image_id)
			return (int)index;
	}

	return -1;
}

/*******************************************************************************
 * This function returns the pointer to `bl_mem_params_node_t` object for
 * given image_id, within the image descriptor array provided by
 * bl_mem_params_desc_ptr, if the image is found else it returns NULL.
 ******************************************************************************/
bl_mem_params_node_t *get_bl_mem_params_node(unsigned int image_id)
{
	int index;
	assert(image_id != INVALID_IMAGE_ID);

	index = get_bl_params_node_index(image_id);
	if (index >= 0)
		return &bl_mem_params_desc_ptr[index];
	else
		return NULL;
}

/*******************************************************************************
 * This function creates the list of loadable images, by populating and
 * linking each `bl_load_info_node_t` type node, using the internal array
 * of image descriptor provided by bl_mem_params_desc_ptr. It also populates
 * and returns `bl_load_info_t` type structure that contains head of the list
 * of loadable images.
 ******************************************************************************/
bl_load_info_t *get_bl_load_info_from_mem_params_desc(void)
{
	unsigned int index = 0;

	/* If there is no image to start with, return NULL */
	if (bl_mem_params_desc_num == 0U)
		return NULL;

	/* Assign initial data structures */
	bl_load_info_node_t *bl_node_info =
		&bl_mem_params_desc_ptr[index].load_node_mem;
	bl_load_info.head = bl_node_info;
	SET_PARAM_HEAD(&bl_load_info, PARAM_BL_LOAD_INFO, VERSION_2, 0U);

	/* Go through the image descriptor array and create the list */
	for (; index < bl_mem_params_desc_num; index++) {

		/* Populate the image information */
		bl_node_info->image_id = bl_mem_params_desc_ptr[index].image_id;
		bl_node_info->image_info = &bl_mem_params_desc_ptr[index].image_info;

		/* Link next image if present */
		if ((index + 1U) < bl_mem_params_desc_num) {
			/* Get the memory and link the next node */
			bl_node_info->next_load_info =
				&bl_mem_params_desc_ptr[index + 1U].load_node_mem;
			bl_node_info = bl_node_info->next_load_info;
		}
	}

	return &bl_load_info;
}

/*******************************************************************************
 * This function creates the list of executable images, by populating and
 * linking each `bl_params_node_t` type node, using the internal array of
 * image descriptor provided by bl_mem_params_desc_ptr. It also populates
 * and returns `bl_params_t` type structure that contains head of the list
 * of executable images.
 ******************************************************************************/
bl_params_t *get_next_bl_params_from_mem_params_desc(void)
{
	unsigned int count;
	unsigned int img_id = 0U;
	unsigned int link_index = 0U;
	bl_params_node_t *bl_current_exec_node = NULL;
	bl_params_node_t *bl_last_exec_node = NULL;
	bl_mem_params_node_t *desc_ptr;

	/* If there is no image to start with, return NULL */
	if (bl_mem_params_desc_num == 0U)
		return NULL;

	/* Get the list HEAD */
	for (count = 0U; count < bl_mem_params_desc_num; count++) {

		desc_ptr = &bl_mem_params_desc_ptr[count];

		if ((EP_GET_EXE(desc_ptr->ep_info.h.attr) == EXECUTABLE) &&
			(EP_GET_FIRST_EXE(desc_ptr->ep_info.h.attr) == EP_FIRST_EXE)) {
			next_bl_params.head = &desc_ptr->params_node_mem;
			link_index = count;
			break;
		}
	}

	/* Make sure we have a HEAD node */
	assert(next_bl_params.head != NULL);

	/* Populate the HEAD information */
	SET_PARAM_HEAD(&next_bl_params, PARAM_BL_PARAMS, VERSION_2, 0U);

	/*
	 * Go through the image descriptor array and create the list.
	 * This bounded loop is to make sure that we are not looping forever.
	 */
	for (count = 0U; count < bl_mem_params_desc_num; count++) {

		desc_ptr = &bl_mem_params_desc_ptr[link_index];

		/* Make sure the image is executable */
		assert(EP_GET_EXE(desc_ptr->ep_info.h.attr) == EXECUTABLE);

		/* Get the memory for current node */
		bl_current_exec_node = &desc_ptr->params_node_mem;

		/* Populate the image information */
		bl_current_exec_node->image_id = desc_ptr->image_id;
		bl_current_exec_node->image_info = &desc_ptr->image_info;
		bl_current_exec_node->ep_info = &desc_ptr->ep_info;

		if (bl_last_exec_node != NULL) {
			/* Assert if loop detected */
			assert(bl_last_exec_node->next_params_info == NULL);

			/* Link the previous node to the current one */
			bl_last_exec_node->next_params_info = bl_current_exec_node;
		}

		/* Update the last node */
		bl_last_exec_node = bl_current_exec_node;

		/* If no next hand-off image then break out */
		img_id = desc_ptr->next_handoff_image_id;
		if (img_id == INVALID_IMAGE_ID)
			break;

		/* Get the index for the next hand-off image */
		link_index = get_bl_params_node_index(img_id);
		assert((link_index > 0U) &&
			(link_index < bl_mem_params_desc_num));
	}

	/* Invalid image is expected to terminate the loop */
	assert(img_id == INVALID_IMAGE_ID);

	return &next_bl_params;
}

/*******************************************************************************
 * This function populates the entry point information with the corresponding
 * config file for all executable BL images described in bl_params.
 ******************************************************************************/
void populate_next_bl_params_config(bl_params_t *bl2_to_next_bl_params)
{
	bl_params_node_t *params_node;
	unsigned int fw_config_id;
	uintptr_t hw_config_base = 0, fw_config_base;
	bl_mem_params_node_t *mem_params;

	assert(bl2_to_next_bl_params != NULL);

	/*
	 * Get the `bl_mem_params_node_t` corresponding to HW_CONFIG
	 * if available.
	 */
	mem_params = get_bl_mem_params_node(HW_CONFIG_ID);
	if (mem_params != NULL)
		hw_config_base = mem_params->image_info.image_base;

	for (params_node = bl2_to_next_bl_params->head; params_node != NULL;
			params_node = params_node->next_params_info) {

		fw_config_base = 0;

		switch (params_node->image_id) {
		case BL31_IMAGE_ID:
			fw_config_id = SOC_FW_CONFIG_ID;
			break;
		case BL32_IMAGE_ID:
			fw_config_id = TOS_FW_CONFIG_ID;
			break;
		case BL33_IMAGE_ID:
			fw_config_id = NT_FW_CONFIG_ID;
			break;
		default:
			fw_config_id = INVALID_IMAGE_ID;
			break;
		}

		if (fw_config_id != INVALID_IMAGE_ID) {
			mem_params = get_bl_mem_params_node(fw_config_id);
			if (mem_params != NULL)
				fw_config_base = mem_params->image_info.image_base;
		}

		/*
		 * Pass hw and tb_fw config addresses to next images. NOTE - for
		 * EL3 runtime images (BL31 for AArch64 and BL32 for AArch32),
		 * arg0 is already used by generic code. Take care of not
		 * overwriting the previous initialisations.
		 */
		if (params_node == bl2_to_next_bl_params->head) {
			if (params_node->ep_info->args.arg1 == 0U)
				params_node->ep_info->args.arg1 =
								fw_config_base;
			if (params_node->ep_info->args.arg2 == 0U)
				params_node->ep_info->args.arg2 =
								hw_config_base;
		} else {
			if (params_node->ep_info->args.arg0 == 0U)
				params_node->ep_info->args.arg0 =
								fw_config_base;
			if (params_node->ep_info->args.arg1 == 0U)
				params_node->ep_info->args.arg1 =
								hw_config_base;
		}
	}
}

/*******************************************************************************
 * Helper to extract BL32/BL33 entry point info from arg0 passed to BL31, for
 * platforms that are only interested in those. Platforms that need to extract
 * more information can parse the structures themselves.
 ******************************************************************************/

void bl31_params_parse_helper(u_register_t param,
			      entry_point_info_t *bl32_ep_info_out,
			      entry_point_info_t *bl33_ep_info_out)
{
	bl_params_node_t *node;
	bl_params_t *v2 = (void *)(uintptr_t)param;

#if !ERROR_DEPRECATED
	if (v2->h.version == PARAM_VERSION_1) {
		struct { /* Deprecated version 1 parameter structure. */
			param_header_t h;
			image_info_t *bl31_image_info;
			entry_point_info_t *bl32_ep_info;
			image_info_t *bl32_image_info;
			entry_point_info_t *bl33_ep_info;
			image_info_t *bl33_image_info;
		} *v1 = (void *)(uintptr_t)param;
		assert(v1->h.type == PARAM_BL31);
		if (bl32_ep_info_out)
			*bl32_ep_info_out = *v1->bl32_ep_info;
		if (bl33_ep_info_out)
			*bl33_ep_info_out = *v1->bl33_ep_info;
		return;
	}
#endif /* !ERROR_DEPRECATED */

	assert(v2->h.version == PARAM_VERSION_2);
	assert(v2->h.type == PARAM_BL_PARAMS);
	for (node = v2->head; node; node = node->next_params_info) {
		if (node->image_id == BL32_IMAGE_ID)
			if (bl32_ep_info_out)
				*bl32_ep_info_out = *node->ep_info;
		if (node->image_id == BL33_IMAGE_ID)
			if (bl33_ep_info_out)
				*bl33_ep_info_out = *node->ep_info;
	}
}
