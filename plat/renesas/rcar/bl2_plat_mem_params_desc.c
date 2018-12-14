/*
 * Copyright (c) 2016-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>

#include <common/bl_common.h>
#include <common/desc_image_load.h>
#include <lib/xlat_tables/xlat_tables_defs.h>
#include <plat/common/platform.h>

#if (RCAR_BL33_EXECUTION_EL != 0) && (RCAR_BL33_EXECUTION_EL != 1)
#error
#endif

#if (RCAR_BL33_EXECUTION_EL == 0)
#define BL33_MODE MODE_EL1
#else
#define BL33_MODE MODE_EL2
#endif

extern uint64_t fdt_blob[PAGE_SIZE_4KB / sizeof(uint64_t)];

static bl_mem_params_node_t bl2_mem_params_descs[] = {
	{
		.image_id = BL31_IMAGE_ID,

		SET_STATIC_PARAM_HEAD(ep_info, PARAM_EP, VERSION_2,
			entry_point_info_t, SECURE | EXECUTABLE | EP_FIRST_EXE),
		.ep_info.spsr = SPSR_64(MODE_EL3,
			MODE_SP_ELX, DISABLE_ALL_EXCEPTIONS),
		.ep_info.pc = BL31_BASE,


		SET_STATIC_PARAM_HEAD(image_info, PARAM_EP, VERSION_2,
			image_info_t, IMAGE_ATTRIB_PLAT_SETUP),
		.image_info.image_max_size = BL31_LIMIT - BL31_BASE,
		.image_info.image_base = BL31_BASE,

# ifdef BL32_BASE
		.next_handoff_image_id = BL32_IMAGE_ID,
# else
		.next_handoff_image_id = BL33_IMAGE_ID,
# endif
	},
# ifdef BL32_BASE
	{
		.image_id = BL32_IMAGE_ID,

		SET_STATIC_PARAM_HEAD(ep_info, PARAM_EP, VERSION_2,
			entry_point_info_t, SECURE | EXECUTABLE),
		.ep_info.pc = BL32_BASE,
		.ep_info.spsr = 0,

		SET_STATIC_PARAM_HEAD(image_info, PARAM_EP, VERSION_2,
			image_info_t, 0),
		.image_info.image_max_size = BL32_LIMIT - BL32_BASE,
		.image_info.image_base = BL32_BASE,

		.next_handoff_image_id = BL33_IMAGE_ID,
	},
#endif
	{
		.image_id = BL33_IMAGE_ID,

		SET_STATIC_PARAM_HEAD(ep_info, PARAM_EP, VERSION_2,
			entry_point_info_t, NON_SECURE | EXECUTABLE),
		.ep_info.spsr = SPSR_64(BL33_MODE, MODE_SP_ELX,
			DISABLE_ALL_EXCEPTIONS),
		.ep_info.pc = BL33_BASE,
#ifdef RCAR_BL33_ARG0
		.ep_info.args.arg0 = RCAR_BL33_ARG0,
#endif
		.ep_info.args.arg1 = (uintptr_t)fdt_blob,
		SET_STATIC_PARAM_HEAD(image_info, PARAM_EP, VERSION_2,
			image_info_t, 0),
		.image_info.image_max_size =
				(uint32_t) (DRAM_LIMIT - BL33_BASE),
		.image_info.image_base = BL33_BASE,

		.next_handoff_image_id = INVALID_IMAGE_ID,
	}
};

REGISTER_BL_IMAGE_DESCS(bl2_mem_params_descs)
