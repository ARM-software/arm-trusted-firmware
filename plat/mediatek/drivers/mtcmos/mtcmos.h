/*
 * Copyright (c) 2025, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MTCMOS_H
#define MTCMOS_H

enum mtcmos_state {
	STA_POWER_DOWN,
	STA_POWER_ON,
};

int spm_mtcmos_ctrl_ufs0(enum mtcmos_state state);
int spm_mtcmos_ctrl_ufs0_phy(enum mtcmos_state state);

#endif /* MTCMOS_H */
