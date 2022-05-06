/*
 * Copyright (c) 2013-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <common/debug.h>
#include <common/desc_image_load.h>
#include <drivers/arm/sp804_delay_timer.h>
#include <lib/fconf/fconf.h>
#include <lib/fconf/fconf_dyn_cfg_getter.h>

#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>
#include <platform_def.h>

#include "fvp_private.h"

void bl2_early_platform_setup2(u_register_t arg0, u_register_t arg1, u_register_t arg2, u_register_t arg3)
{
	arm_bl2_early_platform_setup((uintptr_t)arg0, (meminfo_t *)arg1);

	/* Initialize the platform config for future decision making */
	fvp_config_setup();
}

void bl2_platform_setup(void)
{
	arm_bl2_platform_setup();

	/* Initialize System level generic or SP804 timer */
	fvp_timer_init();
}

/*******************************************************************************
 * This function returns the list of executable images
 ******************************************************************************/
struct bl_params *plat_get_next_bl_params(void)
{
	struct bl_params *arm_bl_params;
	const struct dyn_cfg_dtb_info_t *hw_config_info __unused;
	bl_mem_params_node_t *param_node __unused;

	arm_bl_params = arm_get_next_bl_params();

#if !BL2_AT_EL3 && !EL3_PAYLOAD_BASE
	const struct dyn_cfg_dtb_info_t *fw_config_info;
	uintptr_t fw_config_base = 0UL;
	entry_point_info_t *ep_info;

#if __aarch64__
	/* Get BL31 image node */
	param_node = get_bl_mem_params_node(BL31_IMAGE_ID);
#else /* aarch32 */
	/* Get SP_MIN image node */
	param_node = get_bl_mem_params_node(BL32_IMAGE_ID);
#endif /* __aarch64__ */
	assert(param_node != NULL);

	/* get fw_config load address */
	fw_config_info = FCONF_GET_PROPERTY(dyn_cfg, dtb, FW_CONFIG_ID);
	assert(fw_config_info != NULL);

	fw_config_base = fw_config_info->config_addr;
	assert(fw_config_base != 0UL);

	/*
	 * Get the entry point info of next executable image and override
	 * arg1 of entry point info with fw_config base address
	 */
	ep_info = &param_node->ep_info;
	ep_info->args.arg1 = (uint32_t)fw_config_base;

	/* grab NS HW config address */
	hw_config_info = FCONF_GET_PROPERTY(dyn_cfg, dtb, HW_CONFIG_ID);
	assert(hw_config_info != NULL);

	/* To retrieve actual size of the HW_CONFIG */
	param_node = get_bl_mem_params_node(HW_CONFIG_ID);
	assert(param_node != NULL);

	/* Copy HW config from Secure address to NS address */
	memcpy((void *)hw_config_info->ns_config_addr,
	       (void *)hw_config_info->config_addr,
	       (size_t)param_node->image_info.image_size);

	/*
	 * Ensure HW-config device tree committed to memory, as there is
	 * a possibility to use HW-config without cache and MMU enabled
	 * at BL33
	 */
	flush_dcache_range(hw_config_info->ns_config_addr,
			   param_node->image_info.image_size);

	param_node = get_bl_mem_params_node(BL33_IMAGE_ID);
	assert(param_node != NULL);

	/* Update BL33's ep info with NS HW config address  */
	param_node->ep_info.args.arg1 = hw_config_info->ns_config_addr;
#endif /* !BL2_AT_EL3 && !EL3_PAYLOAD_BASE */

	return arm_bl_params;
}
