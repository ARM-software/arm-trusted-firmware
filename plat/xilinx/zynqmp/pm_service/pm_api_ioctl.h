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
	IOCTL_GET_RPU_OPER_MODE = 0,
	IOCTL_SET_RPU_OPER_MODE = 1,
	IOCTL_RPU_BOOT_ADDR_CONFIG = 2,
	IOCTL_TCM_COMB_CONFIG = 3,
	IOCTL_SET_TAPDELAY_BYPASS = 4,
	IOCTL_SET_SGMII_MODE = 5,
	IOCTL_SD_DLL_RESET = 6,
	IOCTL_SET_SD_TAPDELAY = 7,
	 /* Ioctl for clock driver */
	IOCTL_SET_PLL_FRAC_MODE = 8,
	IOCTL_GET_PLL_FRAC_MODE = 9,
	IOCTL_SET_PLL_FRAC_DATA = 10,
	IOCTL_GET_PLL_FRAC_DATA = 11,
	IOCTL_WRITE_GGS = 12,
	IOCTL_READ_GGS = 13,
	IOCTL_WRITE_PGGS = 14,
	IOCTL_READ_PGGS = 15,
	/* IOCTL for ULPI reset */
	IOCTL_ULPI_RESET = 16,
	/* Set healthy bit value */
	IOCTL_SET_BOOT_HEALTH_STATUS = 17,
	IOCTL_AFI = 18,
	/* Probe counter read/write */
	IOCTL_PROBE_COUNTER_READ = 19,
	IOCTL_PROBE_COUNTER_WRITE = 20,
	IOCTL_OSPI_MUX_SELECT = 21,
	/* IOCTL for USB power request */
	IOCTL_USB_SET_STATE = 22,
	/* IOCTL to get last reset reason */
	IOCTL_GET_LAST_RESET_REASON = 23,
	/* AI engine NPI ISR clear */
	IOCTL_AIE_ISR_CLEAR = 24,
	/* Register SGI to ATF */
	IOCTL_REGISTER_SGI = 25,
	/* Runtime feature configuration */
	IOCTL_SET_FEATURE_CONFIG = 26,
	IOCTL_GET_FEATURE_CONFIG = 27,
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
