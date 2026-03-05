/*
 * Copyright (c) 2015-2026, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <inttypes.h>

#include <platform_def.h>

#include <bl1/tbbr/tbbr_img_desc.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <lib/cassert.h>
#include <lib/utils.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>

#pragma weak bl1_plat_get_image_desc

/* Compile-time checks for static FWU map entries. */
CASSERT(!check_uptr_overflow(ARM_SHARED_RAM_BASE, ARM_SHARED_RAM_SIZE),
	assert_fwu_secure_ram_overflow);
CASSERT(!check_uptr_overflow(ARM_NS_DRAM1_BASE, ARM_NS_DRAM1_SIZE),
	assert_fwu_ns_dram1_overflow);
CASSERT(!check_uptr_overflow(PLAT_ARM_NVM_BASE, PLAT_ARM_NVM_SIZE),
	assert_fwu_ns_nvm_overflow);

/* Struct to keep track of usable memory */
typedef struct bl1_mem_info {
	uintptr_t mem_base;
	unsigned int mem_size;
} bl1_mem_info_t;

static const bl1_mem_info_t fwu_addr_map_secure[] = {
	{
		.mem_base = ARM_SHARED_RAM_BASE,
		.mem_size = ARM_SHARED_RAM_SIZE
	},
	{
		.mem_size = 0
	}
};

static const bl1_mem_info_t fwu_addr_map_non_secure[] = {
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
	const bl1_mem_info_t *mmap;
	uintptr_t end;
	uintptr_t region_end;

	/*
	 * The caller of this function is responsible for checking upfront that
	 * the end address doesn't overflow builds. We double check this.
	 */
	if ((mem_base == 0U) || (mem_size == 0U) ||
	    check_uptr_overflow(mem_base, mem_size)) {
		return -ENOMEM;
	}

	end = mem_base + mem_size;

	/*
	 * Check the given image source and size.
	 */
	if (GET_SECURITY_STATE(flags) == SECURE) {
		mmap = fwu_addr_map_secure;
	} else {
		mmap = fwu_addr_map_non_secure;
	}

	while (mmap[index].mem_size > 0U) {
		region_end = mmap[index].mem_base + mmap[index].mem_size;

		if ((mem_base >= mmap[index].mem_base) &&
		    (end <= region_end)) {
			return 0;
		}

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
