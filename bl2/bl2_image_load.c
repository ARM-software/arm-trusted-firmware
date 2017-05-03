/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_helpers.h>
#include <assert.h>
#include <auth_mod.h>
#include <bl_common.h>
#include <debug.h>
#include <errno.h>
#include <platform.h>
#include <platform_def.h>
#include <stdint.h>

/*
 * Check for platforms that use obsolete image terminology
 */
#ifdef BL30_BASE
# error "BL30_BASE platform define no longer used - please use SCP_BL2_BASE"
#endif

/*******************************************************************************
 * Load the SCP_BL2 image if there's one.
 * If a platform does not want to attempt to load SCP_BL2 image it must leave
 * SCP_BL2_BASE undefined.
 * Return 0 on success or if there's no SCP_BL2 image to load, a negative error
 * code otherwise.
 ******************************************************************************/
static int load_scp_bl2(void)
{
	int e = 0;
#ifdef SCP_BL2_BASE
	meminfo_t scp_bl2_mem_info;
	image_info_t scp_bl2_image_info;

	/*
	 * It is up to the platform to specify where SCP_BL2 should be loaded if
	 * it exists. It could create space in the secure sram or point to a
	 * completely different memory.
	 *
	 * The entry point information is not relevant in this case as the AP
	 * won't execute the SCP_BL2 image.
	 */
	INFO("BL2: Loading SCP_BL2\n");
	bl2_plat_get_scp_bl2_meminfo(&scp_bl2_mem_info);
	scp_bl2_image_info.h.version = VERSION_1;
	e = load_auth_image(&scp_bl2_mem_info,
			    SCP_BL2_IMAGE_ID,
			    SCP_BL2_BASE,
			    &scp_bl2_image_info,
			    NULL);

	if (e == 0) {
		/* The subsequent handling of SCP_BL2 is platform specific */
		e = bl2_plat_handle_scp_bl2(&scp_bl2_image_info);
		if (e) {
			ERROR("Failure in platform-specific handling of SCP_BL2 image.\n");
		}
	}
#endif /* SCP_BL2_BASE */

	return e;
}

#ifndef EL3_PAYLOAD_BASE
/*******************************************************************************
 * Load the BL31 image.
 * The bl2_to_bl31_params and bl31_ep_info params will be updated with the
 * relevant BL31 information.
 * Return 0 on success, a negative error code otherwise.
 ******************************************************************************/
static int load_bl31(bl31_params_t *bl2_to_bl31_params,
		     entry_point_info_t *bl31_ep_info)
{
	meminfo_t *bl2_tzram_layout;
	int e;

	INFO("BL2: Loading BL31\n");
	assert(bl2_to_bl31_params != NULL);
	assert(bl31_ep_info != NULL);

	/* Find out how much free trusted ram remains after BL2 load */
	bl2_tzram_layout = bl2_plat_sec_mem_layout();

	/* Set the X0 parameter to BL31 */
	bl31_ep_info->args.arg0 = (unsigned long)bl2_to_bl31_params;

	/* Load the BL31 image */
	e = load_auth_image(bl2_tzram_layout,
			    BL31_IMAGE_ID,
			    BL31_BASE,
			    bl2_to_bl31_params->bl31_image_info,
			    bl31_ep_info);

	if (e == 0) {
		bl2_plat_set_bl31_ep_info(bl2_to_bl31_params->bl31_image_info,
					  bl31_ep_info);
	}

	return e;
}

/*******************************************************************************
 * Load the BL32 image if there's one.
 * The bl2_to_bl31_params param will be updated with the relevant BL32
 * information.
 * If a platform does not want to attempt to load BL32 image it must leave
 * BL32_BASE undefined.
 * Return 0 on success or if there's no BL32 image to load, a negative error
 * code otherwise.
 ******************************************************************************/
static int load_bl32(bl31_params_t *bl2_to_bl31_params)
{
	int e = 0;
#ifdef BL32_BASE
	meminfo_t bl32_mem_info;

	INFO("BL2: Loading BL32\n");
	assert(bl2_to_bl31_params != NULL);

	/*
	 * It is up to the platform to specify where BL32 should be loaded if
	 * it exists. It could create space in the secure sram or point to a
	 * completely different memory.
	 */
	bl2_plat_get_bl32_meminfo(&bl32_mem_info);
	e = load_auth_image(&bl32_mem_info,
			    BL32_IMAGE_ID,
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

#ifndef PRELOADED_BL33_BASE
/*******************************************************************************
 * Load the BL33 image.
 * The bl2_to_bl31_params param will be updated with the relevant BL33
 * information.
 * Return 0 on success, a negative error code otherwise.
 ******************************************************************************/
static int load_bl33(bl31_params_t *bl2_to_bl31_params)
{
	meminfo_t bl33_mem_info;
	int e;

	INFO("BL2: Loading BL33\n");
	assert(bl2_to_bl31_params != NULL);

	bl2_plat_get_bl33_meminfo(&bl33_mem_info);

	/* Load the BL33 image in non-secure memory provided by the platform */
	e = load_auth_image(&bl33_mem_info,
			    BL33_IMAGE_ID,
			    plat_get_ns_image_entrypoint(),
			    bl2_to_bl31_params->bl33_image_info,
			    bl2_to_bl31_params->bl33_ep_info);

	if (e == 0) {
		bl2_plat_set_bl33_ep_info(bl2_to_bl31_params->bl33_image_info,
					  bl2_to_bl31_params->bl33_ep_info);
	}

	return e;
}
#endif /* PRELOADED_BL33_BASE */

#endif /* EL3_PAYLOAD_BASE */

/*******************************************************************************
 * This function loads SCP_BL2/BL3x images and returns the ep_info for
 * the next executable image.
 ******************************************************************************/
entry_point_info_t *bl2_load_images(void)
{
	bl31_params_t *bl2_to_bl31_params;
	entry_point_info_t *bl31_ep_info;
	int e;

	e = load_scp_bl2();
	if (e) {
		ERROR("Failed to load SCP_BL2 (%i)\n", e);
		plat_error_handler(e);
	}

	/* Perform platform setup in BL2 after loading SCP_BL2 */
	bl2_platform_setup();

	/*
	 * Get a pointer to the memory the platform has set aside to pass
	 * information to BL31.
	 */
	bl2_to_bl31_params = bl2_plat_get_bl31_params();
	bl31_ep_info = bl2_plat_get_bl31_ep_info();

#ifdef EL3_PAYLOAD_BASE
	/*
	 * In the case of an EL3 payload, we don't need to load any further
	 * images. Just update the BL31 entrypoint info structure to make BL1
	 * jump to the EL3 payload.
	 * The pointer to the memory the platform has set aside to pass
	 * information to BL31 in the normal boot flow is reused here, even
	 * though only a fraction of the information contained in the
	 * bl31_params_t structure makes sense in the context of EL3 payloads.
	 * This will be refined in the future.
	 */
	INFO("BL2: Populating the entrypoint info for the EL3 payload\n");
	bl31_ep_info->pc = EL3_PAYLOAD_BASE;
	bl31_ep_info->args.arg0 = (unsigned long) bl2_to_bl31_params;
	bl2_plat_set_bl31_ep_info(NULL, bl31_ep_info);
#else
	e = load_bl31(bl2_to_bl31_params, bl31_ep_info);
	if (e) {
		ERROR("Failed to load BL31 (%i)\n", e);
		plat_error_handler(e);
	}

	e = load_bl32(bl2_to_bl31_params);
	if (e) {
		if (e == -EAUTH) {
			ERROR("Failed to authenticate BL32\n");
			plat_error_handler(e);
		} else {
			WARN("Failed to load BL32 (%i)\n", e);
		}
	}

#ifdef PRELOADED_BL33_BASE
	/*
	 * In this case, don't load the BL33 image as it's already loaded in
	 * memory. Update BL33 entrypoint information.
	 */
	INFO("BL2: Populating the entrypoint info for the preloaded BL33\n");
	bl2_to_bl31_params->bl33_ep_info->pc = PRELOADED_BL33_BASE;
	bl2_plat_set_bl33_ep_info(NULL, bl2_to_bl31_params->bl33_ep_info);
#else
	e = load_bl33(bl2_to_bl31_params);
	if (e) {
		ERROR("Failed to load BL33 (%i)\n", e);
		plat_error_handler(e);
	}
#endif /* PRELOADED_BL33_BASE */

#endif /* EL3_PAYLOAD_BASE */

	/* Flush the params to be passed to memory */
	bl2_plat_flush_bl31_params();

	return bl31_ep_info;
}
