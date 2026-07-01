/*
 * Copyright (c) 2026, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <drivers/arm/mhu.h>
#include <drivers/arm/rse_comms.h>
#include <platform_def.h>

#include "rdaspen_rse_comms.h"

int plat_rse_comms_init(void)
{
	struct mhu_addr mhu_addresses;

	/* Get sender and receiver frames for AP-RSE communication */
	mhu_addresses.sender_base = AP_RSE_SECURE_MHU_V3_PBX;
	mhu_addresses.receiver_base = AP_RSE_SECURE_MHU_V3_MBX;

	/* Initialize the communication channel between AP and RSE */
	return rse_mbx_init(&mhu_addresses);
}
