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

#include <assert.h>
#include <bl_common.h>
#include <desc_image_load.h>
#include <plat_arm.h>

#if JUNO_AARCH32_EL3_RUNTIME
/*******************************************************************************
 * This function changes the spsr for BL32 image to bypass
 * the check in BL1 AArch64 exception handler. This is needed in the aarch32
 * boot flow as the core comes up in aarch64 and to enter the BL32 image a warm
 * reset in aarch32 state is required.
 ******************************************************************************/
int bl2_plat_handle_post_image_load(unsigned int image_id)
{
	int err = arm_bl2_handle_post_image_load(image_id);

	if (!err && (image_id == BL32_IMAGE_ID)) {
		bl_mem_params_node_t *bl_mem_params = get_bl_mem_params_node(image_id);
		assert(bl_mem_params);
		bl_mem_params->ep_info.spsr = SPSR_64(MODE_EL3, MODE_SP_ELX,
			DISABLE_ALL_EXCEPTIONS);
	}

	return err;
}
#endif /* JUNO_AARCH32_EL3_RUNTIME */
