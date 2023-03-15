/*
 * Copyright (c) 2023, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* TF-A system header */
#include <common/debug.h>

/* Vendor header */
#include "apusys.h"
#include "apusys_power.h"
#include <lib/mtk_init/mtk_init.h>

int apusys_init(void)
{
	apusys_power_init();
	return 0;
}
MTK_PLAT_SETUP_1_INIT(apusys_init);
