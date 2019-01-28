/*
 * Copyright (c) 2014-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>

#include <arch_helpers.h>
#include <drivers/arm/css/css_mhu_doorbell.h>
#include <drivers/arm/css/scmi.h>

void mhu_ring_doorbell(struct scmi_channel_plat_info *plat_info)
{
	MHU_RING_DOORBELL(plat_info->db_reg_addr,
			plat_info->db_modify_mask,
			plat_info->db_preserve_mask);
	return;
}

void mhuv2_ring_doorbell(struct scmi_channel_plat_info *plat_info)
{
	/* wake receiver */
	MHU_V2_ACCESS_REQUEST(MHUV2_BASE_ADDR);

	/* wait for receiver to acknowledge its ready */
	while (MHU_V2_IS_ACCESS_READY(MHUV2_BASE_ADDR) == 0)
		;

	MHU_RING_DOORBELL(plat_info->db_reg_addr,
			plat_info->db_modify_mask,
			plat_info->db_preserve_mask);

	/* clear the access request for the receiver */
	MHU_V2_CLEAR_REQUEST(MHUV2_BASE_ADDR);

	return;
}
