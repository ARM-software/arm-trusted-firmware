/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <bl_common.h>
#include <debug.h>
#include <plat_arm.h>
#include <sgi_plat_config.h>
#include <sgi_ras.h>
#include "../../css/drivers/scmi/scmi.h"
#include "../../css/drivers/mhu/css_mhu_doorbell.h"

static scmi_channel_plat_info_t sgi575_scmi_plat_info = {
		.scmi_mbx_mem = CSS_SCMI_PAYLOAD_BASE,
		.db_reg_addr = PLAT_CSS_MHU_BASE + CSS_SCMI_MHU_DB_REG_OFF,
		.db_preserve_mask = 0xfffffffe,
		.db_modify_mask = 0x1,
		.ring_doorbell = &mhu_ring_doorbell,
};

scmi_channel_plat_info_t *plat_css_get_scmi_info()
{
	return &sgi575_scmi_plat_info;
}

void bl31_early_platform_setup2(u_register_t arg0, u_register_t arg1,
				u_register_t arg2, u_register_t arg3)
{
	/* Initialize the platform configuration structure */
	plat_config_init();

	arm_bl31_early_platform_setup((void *)arg0, arg1, arg2, (void *)arg3);
}

void bl31_platform_setup(void)
{
	arm_bl31_platform_setup();

#if RAS_EXTENSION
	sgi_ras_intr_handler_setup();
#endif
}
