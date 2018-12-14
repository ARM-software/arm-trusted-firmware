/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>

#include <platform_def.h>

#include <arch_helpers.h>
#include <bl1/bl1.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <plat/common/platform.h>

/*
 * The following platform functions are weakly defined. They
 * are default implementations that allow BL1 to compile in
 * absence of real definitions. The Platforms may override
 * with more complex definitions.
 */
#pragma weak bl1_plat_get_next_image_id
#pragma weak bl1_plat_set_ep_info
#pragma weak bl1_plat_get_image_desc
#pragma weak bl1_plat_fwu_done
#pragma weak bl1_plat_handle_pre_image_load
#pragma weak bl1_plat_handle_post_image_load


unsigned int bl1_plat_get_next_image_id(void)
{
	/* BL2 load will be done by default. */
	return BL2_IMAGE_ID;
}

void bl1_plat_set_ep_info(unsigned int image_id,
		struct entry_point_info *ep_info)
{

}

int bl1_plat_handle_pre_image_load(unsigned int image_id)
{
	return 0;
}

/*
 * Following is the default definition that always
 * returns BL2 image details.
 */
struct image_desc *bl1_plat_get_image_desc(unsigned int image_id)
{
	static image_desc_t bl2_img_desc = BL2_IMAGE_DESC;
	return &bl2_img_desc;
}

__dead2 void bl1_plat_fwu_done(void *client_cookie, void *reserved)
{
	while (1)
		wfi();
}

/*
 * The Platforms must override with real definition.
 */
#pragma weak bl1_plat_mem_check

int bl1_plat_mem_check(uintptr_t mem_base, unsigned int mem_size,
		unsigned int flags)
{
	assert(0);
	return -ENOMEM;
}

/*
 * Default implementation for bl1_plat_handle_post_image_load(). This function
 * populates the default arguments to BL2. The BL2 memory layout structure
 * is allocated and the calculated layout is populated in arg1 to BL2.
 */
int bl1_plat_handle_post_image_load(unsigned int image_id)
{
	meminfo_t *bl2_tzram_layout;
	meminfo_t *bl1_tzram_layout;
	image_desc_t *image_desc;
	entry_point_info_t *ep_info;

	if (image_id != BL2_IMAGE_ID)
		return 0;

	/* Get the image descriptor */
	image_desc = bl1_plat_get_image_desc(BL2_IMAGE_ID);
	assert(image_desc != NULL);

	/* Get the entry point info */
	ep_info = &image_desc->ep_info;

	/* Find out how much free trusted ram remains after BL1 load */
	bl1_tzram_layout = bl1_plat_sec_mem_layout();

	/*
	 * Create a new layout of memory for BL2 as seen by BL1 i.e.
	 * tell it the amount of total and free memory available.
	 * This layout is created at the first free address visible
	 * to BL2. BL2 will read the memory layout before using its
	 * memory for other purposes.
	 */
	bl2_tzram_layout = (meminfo_t *) bl1_tzram_layout->total_base;

	bl1_calc_bl2_mem_layout(bl1_tzram_layout, bl2_tzram_layout);

	ep_info->args.arg1 = (uintptr_t)bl2_tzram_layout;

	VERBOSE("BL1: BL2 memory layout address = %p\n",
		(void *) bl2_tzram_layout);
	return 0;
}
