/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <arch_helpers.h>
#include <assert.h>
#include <bl_common.h>
#include <desc_image_load.h>


extern bl_mem_params_node_t *bl_mem_params_desc_ptr;
extern unsigned int bl_mem_params_desc_num;

static bl_load_info_t bl_load_info;
static bl_params_t next_bl_params;


/*******************************************************************************
 * This function flushes the data structures so that they are visible
 * in memory for the next BL image.
 ******************************************************************************/
void flush_bl_params_desc(void)
{
	flush_dcache_range((unsigned long)bl_mem_params_desc_ptr,
			sizeof(*bl_mem_params_desc_ptr) * bl_mem_params_desc_num);
}

/*******************************************************************************
 * This function returns the index for given image_id, within the
 * image descriptor array provided by bl_image_info_descs_ptr, if the
 * image is found else it returns -1.
 ******************************************************************************/
int get_bl_params_node_index(unsigned int image_id)
{
	int index;
	assert(image_id != INVALID_IMAGE_ID);

	for (index = 0; index < bl_mem_params_desc_num; index++) {
		if (bl_mem_params_desc_ptr[index].image_id == image_id)
			return index;
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
	int index = 0;

	/* If there is no image to start with, return NULL */
	if (!bl_mem_params_desc_num)
		return NULL;

	/* Assign initial data structures */
	bl_load_info_node_t *bl_node_info =
		&bl_mem_params_desc_ptr[index].load_node_mem;
	bl_load_info.head = bl_node_info;
	SET_PARAM_HEAD(&bl_load_info, PARAM_BL_LOAD_INFO, VERSION_2, 0);

	/* Go through the image descriptor array and create the list */
	for (; index < bl_mem_params_desc_num; index++) {

		/* Populate the image information */
		bl_node_info->image_id = bl_mem_params_desc_ptr[index].image_id;
		bl_node_info->image_info = &bl_mem_params_desc_ptr[index].image_info;

		/* Link next image if present */
		if ((index + 1) < bl_mem_params_desc_num) {
			/* Get the memory and link the next node */
			bl_node_info->next_load_info =
				&bl_mem_params_desc_ptr[index + 1].load_node_mem;
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
	int count;
	unsigned int img_id = 0;
	int link_index = 0;
	bl_params_node_t *bl_current_exec_node = NULL;
	bl_params_node_t *bl_last_exec_node = NULL;
	bl_mem_params_node_t *desc_ptr;

	/* If there is no image to start with, return NULL */
	if (!bl_mem_params_desc_num)
		return NULL;

	/* Get the list HEAD */
	for (count = 0; count < bl_mem_params_desc_num; count++) {

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
	SET_PARAM_HEAD(&next_bl_params, PARAM_BL_PARAMS, VERSION_2, 0);

	/*
	 * Go through the image descriptor array and create the list.
	 * This bounded loop is to make sure that we are not looping forever.
	 */
	for (count = 0 ; count < bl_mem_params_desc_num; count++) {

		desc_ptr = &bl_mem_params_desc_ptr[link_index];

		/* Make sure the image is executable */
		assert(EP_GET_EXE(desc_ptr->ep_info.h.attr) == EXECUTABLE);

		/* Get the memory for current node */
		bl_current_exec_node = &desc_ptr->params_node_mem;

		/* Populate the image information */
		bl_current_exec_node->image_id = desc_ptr->image_id;
		bl_current_exec_node->image_info = &desc_ptr->image_info;
		bl_current_exec_node->ep_info = &desc_ptr->ep_info;

		if (bl_last_exec_node) {
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
		assert((link_index > 0) &&
			(link_index < bl_mem_params_desc_num));
	}

	/* Invalid image is expected to terminate the loop */
	assert(img_id == INVALID_IMAGE_ID);

	/* Populate arg0 for the next BL image */
	next_bl_params.head->ep_info->args.arg0 = (unsigned long)&next_bl_params;

	/* Flush the parameters to be passed to the next BL image */
	flush_dcache_range((unsigned long)&next_bl_params,
			sizeof(next_bl_params));

	return &next_bl_params;
}
