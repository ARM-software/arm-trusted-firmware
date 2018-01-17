/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * ZynqMP system level PM-API functions for pin control.
 */

#ifndef _PM_API_IOCTL_H_
#define _PM_API_IOCTL_H_

#include "pm_common.h"

enum pm_ioctl_id {
	IOCTL_GET_RPU_OPER_MODE,
	IOCTL_SET_RPU_OPER_MODE,
	IOCTL_RPU_BOOT_ADDR_CONFIG,
	IOCTL_TCM_COMB_CONFIG,
	IOCTL_SET_TAPDELAY_BYPASS,
	IOCTL_SET_SGMII_MODE,
	IOCTL_SD_DLL_RESET,
	IOCTL_SET_SD_TAPDELAY,
	 /* Ioctl for clock driver */
	IOCTL_SET_PLL_FRAC_MODE,
	IOCTL_GET_PLL_FRAC_MODE,
	IOCTL_SET_PLL_FRAC_DATA,
	IOCTL_GET_PLL_FRAC_DATA,
};

enum rpu_oper_mode {
	PM_RPU_MODE_LOCKSTEP,
	PM_RPU_MODE_SPLIT,
};

enum rpu_boot_mem {
	PM_RPU_BOOTMEM_LOVEC,
	PM_RPU_BOOTMEM_HIVEC,
};

enum rpu_tcm_comb {
	PM_RPU_TCM_SPLIT,
	PM_RPU_TCM_COMB,
};

enum tap_delay_signal_type {
	PM_TAPDELAY_NAND_DQS_IN,
	PM_TAPDELAY_NAND_DQS_OUT,
	PM_TAPDELAY_QSPI,
	PM_TAPDELAY_MAX,
};

enum tap_delay_bypass_ctrl {
	PM_TAPDELAY_BYPASS_DISABLE,
	PM_TAPDELAY_BYPASS_ENABLE,
};

enum sgmii_mode {
	PM_SGMII_DISABLE,
	PM_SGMII_ENABLE,
};

enum tap_delay_type {
	PM_TAPDELAY_INPUT,
	PM_TAPDELAY_OUTPUT,
};

enum dll_reset_type {
	PM_DLL_RESET_ASSERT,
	PM_DLL_RESET_RELEASE,
	PM_DLL_RESET_PULSE,
};

enum pm_ret_status pm_api_ioctl(enum pm_node_id nid,
				unsigned int ioctl_id,
				unsigned int arg1,
				unsigned int arg2,
				unsigned int *value);
#endif /* _PM_API_IOCTL_H_ */
