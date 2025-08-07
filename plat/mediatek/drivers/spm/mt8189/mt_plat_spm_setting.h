/*
 * Copyright (c) 2025, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MT_PLAT_SPM_SETTING_H
#define MT_PLAT_SPM_SETTING_H

#include <sleep_def.h>

enum plat_spm_cond {
	PLAT_SPM_COND_MAX = 0,
};

enum pmic_wrap_phase_id {
	PMIC_WRAP_PHASE_ALLINONE,
	NR_PMIC_WRAP_PHASE,
};

/* IDX mapping */
enum {
	CMD_0,
	/* 0x0 */ /* PMIC_WRAP_PHASE_ALLINONE */
	CMD_1, /* 0x1 */
	CMD_2, /* 0x2 */
	CMD_3, /* 0x3 */
	CMD_4, /* 0x4 */
	CMD_5, /* 0x5 */
	CMD_6, /* 0x6 */
	CMD_7, /* 0x7 */
	CMD_8, /* 0x8 */
	CMD_9, /* 0x9 */
	CMD_10, /* 0xA */
	CMD_11, /* 0xB */
	CMD_12, /* 0xC */
	CMD_13, /* 0xD */
	CMD_14, /* 0xE */
	CMD_15, /* 0xF */
	CMD_16, /* 0x10 */
	CMD_17, /* 0x11 */
	CMD_18, /* 0x12 */
	CMD_19, /* 0x13 */
	CMD_20, /* 0x14 */
	CMD_21, /* 0x15 */
	CMD_22, /* 0x16 */
	CMD_23, /* 0x17 */
	CMD_24, /* 0x18 */
	NR_IDX_ALL,
};

/* APIs */
void plat_spm_pmic_wrap_init(void);
void plat_spm_cond_init(void);

#endif
