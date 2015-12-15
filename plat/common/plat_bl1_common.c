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
