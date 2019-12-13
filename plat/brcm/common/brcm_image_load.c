/*
 * Copyright (c) 2019-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/bl_common.h>
#include <common/desc_image_load.h>
#include <plat/common/platform.h>

#pragma weak plat_flush_next_bl_params
#pragma weak plat_get_bl_image_load_info
#pragma weak plat_get_next_bl_params

/*******************************************************************************
 * This function flushes the data structures so that they are visible
 * in memory for the next BL image.
 ******************************************************************************/
void plat_flush_next_bl_params(void)
{
	flush_bl_params_desc();
}

/*******************************************************************************
 * This function returns the list of loadable images.
 ******************************************************************************/
struct bl_load_info *plat_get_bl_image_load_info(void)
{
	return get_bl_load_info_from_mem_params_desc();
}

/*******************************************************************************
 * This function returns the list of executable images.
 ******************************************************************************/
struct bl_params *plat_get_next_bl_params(void)
{
	bl_params_t *next_bl_params = get_next_bl_params_from_mem_params_desc();

	populate_next_bl_params_config(next_bl_params);
	return next_bl_params;
}
