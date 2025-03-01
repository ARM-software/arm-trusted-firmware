/*
 * Copyright (c) 2021-2025, ARM Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <common/bl_common.h>
#include <common/desc_image_load.h>
#include <lib/fconf/fconf.h>
#include <lib/fconf/fconf_dyn_cfg_getter.h>

#include <plat/arm/common/plat_arm.h>

/*******************************************************************************
 * This function returns the list of executable images
 ******************************************************************************/
struct bl_params *plat_get_next_bl_params(void)
{
	struct bl_params *arm_bl_params;
	bl_mem_params_node_t *param_node;
	const struct dyn_cfg_dtb_info_t *fw_config_info __maybe_unused;
	uintptr_t fw_config_base __maybe_unused;
	entry_point_info_t *ep_info __maybe_unused;

	arm_bl_params = arm_get_next_bl_params();

	/* Get BL31 image node */
	param_node = get_bl_mem_params_node(BL31_IMAGE_ID);
	assert(param_node != NULL);
#if TRANSFER_LIST
	assert(arm_bl_params != NULL);

	arm_bl_params->head = &param_node->params_node_mem;
	arm_bl_params->head->ep_info = &param_node->ep_info;
	arm_bl_params->head->image_id = param_node->image_id;

	arm_bl2_setup_next_ep_info(param_node);
#else
	/* Get fw_config load address */
	fw_config_info = FCONF_GET_PROPERTY(dyn_cfg, dtb, FW_CONFIG_ID);
	assert(fw_config_info != NULL);

	fw_config_base = fw_config_info->config_addr;
	assert(fw_config_base != 0U);

	/*
	 * Get the entry point info of BL31 image and override
	 * arg1 of entry point info with fw_config base address
	 */
	ep_info = &param_node->ep_info;
	ep_info->args.arg1 = (uint32_t)fw_config_base;
#endif /* TRANSFER_LIST */

	return arm_bl_params;
}
