/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <assert.h>
#include <bl_common.h>
#include <debug.h>
#include <errno.h>
#include <platform_def.h>

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


unsigned int bl1_plat_get_next_image_id(void)
{
	/* BL2 load will be done by default. */
	return BL2_IMAGE_ID;
}

void bl1_plat_set_ep_info(unsigned int image_id,
		entry_point_info_t *ep_info)
{

}

/*
 * Following is the default definition that always
 * returns BL2 image details.
 */
image_desc_t *bl1_plat_get_image_desc(unsigned int image_id)
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
