/*
 * Copyright (c) 2025, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DISP_PRIV_H
#define DISP_PRIV_H

#include <common/debug.h>
#include <lib/mmio.h>

#define DISP_CFG_ENTRY(base_reg, mask)			\
	{ .base = (base_reg), .ns_mask = (mask)}

struct mtk_disp_config {
	uint32_t base;
	uint32_t ns_mask;
};

enum DISP_ATF_CMD {
	DISP_ATF_CMD_CONFIG_DISP_CONFIG,
	DISP_ATF_CMD_COUNT,
};

/* disable secure mode for disp */
extern const struct mtk_disp_config *disp_cfg;
extern const size_t disp_cfg_count;

#endif	/* DISP_PRIV_H */
