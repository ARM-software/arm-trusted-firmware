/*
 * Copyright (c) 2024, Pengutronix, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <errno.h>
#include <stdint.h>

#include <common/bl_common.h>
#include <common/desc_image_load.h>

#include <plat_common.h>

/*
 * This function checks if @arg0 can safely be accessed as a pointer
 * and if it does, it fills in @bl32_info and @bl33_info with data
 * found in @arg0.
 *
 * Returns 0 when @arg0 can be used as entry point info and a negative
 * error code otherwise.
 */
int imx_bl31_params_parse(uintptr_t arg0, uintptr_t ocram_base,
			  uintptr_t ocram_size,
			  entry_point_info_t *bl32_info,
			  entry_point_info_t *bl33_info)
{
	bl_params_t *v2 = (void *)(uintptr_t)arg0;

	if (arg0 & 0x3) {
		return -EINVAL;
	}

	if (arg0 < ocram_base || arg0 >= ocram_base + ocram_size) {
		return -EINVAL;
	}

	if (v2->h.version != PARAM_VERSION_2) {
		return -EINVAL;
	}

	if (v2->h.type != PARAM_BL_PARAMS) {
		return -EINVAL;
	}

	bl31_params_parse_helper(arg0, bl32_info, bl33_info);

	return 0;
}
