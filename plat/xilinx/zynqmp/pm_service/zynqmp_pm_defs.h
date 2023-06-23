/*
 * Copyright (c) 2013-2022, Arm Limited and Contributors. All rights reserved.
 * Copyright (c) 2022-2023, Advanced Micro Devices, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* ZynqMP power management enums and defines */

#ifndef ZYNQMP_PM_DEFS_H
#define ZYNQMP_PM_DEFS_H

/*********************************************************************
 * Macro definitions
 ********************************************************************/

/*
 * Version number is a 32bit value, like:
 * (PM_VERSION_MAJOR << 16) | PM_VERSION_MINOR
 */
#define PM_VERSION_MAJOR	1U
#define PM_VERSION_MINOR	1U

#define PM_VERSION	((PM_VERSION_MAJOR << 16U) | PM_VERSION_MINOR)

/*
 * PM API versions
 */

/* Expected version of firmware APIs */
#define FW_API_BASE_VERSION		(1U)
/* Expected version of firmware API for feature check */
#define FW_API_VERSION_2		(2U)
/* Version of APIs implemented in TF-A */
#define TFA_API_BASE_VERSION		(1U)
/* Updating the QUERY_DATA API versioning as the bitmask functionality
 * support is added in the v2.*/
#define TFA_API_QUERY_DATA_VERSION	(2U)

/* Capabilities for RAM */
#define PM_CAP_ACCESS	0x1U
#define PM_CAP_CONTEXT	0x2U

/* APU processor states */
#define PM_PROC_STATE_FORCEDOFF		0U
#define PM_PROC_STATE_ACTIVE		1U
#define PM_PROC_STATE_SLEEP		2U
#define PM_PROC_STATE_SUSPENDING	3U

#define PM_SET_SUSPEND_MODE		0xa02

/*********************************************************************
 * Enum definitions
 ********************************************************************/

enum pm_node_id {
	NODE_UNKNOWN = 0,
	NODE_APU,
	NODE_APU_0,
	NODE_APU_1,
	NODE_APU_2,
	NODE_APU_3,
	NODE_RPU,
	NODE_RPU_0,
	NODE_RPU_1,
	NODE_PLD,
	NODE_FPD,
	NODE_OCM_BANK_0,
	NODE_OCM_BANK_1,
	NODE_OCM_BANK_2,
	NODE_OCM_BANK_3,
	NODE_TCM_0_A,
	NODE_TCM_0_B,
	NODE_TCM_1_A,
	NODE_TCM_1_B,
	NODE_L2,
	NODE_GPU_PP_0,
	NODE_GPU_PP_1,
	NODE_USB_0,
	NODE_USB_1,
	NODE_TTC_0,
	NODE_TTC_1,
	NODE_TTC_2,
	NODE_TTC_3,
	NODE_SATA,
	NODE_ETH_0,
	NODE_ETH_1,
	NODE_ETH_2,
	NODE_ETH_3,
	NODE_UART_0,
	NODE_UART_1,
	NODE_SPI_0,
	NODE_SPI_1,
	NODE_I2C_0,
	NODE_I2C_1,
	NODE_SD_0,
	NODE_SD_1,
	NODE_DP,
	NODE_GDMA,
	NODE_ADMA,
	NODE_NAND,
	NODE_QSPI,
	NODE_GPIO,
	NODE_CAN_0,
	NODE_CAN_1,
	NODE_EXTERN,
	NODE_APLL,
	NODE_VPLL,
	NODE_DPLL,
	NODE_RPLL,
	NODE_IOPLL,
	NODE_DDR,
	NODE_IPI_APU,
	NODE_IPI_RPU_0,
	NODE_GPU,
	NODE_PCIE,
	NODE_PCAP,
	NODE_RTC,
	NODE_LPD,
	NODE_VCU,
	NODE_IPI_RPU_1,
	NODE_IPI_PL_0,
	NODE_IPI_PL_1,
	NODE_IPI_PL_2,
	NODE_IPI_PL_3,
	NODE_PL,
	NODE_GEM_TSU,
	NODE_SWDT_0,
	NODE_SWDT_1,
	NODE_CSU,
	NODE_PJTAG,
	NODE_TRACE,
	NODE_TESTSCAN,
	NODE_PMU,
	NODE_MAX,
};

enum pm_request_ack {
	REQ_ACK_NO = 1,
	REQ_ACK_BLOCKING,
	REQ_ACK_NON_BLOCKING,
};

enum pm_suspend_reason {
	SUSPEND_REASON_PU_REQ = 201,
	SUSPEND_REASON_ALERT,
	SUSPEND_REASON_SYS_SHUTDOWN,
};

enum pm_ram_state {
	PM_RAM_STATE_OFF = 1,
	PM_RAM_STATE_RETENTION,
	PM_RAM_STATE_ON,
};

/**
 * enum pm_boot_status - enum represents the boot status of the PM.
 * @PM_INITIAL_BOOT: boot is a fresh system startup.
 * @PM_RESUME: boot is a resume.
 * @PM_BOOT_ERROR: error, boot cause cannot be identified.
 *
 */
enum pm_boot_status {
	PM_INITIAL_BOOT,
	PM_RESUME,
	PM_BOOT_ERROR,
};

/**
 * enum pm_shutdown_type - enum represents the shutdown type of the PM.
 * @PMF_SHUTDOWN_TYPE_SHUTDOWN: shutdown.
 * @PMF_SHUTDOWN_TYPE_RESET: reset/reboot.
 * @PMF_SHUTDOWN_TYPE_SETSCOPE_ONLY: set the shutdown/reboot scope.
 *
 */
enum pm_shutdown_type {
	PMF_SHUTDOWN_TYPE_SHUTDOWN,
	PMF_SHUTDOWN_TYPE_RESET,
	PMF_SHUTDOWN_TYPE_SETSCOPE_ONLY,
};

/**
 * enum pm_shutdown_subtype - enum represents the shutdown subtype of the PM.
 * @PMF_SHUTDOWN_SUBTYPE_SUBSYSTEM: shutdown/reboot APU subsystem only.
 * @PMF_SHUTDOWN_SUBTYPE_PS_ONLY: shutdown/reboot entire PS (but not PL).
 * @PMF_SHUTDOWN_SUBTYPE_SYSTEM: shutdown/reboot entire system.
 *
 */
enum pm_shutdown_subtype {
	PMF_SHUTDOWN_SUBTYPE_SUBSYSTEM,
	PMF_SHUTDOWN_SUBTYPE_PS_ONLY,
	PMF_SHUTDOWN_SUBTYPE_SYSTEM,
};

/**
 * enum pm_pll_mode - enum represents the mode of the PLL.
 * @PM_PLL_MODE_RESET: PLL is in reset (not locked).
 * @PM_PLL_MODE_INTEGER: PLL is locked in integer mode.
 * @PM_PLL_MODE_FRACTIONAL: PLL is locked in fractional mode.
 * @PM_PLL_MODE_MAX: Represents the maximum mode value for the PLL.
 */
enum pm_pll_mode {
	PM_PLL_MODE_RESET,
	PM_PLL_MODE_INTEGER,
	PM_PLL_MODE_FRACTIONAL,
	PM_PLL_MODE_MAX,
};

/**
 * enum pm_clock_div_id - enum represents the clock division identifiers in the
 *                        PM.
 * @PM_CLOCK_DIV0_ID: Clock divider 0.
 * @PM_CLOCK_DIV1_ID: Clock divider 1.
 */
enum pm_clock_div_id {
	PM_CLOCK_DIV0_ID,
	PM_CLOCK_DIV1_ID,
};

#endif /* ZYNQMP_PM_DEFS_H */
