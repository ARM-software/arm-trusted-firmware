/*
 * Copyright (c) 2025, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/mtk_init/mtk_init.h>
#include <mtk_bl31_interface.h>

static int mtk_smmu_sid_init(void)
{
	return smmu_sid_init();
}

MTK_PLAT_SETUP_0_INIT(mtk_smmu_sid_init);
