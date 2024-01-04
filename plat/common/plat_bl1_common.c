/*
 * Copyright (c) 2015-2024, Arm Limited and Contributors. All rights reserved.
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
	while (true)
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
	meminfo_t *bl1_tzram_layout;
	image_desc_t *image_desc;

	if (image_id != BL2_IMAGE_ID)
		return 0;

	/* Get the image descriptor */
	image_desc = bl1_plat_get_image_desc(BL2_IMAGE_ID);
	assert(image_desc != NULL);

	/* Find out how much free trusted ram remains after BL1 load */
	bl1_tzram_layout = bl1_plat_sec_mem_layout();

	/*
	 * Convey this information to BL2 by storing the layout at the first free
	 * address visible to BL2.
	 */
	bl1_plat_calc_bl2_layout(bl1_tzram_layout,
				 (meminfo_t *)bl1_tzram_layout->total_base);

	image_desc->ep_info.args.arg1 = (uintptr_t)bl1_tzram_layout->total_base;

	VERBOSE("BL1: BL2 memory layout address = %p\n",
		(void *)image_desc->ep_info.args.arg1);

	return 0;
}

/*******************************************************************************
 * Helper utility to calculate the BL2 memory layout taking into consideration
 * the BL1 RW data assuming that it is at the top of the memory layout.
 ******************************************************************************/
void bl1_plat_calc_bl2_layout(const meminfo_t *bl1_mem_layout,
			      meminfo_t *bl2_mem_layout)
{
	assert(bl1_mem_layout != NULL);
	assert(bl2_mem_layout != NULL);

	/*
	 * Remove BL1 RW data from the scope of memory visible to BL2.
	 * This is assuming BL1 RW data is at the top of bl1_mem_layout.
	 */
	assert(BL1_RW_BASE > bl1_mem_layout->total_base);
	bl2_mem_layout->total_base = bl1_mem_layout->total_base;
	bl2_mem_layout->total_size = BL1_RW_BASE - bl1_mem_layout->total_base;

	flush_dcache_range((uintptr_t)bl2_mem_layout, sizeof(meminfo_t));
}
