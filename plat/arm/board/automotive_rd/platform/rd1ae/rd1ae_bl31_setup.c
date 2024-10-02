/*
 * Copyright (c) 2024, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <drivers/arm/css/css_mhu_doorbell.h>
#include <drivers/arm/css/scmi.h>

static scmi_channel_plat_info_t plat_rd_scmi_info[] = {
	{
		.scmi_mbx_mem = CSS_SCMI_PAYLOAD_BASE,
		.db_reg_addr = PLAT_CSS_MHU_BASE + MHU_V3_SENDER_REG_SET(0),
		.db_preserve_mask = 0xfffffffe,
		.db_modify_mask = 0x1,
		.ring_doorbell = &mhu_ring_doorbell,
	},
};

scmi_channel_plat_info_t *plat_css_get_scmi_info(unsigned int channel_id)
{
	return &plat_rd_scmi_info[channel_id];
}

const plat_psci_ops_t *plat_arm_psci_override_pm_ops(plat_psci_ops_t *ops)
{
	return css_scmi_override_pm_ops(ops);
}

#if defined(SPD_spmd) && (SPMC_AT_EL3 == 0)
/*
 * A dummy implementation of the platform handler for Group0 secure interrupt.
 */
int plat_spmd_handle_group0_interrupt(uint32_t intid)
{
	(void)intid;
	return -1;
}
#endif /* defined(SPD_spmd) && (SPMC_AT_EL3 == 0) */
