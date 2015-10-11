/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
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

#include <assert.h>
#include <bl_common.h>
#include <debug.h>
#include <errno.h>
#include <plat_arm.h>
#include <tbbr_img_desc.h>


/* Struct to keep track of usable memory */
typedef struct bl1_mem_info{
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
		.mem_base = V2M_FLASH0_BASE,
		.mem_size = V2M_FLASH0_SIZE
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
	 * Check the given image source and size.
	 */
	if (GET_SEC_STATE(flags) == SECURE)
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
