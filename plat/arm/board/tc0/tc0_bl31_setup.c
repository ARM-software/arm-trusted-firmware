/*
 * Copyright (c) 2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <libfdt.h>
#include <tc0_plat.h>

#include <common/bl_common.h>
#include <common/debug.h>
#include <drivers/arm/css/css_mhu_doorbell.h>
#include <drivers/arm/css/scmi.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>

static scmi_channel_plat_info_t tc0_scmi_plat_info[] = {
	{
		.scmi_mbx_mem = CSS_SCMI_PAYLOAD_BASE,
		.db_reg_addr = PLAT_CSS_MHU_BASE + SENDER_REG_SET(0),
		.db_preserve_mask = 0xfffffffe,
		.db_modify_mask = 0x1,
		.ring_doorbell = &mhuv2_ring_doorbell,
	}
};

void bl31_platform_setup(void)
{
	tc0_bl31_common_platform_setup();
}

scmi_channel_plat_info_t *plat_css_get_scmi_info(int channel_id)
{

	return &tc0_scmi_plat_info[channel_id];

}

void bl31_early_platform_setup2(u_register_t arg0, u_register_t arg1,
				u_register_t arg2, u_register_t arg3)
{
	arm_bl31_early_platform_setup((void *)arg0, arg1, arg2, (void *)arg3);
}

void tc0_bl31_common_platform_setup(void)
{
	arm_bl31_platform_setup();
}

const plat_psci_ops_t *plat_arm_psci_override_pm_ops(plat_psci_ops_t *ops)
{
	return css_scmi_override_pm_ops(ops);
}
