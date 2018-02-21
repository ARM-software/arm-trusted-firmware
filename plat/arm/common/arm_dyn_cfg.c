/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arm_dyn_cfg_helpers.h>
#include <assert.h>
#include <debug.h>
#include <desc_image_load.h>
#include <plat_arm.h>
#include <platform.h>
#include <platform_def.h>
#include <string.h>
#include <tbbr_img_def.h>

#if LOAD_IMAGE_V2

/* Variable to store the address to TB_FW_CONFIG passed from BL1 */
static void *tb_fw_cfg_dtb;

/*
 * Helper function to load TB_FW_CONFIG and populate the load information to
 * arg0 of BL2 entrypoint info.
 */
void arm_load_tb_fw_config(void)
{
	int err;
	uintptr_t config_base = 0;
	image_desc_t *image_desc;

	image_desc_t arm_tb_fw_info = {
		.image_id = TB_FW_CONFIG_ID,
		SET_STATIC_PARAM_HEAD(image_info, PARAM_IMAGE_BINARY,
				VERSION_2, image_info_t, 0),
		.image_info.image_base = ARM_TB_FW_CONFIG_BASE,
		.image_info.image_max_size = ARM_TB_FW_CONFIG_LIMIT - ARM_TB_FW_CONFIG_BASE,
	};

	VERBOSE("BL1: Loading TB_FW_CONFIG\n");
	err = load_auth_image(TB_FW_CONFIG_ID, &arm_tb_fw_info.image_info);
	if (err != 0) {
		/* Return if TB_FW_CONFIG is not loaded */
		VERBOSE("Failed to load TB_FW_CONFIG\n");
		return;
	}

	config_base = arm_tb_fw_info.image_info.image_base;

	/* The BL2 ep_info arg0 is modified to point to TB_FW_CONFIG */
	image_desc = bl1_plat_get_image_desc(BL2_IMAGE_ID);
	assert(image_desc != NULL);
	image_desc->ep_info.args.arg0 = config_base;

	INFO("BL1: TB_FW_CONFIG loaded at address = %p\n",
			(void *) config_base);
}

/*
 * BL2 utility function to set the address of TB_FW_CONFIG passed from BL1.
 */
void arm_bl2_set_tb_cfg_addr(void *dtb)
{
	assert(dtb != NULL);
	tb_fw_cfg_dtb = dtb;
}

/*
 * BL2 utility function to initialize dynamic configuration specified by
 * TB_FW_CONFIG. Return early if TB_FW_CONFIG is not found or HW_CONFIG is
 * not specified in TB_FW_CONFIG.
 */
void arm_bl2_dyn_cfg_init(void)
{
	int err = 0;
	int tb_fw_node;
	bl_mem_params_node_t *hw_cfg_mem_params = NULL;

	if (tb_fw_cfg_dtb == NULL) {
		VERBOSE("No TB_FW_CONFIG specified\n");
		return;
	}

	err = arm_dyn_tb_fw_cfg_init((void *)tb_fw_cfg_dtb, &tb_fw_node);
	if (err < 0) {
		ERROR("Invalid TB_FW_CONFIG passed from BL1\n");
		panic();
	}

	/* Get the hw_config load address and size from TB_FW_CONFIG */
	hw_cfg_mem_params = get_bl_mem_params_node(HW_CONFIG_ID);
	if (hw_cfg_mem_params == NULL) {
		VERBOSE("Couldn't find HW_CONFIG in bl_mem_params_node\n");
		return;
	}

	err = arm_dyn_get_hwconfig_info((void *)tb_fw_cfg_dtb, tb_fw_node,
		(uint64_t *) &hw_cfg_mem_params->image_info.image_base,
		&hw_cfg_mem_params->image_info.image_max_size);
	if (err < 0) {
		VERBOSE("Couldn't find HW_CONFIG load info in TB_FW_CONFIG\n");
		return;
	}

	/* Remove the IMAGE_ATTRIB_SKIP_LOADING attribute from HW_CONFIG node */
	hw_cfg_mem_params->image_info.h.attr &= ~IMAGE_ATTRIB_SKIP_LOADING;
}

#endif /* LOAD_IMAGE_V2 */
