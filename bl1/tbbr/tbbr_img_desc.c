/*
 * Copyright (c) 2015-2016, ARM Limited and Contributors. All rights reserved.
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

#include <bl1.h>
#include <bl_common.h>
#include <platform_def.h>

image_desc_t bl1_tbbr_image_descs[] = {
    {
	    .image_id = FWU_CERT_ID,
	    SET_STATIC_PARAM_HEAD(image_info, PARAM_IMAGE_BINARY,
		    VERSION_1, image_info_t, 0),
	    .image_info.image_base = BL2_BASE,
	    SET_STATIC_PARAM_HEAD(ep_info, PARAM_IMAGE_BINARY,
		    VERSION_1, entry_point_info_t, SECURE),
    },
#if NS_BL1U_BASE
    {
	    .image_id = NS_BL1U_IMAGE_ID,
	    SET_STATIC_PARAM_HEAD(ep_info, PARAM_EP,
		    VERSION_1, entry_point_info_t, NON_SECURE | EXECUTABLE),
	    .ep_info.pc = NS_BL1U_BASE,
    },
#endif
#if SCP_BL2U_BASE
    {
	    .image_id = SCP_BL2U_IMAGE_ID,
	    SET_STATIC_PARAM_HEAD(image_info, PARAM_IMAGE_BINARY,
		    VERSION_1, image_info_t, 0),
	    .image_info.image_base = SCP_BL2U_BASE,
	    SET_STATIC_PARAM_HEAD(ep_info, PARAM_IMAGE_BINARY,
		    VERSION_1, entry_point_info_t, SECURE),
    },
#endif
#if BL2U_BASE
    {
	    .image_id = BL2U_IMAGE_ID,
	    SET_STATIC_PARAM_HEAD(image_info, PARAM_EP,
		    VERSION_1, image_info_t, 0),
	    .image_info.image_base = BL2U_BASE,
	    SET_STATIC_PARAM_HEAD(ep_info, PARAM_EP,
		    VERSION_1, entry_point_info_t, SECURE | EXECUTABLE),
	    .ep_info.pc = BL2U_BASE,
    },
#endif
#if NS_BL2U_BASE
    {
	    .image_id = NS_BL2U_IMAGE_ID,
	    SET_STATIC_PARAM_HEAD(ep_info, PARAM_EP,
		    VERSION_1, entry_point_info_t, NON_SECURE),
    },
#endif
	    BL2_IMAGE_DESC,

    {
	    .image_id = INVALID_IMAGE_ID,
    }
};
