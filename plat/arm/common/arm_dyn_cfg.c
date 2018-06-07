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

#if TRUSTED_BOARD_BOOT && defined(DYN_DISABLE_AUTH)
	int tb_fw_node;
	uint32_t disable_auth = 0;

	err = arm_dyn_tb_fw_cfg_init((void *)config_base, &tb_fw_node);
	if (err < 0) {
		WARN("Invalid TB_FW_CONFIG loaded\n");
		return;
	}

	err = arm_dyn_get_disable_auth((void *)config_base, tb_fw_node, &disable_auth);
	if (err < 0)
		return;

	if (disable_auth == 1)
		dyn_disable_auth();
#endif
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
 * TB_FW_CONFIG. Populate the bl_mem_params_node_t of other FW_CONFIGs if
 * specified in TB_FW_CONFIG.
 */
void arm_bl2_dyn_cfg_init(void)
{
	int err = 0, tb_fw_node;
	unsigned int i;
	bl_mem_params_node_t *cfg_mem_params = NULL;
	uint64_t image_base;
	uint32_t image_size;
	const unsigned int config_ids[] = {
			HW_CONFIG_ID,
			SOC_FW_CONFIG_ID,
			NT_FW_CONFIG_ID,
#ifdef SPD_tspd
			/* Currently tos_fw_config is only present for TSP */
			TOS_FW_CONFIG_ID
#endif
	};

	if (tb_fw_cfg_dtb == NULL) {
		VERBOSE("No TB_FW_CONFIG specified\n");
		return;
	}

	err = arm_dyn_tb_fw_cfg_init((void *)tb_fw_cfg_dtb, &tb_fw_node);
	if (err < 0) {
		ERROR("Invalid TB_FW_CONFIG passed from BL1\n");
		panic();
	}

	/* Iterate through all the fw config IDs */
	for (i = 0; i < ARRAY_SIZE(config_ids); i++) {
		/* Get the config load address and size from TB_FW_CONFIG */
		cfg_mem_params = get_bl_mem_params_node(config_ids[i]);
		if (cfg_mem_params == NULL) {
			VERBOSE("Couldn't find HW_CONFIG in bl_mem_params_node\n");
			continue;
		}

		err = arm_dyn_get_config_load_info((void *)tb_fw_cfg_dtb, tb_fw_node,
				config_ids[i], &image_base, &image_size);
		if (err < 0) {
			VERBOSE("Couldn't find config_id %d load info in TB_FW_CONFIG\n",
					config_ids[i]);
			continue;
		}

		/*
		 * Do some runtime checks on the load addresses of soc_fw_config,
		 * tos_fw_config, nt_fw_config. This is not a comprehensive check
		 * of all invalid addresses but to prevent trivial porting errors.
		 */
		if (config_ids[i] != HW_CONFIG_ID) {

			if (check_uptr_overflow(image_base, image_size) != 0)
				continue;

			/* Ensure the configs don't overlap with BL31 */
			if ((image_base > BL31_BASE) || ((image_base + image_size) > BL31_BASE))
				continue;

			/* Ensure the configs are loaded in a valid address */
			if (image_base < ARM_BL_RAM_BASE)
				continue;
#ifdef BL32_BASE
			/*
			 * If BL32 is present, ensure that the configs don't
			 * overlap with it.
			 */
			if (image_base >= BL32_BASE && image_base <= BL32_LIMIT)
				continue;
#endif
		}


		cfg_mem_params->image_info.image_base = (uintptr_t)image_base;
		cfg_mem_params->image_info.image_max_size = image_size;

		/* Remove the IMAGE_ATTRIB_SKIP_LOADING attribute from HW_CONFIG node */
		cfg_mem_params->image_info.h.attr &= ~IMAGE_ATTRIB_SKIP_LOADING;
	}

#if TRUSTED_BOARD_BOOT && defined(DYN_DISABLE_AUTH)
	uint32_t disable_auth = 0;

	err = arm_dyn_get_disable_auth((void *)tb_fw_cfg_dtb, tb_fw_node,
					&disable_auth);
	if (err < 0)
		return;

	if (disable_auth == 1)
		dyn_disable_auth();
#endif
}

#endif /* LOAD_IMAGE_V2 */
