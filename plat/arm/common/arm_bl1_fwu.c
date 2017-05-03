/*
 * Copyright (c) 2015-2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <bl_common.h>
#include <debug.h>
#include <errno.h>
#include <plat_arm.h>
#include <platform_def.h>
#include <tbbr_img_desc.h>
#include <utils.h>

/* Struct to keep track of usable memory */
typedef struct bl1_mem_info {
	uintptr_t mem_base;
	unsigned int mem_size;
} bl1_mem_info_t;

bl1_mem_info_t fwu_addr_map_secure[] = {
	{
		.mem_base = ARM_SHARED_RAM_BASE,
		.mem_size = ARM_SHARED_RAM_SIZE
	},
	{
		.mem_size = 0
	}
};

bl1_mem_info_t fwu_addr_map_non_secure[] = {
	{
		.mem_base = ARM_NS_DRAM1_BASE,
		.mem_size = ARM_NS_DRAM1_SIZE
	},
	{
		.mem_base = PLAT_ARM_NVM_BASE,
		.mem_size = PLAT_ARM_NVM_SIZE
	},
	{
		.mem_size = 0
	}
};

int bl1_plat_mem_check(uintptr_t mem_base,
		unsigned int mem_size,
		unsigned int flags)
{
	unsigned int index = 0;
	bl1_mem_info_t *mmap;

	assert(mem_base);
	assert(mem_size);
	/*
	 * The caller of this function is responsible for checking upfront that
	 * the end address doesn't overflow. We double-check this in debug
	 * builds.
	 */
	assert(!check_uptr_overflow(mem_base, mem_size - 1));

	/*
	 * Check the given image source and size.
	 */
	if (GET_SECURITY_STATE(flags) == SECURE)
		mmap = fwu_addr_map_secure;
	else
		mmap = fwu_addr_map_non_secure;

	while (mmap[index].mem_size) {
		if ((mem_base >= mmap[index].mem_base) &&
			((mem_base + mem_size)
			<= (mmap[index].mem_base +
			mmap[index].mem_size)))
			return 0;

		index++;
	}

	return -ENOMEM;
}

/*******************************************************************************
 * This function does linear search for image_id and returns image_desc.
 ******************************************************************************/
image_desc_t *bl1_plat_get_image_desc(unsigned int image_id)
{
	unsigned int index = 0;

	while (bl1_tbbr_image_descs[index].image_id != INVALID_IMAGE_ID) {
		if (bl1_tbbr_image_descs[index].image_id == image_id)
			return &bl1_tbbr_image_descs[index];
		index++;
	}

	return NULL;
}
