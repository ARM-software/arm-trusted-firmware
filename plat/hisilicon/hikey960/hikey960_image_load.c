/*
 * Copyright (c) 2017-2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <common/bl_common.h>
#include <common/desc_image_load.h>
#include <plat/common/platform.h>

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
bl_load_info_t *plat_get_bl_image_load_info(void)
{
	return get_bl_load_info_from_mem_params_desc();
}


/*******************************************************************************
 * ARM helper function to return the list of executable images. Since the default
 * descriptors are allocated within BL2 RW memory, this prevents BL31/BL32
 * overlay of BL2 memory. Hence this function also copies the descriptors to a
 * pre-allocated memory indicated by ARM_BL2_MEM_DESC_BASE.
 ******************************************************************************/
struct bl_params *hikey960_get_next_bl_params(void)
{
	bl_params_t *next_bl_params;

	next_bl_params = get_next_bl_params_from_mem_params_desc();
	assert(next_bl_params != NULL);

	populate_next_bl_params_config(next_bl_params);

	return next_bl_params;
}


/*******************************************************************************
 * This function returns the list of executable images.
 ******************************************************************************/
bl_params_t *plat_get_next_bl_params(void)
{
	return hikey960_get_next_bl_params();
}
