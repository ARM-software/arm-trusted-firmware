/*
 * Copyright (c) 2025, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MTCMOS_H
#define MTCMOS_H

#include <stdint.h>

enum mtcmos_state {
	STA_POWER_DOWN,
	STA_POWER_ON,
};

struct bus_protect {
	uint32_t en_addr;
	uint32_t rdy_addr;
	uint32_t mask;
};

int spm_mtcmos_ctrl_ufs0(enum mtcmos_state state);
int spm_mtcmos_ctrl_ufs0_phy(enum mtcmos_state state);

#endif /* MTCMOS_H */
