/*
 * Copyright (c) 2023, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <common/debug.h>
#include <lib/mtk_init/mtk_init.h>
#include <lpm/mt_lp_api.h>
#include <platform_def.h>

int mt_usb_init(void)
{
	INFO("[%s] mt_usb initialization\n", __func__);

	/* Keep infra and peri on to support wake-up from USB */
	mtk_usb_update(LPM_USB_ENTER);

	return 0;
}
MTK_PLAT_SETUP_0_INIT(mt_usb_init);
