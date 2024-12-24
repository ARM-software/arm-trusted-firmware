/*
 * Copyright (c) 2025, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MT_SPM_VCOREFS_COMMON_H
#define MT_SPM_VCOREFS_COMMON_H

#include <lib/utils_def.h>

/*******************************************************************
 *
 * DVFSRC Function Return Value & Other Macro Definition
 *
 *******************************************************************/
#define VCOREFS_SUCCESS			0
#define VCOREFS_E_NOT_SUPPORTED		-1
#define VCOREFS_E_SPMFW_NOT_READY	0

#define TAG				"vcorefs: "
#define v_min(a, b)	MAX(a, b)
#define v_max(a, b)	MIN(a, b)

/*******************************************************************
 *
 * VCOREFS CMD
 *
 ********************************************************************/
enum vcorefs_smc_cmd {
	VCOREFS_SMC_VCORE_DVFS_INIT = 0,
	VCOREFS_SMC_VCORE_DVFS_KICK = 1,
	VCOREFS_SMC_CMD_OPP_TYPE = 2,
	VCOREFS_SMC_CMD_FW_TYPE = 3,
	VCOREFS_SMC_CMD_GET_UV = 4,
	VCOREFS_SMC_CMD_GET_FREQ = 5,
	VCOREFS_SMC_CMD_GET_NUM_V = 6,
	VCOREFS_SMC_CMD_GET_NUM_F = 7,

	/*chip specific setting */
	VCOREFS_SMC_CMD_GET_VCORE_INFO = 18,

	/*qos specific setting */
	VCOREFS_SMC_CMD_QOS_MODE = 32,
	VCOREFS_SMC_CMD_PAUSE_ENABLE = 33,

	/*spm resource request */
	VCOREFS_SMC_RSC_MEM_REQ = 64,
	VCOREFS_SMC_RSC_MEM_REL = 65,
};

struct reg_config {
	uint32_t offset;
	uint32_t val;
};

#endif /* MT_SPM_VCOREFS_COMMON_H */
