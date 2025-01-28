/*
 * Copyright (c) 2025, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLAT_MEDIATEK_DRIVERS_MTCMOS_MT8196_MTCMOS_H_
#define PLAT_MEDIATEK_DRIVERS_MTCMOS_MT8196_MTCMOS_H_

enum mtcmos_state {
	STA_POWER_DOWN,
	STA_POWER_ON,
};

int spm_mtcmos_ctrl_ufs0(enum mtcmos_state state);
int spm_mtcmos_ctrl_ufs0_phy(enum mtcmos_state state);

#endif /* PLAT_MEDIATEK_DRIVERS_MTCMOS_MT8196_MTCMOS_H_ */
