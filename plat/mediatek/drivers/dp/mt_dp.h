/*
 * Copyright (c) 2020-2022, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MT_DP_H
#define MT_DP_H

#define DP_TX_SECURE_REG11		(0x2c)

#define VIDEO_MUTE_SEL_SECURE_FLDMASK	(0x10)
#define VIDEO_MUTE_SW_SECURE_FLDMASK	(0x8)

enum DP_ATF_HW_TYPE {
	DP_ATF_TYPE_DP = 0,
	DP_ATF_TYPE_EDP = 1
};

enum DP_ATF_CMD {
	DP_ATF_DP_VIDEO_UNMUTE = 0x20,
	DP_ATF_EDP_VIDEO_UNMUTE,
	DP_ATF_CMD_COUNT
};

int32_t dp_secure_handler(uint64_t cmd, uint64_t para, uint32_t *val);

#endif
