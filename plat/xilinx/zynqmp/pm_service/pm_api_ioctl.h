/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * ZynqMP system level PM-API functions for pin control.
 */

#ifndef PM_API_IOCTL_H
#define PM_API_IOCTL_H

#include "pm_common.h"

//ioctl id
enum {
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
	IOCTL_WRITE_GGS,
	IOCTL_READ_GGS,
	IOCTL_WRITE_PGGS,
	IOCTL_READ_PGGS,
	/* IOCTL for ULPI reset */
	IOCTL_ULPI_RESET,
	/* Set healthy bit value */
	IOCTL_SET_BOOT_HEALTH_STATUS,
	IOCTL_AFI,
};

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

//sgmii mode
#define	PM_SGMII_DISABLE 0U
#define	PM_SGMII_ENABLE 1U

enum tap_delay_type {
	PM_TAPDELAY_INPUT,
	PM_TAPDELAY_OUTPUT,
};

//dll reset type
#define	PM_DLL_RESET_ASSERT 0U
#define	PM_DLL_RESET_RELEASE 1U
#define	PM_DLL_RESET_PULSE 2U

enum pm_ret_status pm_api_ioctl(enum pm_node_id nid,
				unsigned int ioctl_id,
				unsigned int arg1,
				unsigned int arg2,
				unsigned int *value);
#endif /* PM_API_IOCTL_H */
