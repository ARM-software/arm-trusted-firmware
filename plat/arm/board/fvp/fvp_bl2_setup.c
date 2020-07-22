/*
 * Copyright (c) 2013-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <common/debug.h>
#include <common/desc_image_load.h>
#include <drivers/arm/sp804_delay_timer.h>
#if MEASURED_BOOT
#include <drivers/measured_boot/measured_boot.h>
#endif
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

	arm_bl_params = arm_get_next_bl_params();

#if __aarch64__ && !BL2_AT_EL3
	const struct dyn_cfg_dtb_info_t *fw_config_info;
	bl_mem_params_node_t *param_node;
	uintptr_t fw_config_base = 0U;
	entry_point_info_t *ep_info;

	/* Get BL31 image node */
	param_node = get_bl_mem_params_node(BL31_IMAGE_ID);
	assert(param_node != NULL);

	/* get fw_config load address */
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
#endif /* __aarch64__ && !BL2_AT_EL3 */

	return arm_bl_params;
}
#if MEASURED_BOOT
static int fvp_bl2_plat_handle_post_image_load(unsigned int image_id)
{
	const bl_mem_params_node_t *bl_mem_params =
				get_bl_mem_params_node(image_id);

	assert(bl_mem_params != NULL);

	image_info_t info = bl_mem_params->image_info;
	int err;

	if ((info.h.attr & IMAGE_ATTRIB_SKIP_LOADING) == 0U) {
		/* Calculate image hash and record data in Event Log */
		err = tpm_record_measurement(info.image_base,
					     info.image_size, image_id);
		if (err != 0) {
			ERROR("%s%s image id %u (%i)\n",
				"BL2: Failed to ", "record", image_id, err);
			return err;
		}
	}

	err = arm_bl2_handle_post_image_load(image_id);
	if (err != 0) {
		ERROR("%s%s image id %u (%i)\n",
			"BL2: Failed to ", "handle", image_id, err);
	}

	return err;
}

int arm_bl2_plat_handle_post_image_load(unsigned int image_id)
{
	int err = fvp_bl2_plat_handle_post_image_load(image_id);

	if (err != 0) {
		ERROR("%s() returns %i\n", __func__, err);
	}

	return err;
}
#endif	/* MEASURED_BOOT */
