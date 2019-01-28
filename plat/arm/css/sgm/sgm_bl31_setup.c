/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/bl_common.h>
#include <common/debug.h>
#include <drivers/arm/css/css_mhu_doorbell.h>
#include <drivers/arm/css/scmi.h>
#include <plat/arm/common/plat_arm.h>

#include <sgm_plat_config.h>

static scmi_channel_plat_info_t sgm775_scmi_plat_info = {
		.scmi_mbx_mem = CSS_SCMI_PAYLOAD_BASE,
		.db_reg_addr = PLAT_CSS_MHU_BASE + CSS_SCMI_MHU_DB_REG_OFF,
		.db_preserve_mask = 0xfffffffe,
		.db_modify_mask = 0x1,
		.ring_doorbell = &mhu_ring_doorbell,
};

scmi_channel_plat_info_t *plat_css_get_scmi_info()
{
	return &sgm775_scmi_plat_info;
}

void bl31_early_platform_setup2(u_register_t arg0, u_register_t arg1,
				u_register_t arg2, u_register_t arg3)
{
	uint32_t plat_version;
	bl_params_node_t *bl_params;

	bl_params = ((bl_params_t *)arg0)->head;

	/* Initialize the platform configuration structure */
	plat_config_init();

	while (bl_params) {
		if (bl_params->image_id == BL33_IMAGE_ID) {
			plat_version = mmio_read_32(SSC_VERSION);
			bl_params->ep_info->args.arg2 = plat_version;
			break;
		}

		bl_params = bl_params->next_params_info;
	}

	arm_bl31_early_platform_setup((void *)arg0, arg1, arg2, (void *)arg3);
}

const plat_psci_ops_t *plat_arm_psci_override_pm_ops(plat_psci_ops_t *ops)
{
	return css_scmi_override_pm_ops(ops);
}
