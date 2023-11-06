/*
 * Copyright (c) 2018-2022, Arm Limited and Contributors. All rights reserved.
 * Copyright (c) 2022-2023, Advanced Micro Devices, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * ZynqMP system level PM-API functions for pin control.
 */

#ifndef PM_API_IOCTL_H
#define PM_API_IOCTL_H

#include "pm_common.h"

//RPU operation mode
#define	PM_RPU_MODE_LOCKSTEP 0U
#define	PM_RPU_MODE_SPLIT 1U

//RPU boot mem
#define	PM_RPU_BOOTMEM_LOVEC 0U
#define	PM_RPU_BOOTMEM_HIVEC 1U

//RPU tcm mpde
#define	PM_RPU_TCM_SPLIT 0U
#define	PM_RPU_TCM_COMB 1U

//tap delay signal type
#define	PM_TAPDELAY_NAND_DQS_IN 0U
#define	PM_TAPDELAY_NAND_DQS_OUT 1U
#define	PM_TAPDELAY_QSPI 2U
#define	PM_TAPDELAY_MAX 3U

//tap delay bypass
#define	PM_TAPDELAY_BYPASS_DISABLE 0U
#define	PM_TAPDELAY_BYPASS_ENABLE 1U

enum tap_delay_type {
	PM_TAPDELAY_INPUT,
	PM_TAPDELAY_OUTPUT,
};

//dll reset type
#define	PM_DLL_RESET_ASSERT 0U
#define	PM_DLL_RESET_RELEASE 1U
#define	PM_DLL_RESET_PULSE 2U

enum pm_ret_status pm_api_ioctl(enum pm_node_id nid,
				uint32_t ioctl_id,
				uint32_t arg1,
				uint32_t arg2,
				uint32_t *value);
enum pm_ret_status tfa_ioctl_bitmask(uint32_t *bit_mask);
#endif /* PM_API_IOCTL_H */
