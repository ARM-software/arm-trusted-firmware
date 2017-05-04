/*
 * Copyright (c) 2015-2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
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
#if LOAD_IMAGE_V2
	    .image_info.image_max_size = BL2_LIMIT - BL2_BASE,
#endif
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
#if LOAD_IMAGE_V2
	    .image_info.image_max_size = SCP_BL2U_LIMIT - SCP_BL2U_BASE,
#endif
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
#if LOAD_IMAGE_V2
	    .image_info.image_max_size = BL2U_LIMIT - BL2U_BASE,
#endif
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
