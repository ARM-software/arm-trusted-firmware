/*
 * Copyright (c) 2022-2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <drivers/arm/mhu.h>
#include <drivers/arm/rse_comms.h>
#include <platform_def.h>
#include <tc_rse_comms.h>

static const struct mhu_addr mhu_addresses = {
	PLAT_RSE_AP_SND_MHU_BASE,
	PLAT_RSE_AP_RCV_MHU_BASE
};

int plat_rse_comms_init(void)
{
	VERBOSE("Initializing the rse_comms now\n");
	/* Initialize the communication channel between AP and RSE */
	return rse_mbx_init(&mhu_addresses);
}
