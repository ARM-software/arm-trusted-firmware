/*
 * Copyright (c) 2018-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <libfdt.h>

#include <common/bl_common.h>
#include <common/debug.h>
#include <drivers/arm/css/css_mhu_doorbell.h>
#include <drivers/arm/css/scmi.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>

#include <sgi_ras.h>
#include <sgi_variant.h>

sgi_platform_info_t sgi_plat_info;

static scmi_channel_plat_info_t sgi575_scmi_plat_info = {
		.scmi_mbx_mem = CSS_SCMI_PAYLOAD_BASE,
		.db_reg_addr = PLAT_CSS_MHU_BASE + CSS_SCMI_MHU_DB_REG_OFF,
		.db_preserve_mask = 0xfffffffe,
		.db_modify_mask = 0x1,
		.ring_doorbell = &mhu_ring_doorbell,
};

static scmi_channel_plat_info_t rd_n1e1_edge_scmi_plat_info = {
		.scmi_mbx_mem = CSS_SCMI_PAYLOAD_BASE,
		.db_reg_addr = PLAT_CSS_MHU_BASE + SENDER_REG_SET(0),
		.db_preserve_mask = 0xfffffffe,
		.db_modify_mask = 0x1,
		.ring_doorbell = &mhuv2_ring_doorbell,
};

scmi_channel_plat_info_t *plat_css_get_scmi_info(void)
{
	if (sgi_plat_info.platform_id == RD_N1E1_EDGE_SID_VER_PART_NUM)
		return &rd_n1e1_edge_scmi_plat_info;
	else if (sgi_plat_info.platform_id == SGI575_SSC_VER_PART_NUM)
		return &sgi575_scmi_plat_info;
	else
		panic();
}

void bl31_early_platform_setup2(u_register_t arg0, u_register_t arg1,
				u_register_t arg2, u_register_t arg3)
{
	sgi_plat_info.platform_id = plat_arm_sgi_get_platform_id();
	sgi_plat_info.config_id = plat_arm_sgi_get_config_id();

	arm_bl31_early_platform_setup((void *)arg0, arg1, arg2, (void *)arg3);
}

void bl31_platform_setup(void)
{
	arm_bl31_platform_setup();

#if RAS_EXTENSION
	sgi_ras_intr_handler_setup();
#endif
}

const plat_psci_ops_t *plat_arm_psci_override_pm_ops(plat_psci_ops_t *ops)
{
	/* For RD-E1-Edge platform only CPU ON/OFF is supported */
	if ((sgi_plat_info.platform_id == RD_N1E1_EDGE_SID_VER_PART_NUM) &&
	    (sgi_plat_info.config_id == RD_E1_EDGE_CONFIG_ID)) {
		ops->cpu_standby = NULL;
		ops->system_off = NULL;
		ops->system_reset = NULL;
		ops->get_sys_suspend_power_state = NULL;
		ops->pwr_domain_suspend = NULL;
		ops->pwr_domain_suspend_finish = NULL;
	}

	return css_scmi_override_pm_ops(ops);
}
