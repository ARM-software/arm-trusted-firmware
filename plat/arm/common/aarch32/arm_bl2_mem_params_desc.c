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

#include <bl_common.h>
#include <desc_image_load.h>
#include <platform.h>
#include <platform_def.h>


/*******************************************************************************
 * Following descriptor provides BL image/ep information that gets used
 * by BL2 to load the images and also subset of this information is
 * passed to next BL image. The image loading sequence is managed by
 * populating the images in required loading order. The image execution
 * sequence is managed by populating the `next_handoff_image_id` with
 * the next executable image id.
 ******************************************************************************/
static bl_mem_params_node_t bl2_mem_params_descs[] = {
#ifdef SCP_BL2_BASE
	/* Fill SCP_BL2 related information if it exists */
    {
	    .image_id = SCP_BL2_IMAGE_ID,

	    SET_STATIC_PARAM_HEAD(ep_info, PARAM_IMAGE_BINARY,
		    VERSION_2, entry_point_info_t, SECURE | NON_EXECUTABLE),

	    SET_STATIC_PARAM_HEAD(image_info, PARAM_IMAGE_BINARY,
		    VERSION_2, image_info_t, 0),
	    .image_info.image_base = SCP_BL2_BASE,
	    .image_info.image_max_size = PLAT_CSS_MAX_SCP_BL2_SIZE,

	    .next_handoff_image_id = INVALID_IMAGE_ID,
    },
#endif /* SCP_BL2_BASE */

	/* Fill BL32 related information */
    {
	    .image_id = BL32_IMAGE_ID,

	    SET_STATIC_PARAM_HEAD(ep_info, PARAM_EP,
		    VERSION_2, entry_point_info_t,
		    SECURE | EXECUTABLE | EP_FIRST_EXE),
	    .ep_info.pc = BL32_BASE,
	    .ep_info.spsr = SPSR_MODE32(MODE32_mon, SPSR_T_ARM,
		    SPSR_E_LITTLE, DISABLE_ALL_EXCEPTIONS),

	    SET_STATIC_PARAM_HEAD(image_info, PARAM_EP,
		    VERSION_2, image_info_t, IMAGE_ATTRIB_PLAT_SETUP),
	    .image_info.image_base = BL32_BASE,
	    .image_info.image_max_size = BL32_LIMIT - BL32_BASE,

	    .next_handoff_image_id = BL33_IMAGE_ID,
    },

	/* Fill BL33 related information */
    {
	    .image_id = BL33_IMAGE_ID,

	    SET_STATIC_PARAM_HEAD(ep_info, PARAM_EP,
		    VERSION_2, entry_point_info_t, NON_SECURE | EXECUTABLE),
#ifdef PRELOADED_BL33_BASE
	    .ep_info.pc = PRELOADED_BL33_BASE,

	    SET_STATIC_PARAM_HEAD(image_info, PARAM_EP,
		    VERSION_2, image_info_t, IMAGE_ATTRIB_SKIP_LOADING),
#else
	    .ep_info.pc = PLAT_ARM_NS_IMAGE_OFFSET,

	    SET_STATIC_PARAM_HEAD(image_info, PARAM_EP,
		    VERSION_2, image_info_t, 0),
	    .image_info.image_base = PLAT_ARM_NS_IMAGE_OFFSET,
	    .image_info.image_max_size = ARM_DRAM1_SIZE,
#endif /* PRELOADED_BL33_BASE */

	    .next_handoff_image_id = INVALID_IMAGE_ID,
    }
};

REGISTER_BL_IMAGE_DESCS(bl2_mem_params_descs)
