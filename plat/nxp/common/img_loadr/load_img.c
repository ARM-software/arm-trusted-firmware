/*
 * Copyright 2018-2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <assert.h>

#include <common/bl_common.h>
#include <common/desc_image_load.h>
#include <lib/xlat_tables/xlat_tables_v2.h>

#include "load_img.h"

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
#ifdef CSF_HEADER_PREPENDED
		.image_info.image_base = *image_base - CSF_HDR_SZ,
		.image_info.image_max_size = *image_size + CSF_HDR_SZ,
#else
		.image_info.image_base = *image_base,
		.image_info.image_max_size = *image_size,
#endif
	};

	/* Create MMU entry for the CSF header */
#if PLAT_XLAT_TABLES_DYNAMIC
#ifdef CSF_HEADER_PREPENDED
	err = mmap_add_dynamic_region(img_info.image_info.image_base,
			img_info.image_info.image_base,
			CSF_HDR_SZ,
			MT_MEMORY | MT_RW | MT_SECURE);
	if (err != 0) {
		ERROR("Failed to add dynamic memory region.\n");
		return err;
	}
#endif
#endif

	VERBOSE("BL2: Loading IMG %d\n", image_id);
	err = load_auth_image(image_id, &img_info.image_info);
	if (err != 0) {
		VERBOSE("Failed to load IMG %d\n", image_id);
		return err;
	}

#ifdef CSF_HEADER_PREPENDED
	*image_base = img_info.image_info.image_base + CSF_HDR_SZ;
	*image_size = img_info.image_info.image_size - CSF_HDR_SZ;
#if PLAT_XLAT_TABLES_DYNAMIC
	mmap_remove_dynamic_region(img_info.image_info.image_base,
				   CSF_HDR_SZ);
#endif
#else
	*image_base = img_info.image_info.image_base;
	*image_size = img_info.image_info.image_size;
#endif

	return err;
}
