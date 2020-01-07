/*
 * Copyright 2020-2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef IMX_CSU_H
#define IMX_CSU_H

#include <lib/utils_def.h>

#include <platform_def.h>

#define CSU_SEC_LEVEL_0		0xff
#define CSU_SEC_LEVEL_1		0xbb
#define CSU_SEC_LEVEL_2		0x3f
#define CSU_SEC_LEVEL_3		0x3b
#define CSU_SEC_LEVEL_4		0x33
#define CSU_SEC_LEVEL_5		0x22
#define CSU_SEC_LEVEL_6		0x03
#define CSU_SEC_LEVEL_7		0x0

#define LOCKED			0x1
#define UNLOCKED		0x0

#define CSLx_REG(x)		(IMX_CSU_BASE + ((x) / 2) * 4)
#define CSLx_LOCK(x)		((0x1 << (((x) % 2) * 16 + 8)))
#define CSLx_CFG(x, n)		((x) << (((n) % 2) * 16))

#define CSU_HP_REG(x)		(IMX_CSU_BASE + ((x) / 16) * 4 + 0x200)
#define CSU_HP_LOCK(x)		((0x1 << (((x) % 16) * 2 + 1)))
#define CSU_HP_CFG(x, n)	((x) << (((n) % 16) * 2))

#define CSU_SA_REG(x)		(IMX_CSU_BASE + 0x218)
#define CSU_SA_LOCK(x)		((0x1 << (((x) % 16) * 2 + 1)))
#define CSU_SA_CFG(x, n)	((x) << (((n) % 16) * 2))

#define CSU_HPCONTROL_REG(x)		(IMX_CSU_BASE + (((x) / 16) * 4) + 0x358)
#define CSU_HPCONTROL_LOCK(x)		((0x1 << (((x) % 16) * 2 + 1)))
#define CSU_HPCONTROL_CFG(x, n)		((x) << (((n) % 16) * 2))

enum csu_cfg_type {
	CSU_INVALID,
	CSU_CSL,
	CSU_HP,
	CSU_SA,
	CSU_HPCONTROL,
};

struct imx_csu_cfg {
	enum csu_cfg_type type;
	uint16_t idx;
	uint16_t lock : 1;
	uint16_t csl_level : 8;
	uint16_t hp : 1;
	uint16_t sa : 1;
	uint16_t hpctrl : 1;
};

#define CSU_CSLx(i, level, lk)	\
	{CSU_CSL, .idx = (i), .csl_level = (level), .lock = (lk),}

#define CSU_HPx(i, val, lk)	\
	{CSU_HP, .idx = (i), .hp = (val), .lock = (lk), }

#define CSU_SA(i, val, lk)	\
	{CSU_SA, .idx = (i), .sa = (val), .lock = (lk), }

#define CSU_HPCTRL(i, val, lk)	\
	{CSU_HPCONTROL, .idx = (i), .hpctrl = (val), .lock = (lk), }

void imx_csu_init(const struct imx_csu_cfg *csu_cfg);

#endif /* IMX_CSU_H */
