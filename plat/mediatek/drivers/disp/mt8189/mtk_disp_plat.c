/*
 * Copyright (c) 2025, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mtk_disp_priv.h>
#include <mtk_mmap_pool.h>
#include <platform_def.h>

/* disp config */
#define MMSYS_SEC_DIS_0	(0xA00)
#define MMSYS_SEC_DIS_1	(0xA04)
#define MMSYS_SEC_DIS_2	(0xA08)
#define MMSYS_SHADOW	(0xC00)
#define MMSYS_CB_CON	(0xC0C)
#define MMSYS_CG_CON	(0x110)

const struct mtk_disp_config mt8189_disp_cfg[] = {
	/*SECURITY0*/
	DISP_CFG_ENTRY(MMSYS_CONFIG_BASE + MMSYS_SEC_DIS_0, 0xFFFFFFFF),
	/*SECURITY1*/
	DISP_CFG_ENTRY(MMSYS_CONFIG_BASE + MMSYS_SEC_DIS_1, 0xFFFFFFFF),
	/*SECURITY2*/
	DISP_CFG_ENTRY(MMSYS_CONFIG_BASE + MMSYS_SEC_DIS_2, 0xFFFFFFFF),
	/*SHADOW*/
	DISP_CFG_ENTRY(MMSYS_CONFIG_BASE + MMSYS_SHADOW, 0x1),
	/*CROSSBAR*/
	DISP_CFG_ENTRY(MMSYS_CONFIG_BASE + MMSYS_CB_CON, 0x00FF0000),
	/*CG_BIT*/
	DISP_CFG_ENTRY(MMSYS_CONFIG_BASE + MMSYS_CG_CON, 0x10000),
};

const struct mtk_disp_config *disp_cfg = &mt8189_disp_cfg[0];
const size_t disp_cfg_count = ARRAY_SIZE(mt8189_disp_cfg);
