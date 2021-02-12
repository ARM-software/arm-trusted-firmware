/*
 * Copyright (c) 2016-2017,2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <common/bl_common.h>
#include <common/desc_image_load.h>
#include <lib/fconf/fconf.h>
#include <lib/fconf/fconf_dyn_cfg_getter.h>

#include <plat/arm/common/plat_arm.h>

#if JUNO_AARCH32_EL3_RUNTIME
/*******************************************************************************
 * This function changes the spsr for BL32 image to bypass
 * the check in BL1 AArch64 exception handler. This is needed in the aarch32
 * boot flow as the core comes up in aarch64 and to enter the BL32 image a warm
 * reset in aarch32 state is required.
 ******************************************************************************/
int arm_bl2_plat_handle_post_image_load(unsigned int image_id)
{
	int err = arm_bl2_handle_post_image_load(image_id);

	if (!err && (image_id == BL32_IMAGE_ID)) {
		bl_mem_params_node_t *bl_mem_params = get_bl_mem_params_node(image_id);
		assert(bl_mem_params);
		bl_mem_params->ep_info.spsr = SPSR_64(MODE_EL3, MODE_SP_ELX,
			DISABLE_ALL_EXCEPTIONS);
	}

	return err;
}

#else

/*******************************************************************************
 * This function returns the list of executable images
 ******************************************************************************/
struct bl_params *plat_get_next_bl_params(void)
{
	struct bl_params *arm_bl_params = arm_get_next_bl_params();

#if __aarch64__
	const struct dyn_cfg_dtb_info_t *fw_config_info;
	bl_mem_params_node_t *param_node;
	uintptr_t fw_config_base = 0U;
	entry_point_info_t *ep_info;

	/* Get BL31 image node */
	param_node = get_bl_mem_params_node(BL31_IMAGE_ID);
	assert(param_node != NULL);

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
#endif /* __aarch64__ */

	return arm_bl_params;
}
#endif /* JUNO_AARCH32_EL3_RUNTIME */
