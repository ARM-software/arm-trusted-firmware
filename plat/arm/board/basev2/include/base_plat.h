/*
 * Copyright (c) 2026, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BASE_PLAT_H
#define BASE_PLAT_H

#include <drivers/arm/mhu.h>
#include <drivers/arm/rse_comms.h>

#include <platform_def.h>

void base_bl31_common_platform_setup(void);

static inline int base_rse_comms_init(void)
{
	struct mhu_addr mhu_addresses = {
		.sender_base = PLAT_RSE_AP_SND_MHU_BASE,
		.receiver_base = PLAT_RSE_AP_RCV_MHU_BASE,
	};

	return rse_mbx_init(&mhu_addresses);
}

#endif /* BASE_PLAT_H */
