/*
 * Copyright (c) 2013-2014, ARM Limited and Contributors. All rights reserved.
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

#include <arch.h>
#include <arch_helpers.h>
#include <assert.h>
#include <bl_common.h>
#include <debug.h>
#include <platform.h>
#include <platform_def.h>
#include "bl2_private.h"

/*******************************************************************************
 * Load the BL3-0 image if there's one.
 * If a platform does not want to attempt to load BL3-0 image it must leave
 * BL30_BASE undefined.
 * Return 0 on success or if there's no BL3-0 image to load, a negative error
 * code otherwise.
 ******************************************************************************/
static int load_bl30(void)
{
	int e = 0;
#ifdef BL30_BASE
	meminfo_t bl30_mem_info;
	image_info_t bl30_image_info;

	/*
	 * It is up to the platform to specify where BL3-0 should be loaded if
	 * it exists. It could create space in the secure sram or point to a
	 * completely different memory.
	 *
	 * The entry point information is not relevant in this case as the AP
	 * won't execute the BL3-0 image.
	 */
	INFO("BL2: Loading BL3-0\n");
	bl2_plat_get_bl30_meminfo(&bl30_mem_info);
	e = load_image(&bl30_mem_info,
		       BL30_IMAGE_NAME,
		       BL30_BASE,
		       &bl30_image_info,
		       NULL);

	if (e == 0) {
		/* The subsequent handling of BL3-0 is platform specific */
		bl2_plat_handle_bl30(&bl30_image_info);
	}
#endif /* BL30_BASE */

	return e;
}

/*******************************************************************************
 * Load the BL3-1 image.
 * The bl2_to_bl31_params and bl31_ep_info params will be updated with the
 * relevant BL3-1 information.
 * Return 0 on success, a negative error code otherwise.
 ******************************************************************************/
static int load_bl31(bl31_params_t *bl2_to_bl31_params,
		     entry_point_info_t *bl31_ep_info)
{
	meminfo_t *bl2_tzram_layout;
	int e;

	INFO("BL2: Loading BL3-1\n");
	assert(bl2_to_bl31_params != NULL);
	assert(bl31_ep_info != NULL);

	/* Find out how much free trusted ram remains after BL2 load */
	bl2_tzram_layout = bl2_plat_sec_mem_layout();

	/* Set the X0 parameter to BL3-1 */
	bl31_ep_info->args.arg0 = (unsigned long)bl2_to_bl31_params;

	/* Load the BL3-1 image */
	e = load_image(bl2_tzram_layout,
		       BL31_IMAGE_NAME,
		       BL31_BASE,
		       bl2_to_bl31_params->bl31_image_info,
		       bl31_ep_info);

	if (e == 0)
		bl2_plat_set_bl31_ep_info(bl2_to_bl31_params->bl31_image_info,
					  bl31_ep_info);

	return e;
}

/*******************************************************************************
 * Load the BL3-2 image if there's one.
 * The bl2_to_bl31_params param will be updated with the relevant BL3-2
 * information.
 * If a platform does not want to attempt to load BL3-2 image it must leave
 * BL32_BASE undefined.
 * Return 0 on success or if there's no BL3-2 image to load, a negative error
 * code otherwise.
 ******************************************************************************/
static int load_bl32(bl31_params_t *bl2_to_bl31_params)
{
	int e = 0;
#ifdef BL32_BASE
	meminfo_t bl32_mem_info;

	INFO("BL2: Loading BL3-2\n");
	assert(bl2_to_bl31_params != NULL);

	/*
	 * It is up to the platform to specify where BL3-2 should be loaded if
	 * it exists. It could create space in the secure sram or point to a
	 * completely different memory.
	 */
	bl2_plat_get_bl32_meminfo(&bl32_mem_info);
	e = load_image(&bl32_mem_info,
		       BL32_IMAGE_NAME,
		       BL32_BASE,
		       bl2_to_bl31_params->bl32_image_info,
		       bl2_to_bl31_params->bl32_ep_info);

	if (e == 0) {
		bl2_plat_set_bl32_ep_info(
			bl2_to_bl31_params->bl32_image_info,
			bl2_to_bl31_params->bl32_ep_info);
	}
#endif /* BL32_BASE */

	return e;
}

/*******************************************************************************
 * Load the BL3-3 image.
 * The bl2_to_bl31_params param will be updated with the relevant BL3-3
 * information.
 * Return 0 on success, a negative error code otherwise.
 ******************************************************************************/
static int load_bl33(bl31_params_t *bl2_to_bl31_params)
{
	meminfo_t bl33_mem_info;
	int e;

	INFO("BL2: Loading BL3-3\n");
	assert(bl2_to_bl31_params != NULL);

	bl2_plat_get_bl33_meminfo(&bl33_mem_info);

	/* Load the BL3-3 image in non-secure memory provided by the platform */
	e = load_image(&bl33_mem_info,
		       BL33_IMAGE_NAME,
		       plat_get_ns_image_entrypoint(),
		       bl2_to_bl31_params->bl33_image_info,
		       bl2_to_bl31_params->bl33_ep_info);

	if (e == 0)
		bl2_plat_set_bl33_ep_info(bl2_to_bl31_params->bl33_image_info,
					  bl2_to_bl31_params->bl33_ep_info);

	return e;
}

/*******************************************************************************
 * The only thing to do in BL2 is to load further images and pass control to
 * BL3-1. The memory occupied by BL2 will be reclaimed by BL3-x stages. BL2 runs
 * entirely in S-EL1.
 ******************************************************************************/
void bl2_main(void)
{
	bl31_params_t *bl2_to_bl31_params;
	entry_point_info_t *bl31_ep_info;
	int e;

	NOTICE("BL2: %s\n", version_string);
	NOTICE("BL2: %s\n", build_message);

	/* Perform remaining generic architectural setup in S-EL1 */
	bl2_arch_setup();

	/* Perform platform setup in BL2 */
	bl2_platform_setup();

	/*
	 * Load the subsequent bootloader images
	 */
	e = load_bl30();
	if (e) {
		ERROR("Failed to load BL3-0 (%i)\n", e);
		panic();
	}

	/*
	 * Get a pointer to the memory the platform has set aside to pass
	 * information to BL3-1.
	 */
	bl2_to_bl31_params = bl2_plat_get_bl31_params();
	bl31_ep_info = bl2_plat_get_bl31_ep_info();

	e = load_bl31(bl2_to_bl31_params, bl31_ep_info);
	if (e) {
		ERROR("Failed to load BL3-1 (%i)\n", e);
		panic();
	}

	e = load_bl32(bl2_to_bl31_params);
	if (e)
		WARN("Failed to load BL3-2 (%i)\n", e);

	e = load_bl33(bl2_to_bl31_params);
	if (e) {
		ERROR("Failed to load BL3-3 (%i)\n", e);
		panic();
	}

	/* Flush the params to be passed to memory */
	bl2_plat_flush_bl31_params();

	/*
	 * Run BL3-1 via an SMC to BL1. Information on how to pass control to
	 * the BL3-2 (if present) and BL3-3 software images will be passed to
	 * BL3-1 as an argument.
	 */
	smc(RUN_IMAGE, (unsigned long)bl31_ep_info, 0, 0, 0, 0, 0, 0);
}
