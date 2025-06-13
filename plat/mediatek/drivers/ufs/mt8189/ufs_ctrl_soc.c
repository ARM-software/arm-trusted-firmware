/*
 * Copyright (c) 2025, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#if defined(CONFIG_MTK_MTCMOS)
#include <mtcmos.h>
#endif
#include <mtk_bl31_interface.h>

void ufs_device_pwr_ctrl_soc(bool vcc_on, uint64_t ufs_version) {}

int ufs_spm_mtcmos_power(bool on)
{
#if defined(CONFIG_MTK_MTCMOS)
	return spm_mtcmos_ctrl_ufs0(on ? STA_POWER_ON : STA_POWER_DOWN);
#else
	return 0;
#endif
}
