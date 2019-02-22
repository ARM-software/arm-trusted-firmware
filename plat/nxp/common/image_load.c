/*
 * Copyright 2018-2019 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <assert.h>

#include <common/bl_common.h>
#include <common/desc_image_load.h>
#include <lib/xlat_tables/xlat_tables_v2.h>

#include <plat_common.h>

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
	return get_next_bl_params_from_mem_params_desc();
}

/******************************************************************************
 * This function can be used to load DDR PHY/FUSE Images
 *
 * @param [in] image_id		 Image ID to be loaded
 *
 * @param [in,out]  image_base   Location at which the image should be loaded
 *				 In case image is prepended by a CSF header,
 *				 image_base is pointer to actual image after
 *				 the header
 *
 * @param [in,out]  image_size   User should pass the maximum size of the image
 *				 possible.(Buffer size starting from image_base)
 *				 Actual size of the image loaded is returned
 *				 back.
 *****************************************************************************/
int load_img(unsigned int image_id, uintptr_t *image_base,
		      uint32_t *image_size)
{
	int err = 0;

	image_desc_t img_info = {
		.image_id = image_id,
		SET_STATIC_PARAM_HEAD(image_info, PARAM_IMAGE_BINARY,
				VERSION_2, image_info_t, 0),
		.image_info.image_base = *image_base,
		.image_info.image_max_size = *image_size,
	};

	/* Create MMU entry for the CSF header */

	VERBOSE("BL2: Loading IMG %d\n", image_id);
	err = load_auth_image(image_id, &img_info.image_info);
	if (err != 0) {
		VERBOSE("Failed to load IMG %d\n", image_id);
		return err;
	}

	*image_base = img_info.image_info.image_base;
	*image_size = img_info.image_info.image_size;

	return err;
}
