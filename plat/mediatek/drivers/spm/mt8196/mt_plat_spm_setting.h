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
	CMD_1,
	CMD_2,
	CMD_3,
	CMD_4,
	CMD_5,
	CMD_6,
	CMD_7,
	CMD_8,
	CMD_9,
	CMD_10,
	CMD_11,
	CMD_12,
	CMD_13,
	CMD_14,
	CMD_15,
	CMD_16,
	CMD_17,
	CMD_18,
	CMD_19,
	CMD_20,
	CMD_21,
	CMD_22,
	CMD_23,
	CMD_24,
	CMD_25,
	CMD_26,
	CMD_27,
	CMD_28,
	CMD_29,
	CMD_30,
	CMD_31,
	NR_IDX_ALL,
};

/* APIs */
void plat_spm_pmic_wrap_init(void);

#endif /* MT_PLAT_SPM_SETTING_H */
