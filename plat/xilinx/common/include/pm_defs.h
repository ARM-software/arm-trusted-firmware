/*
 * Copyright (c) 2019-2022, Xilinx, Inc. All rights reserved.
 * Copyright (c) 2022-2023, Advanced Micro Devices, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Versal power management enums and defines */

#ifndef PM_DEFS_H
#define PM_DEFS_H

#include "pm_node.h"

/*********************************************************************
 * Macro definitions
 ********************************************************************/

/* State arguments of the self suspend */
#define PM_STATE_CPU_IDLE	0x0U
#define PM_STATE_SUSPEND_TO_RAM	0xFU

#define MAX_LATENCY		(~0U)
#define MAX_QOS			100U

/* Processor core device IDs */
#define APU_DEVID(IDX)	NODEID(XPM_NODECLASS_DEVICE, XPM_NODESUBCL_DEV_CORE, \
			       XPM_NODETYPE_DEV_CORE_APU, (IDX))

#define XPM_DEVID_ACPU_0	APU_DEVID(XPM_NODEIDX_DEV_ACPU_0)
#define XPM_DEVID_ACPU_1	APU_DEVID(XPM_NODEIDX_DEV_ACPU_1)

#define PERIPH_DEVID(IDX)	NODEID((uint32_t)XPM_NODECLASS_DEVICE, \
				       (uint32_t)XPM_NODESUBCL_DEV_PERIPH, \
				       (uint32_t)XPM_NODETYPE_DEV_PERIPH, (IDX))

#define PM_GET_CALLBACK_DATA		0xa01U
#define PM_GET_TRUSTZONE_VERSION	0xa03U
#define TF_A_PM_REGISTER_SGI		0xa04U

/* PM API Versions */
#define PM_API_BASE_VERSION		1U
#define PM_API_VERSION_2		2U

/* Loader API ids */
#define PM_LOAD_PDI			0x701U
#define PM_LOAD_GET_HANDOFF_PARAMS	0x70BU

/* System shutdown macros */
#define	XPM_SHUTDOWN_TYPE_SHUTDOWN	0U
#define	XPM_SHUTDOWN_TYPE_RESET		1U
#define	XPM_SHUTDOWN_TYPE_SETSCOPE_ONLY	2U

#define	XPM_SHUTDOWN_SUBTYPE_RST_SUBSYSTEM	0U
#define	XPM_SHUTDOWN_SUBTYPE_RST_PS_ONLY	1U
#define	XPM_SHUTDOWN_SUBTYPE_RST_SYSTEM		2U

/*********************************************************************
 * Enum definitions
 ********************************************************************/

/*
 * ioctl id
 */
enum {
	IOCTL_GET_RPU_OPER_MODE = 0,
	IOCTL_SET_RPU_OPER_MODE = 1,
	IOCTL_RPU_BOOT_ADDR_CONFIG = 2,
	IOCTL_TCM_COMB_CONFIG = 3,
	IOCTL_SET_TAPDELAY_BYPASS = 4,
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
	/* Register SGI to TF-A */
	IOCTL_SET_SGI = 25,
};

/**
 * enum pm_pll_param - enum represents the parameters for a phase-locked loop.
 * @PM_PLL_PARAM_DIV2: Enable for divide by 2 function inside the PLL.
 * @PM_PLL_PARAM_FBDIV: Feedback divisor integer portion for the PLL.
 * @PM_PLL_PARAM_DATA: Feedback divisor fractional portion for the PLL.
 * @PM_PLL_PARAM_PRE_SRC: Clock source for PLL input.
 * @PM_PLL_PARAM_POST_SRC: Clock source for PLL Bypass mode.
 * @PM_PLL_PARAM_LOCK_DLY: Lock circuit config settings for lock windowsize.
 * @PM_PLL_PARAM_LOCK_CNT: Lock circuit counter setting.
 * @PM_PLL_PARAM_LFHF: PLL loop filter high frequency capacitor control.
 * @PM_PLL_PARAM_CP: PLL charge pump control.
 * @PM_PLL_PARAM_RES: PLL loop filter resistor control.
 * @PM_PLL_PARAM_MAX: Represents the maximum parameter value for the PLL
 */
enum pm_pll_param {
	PM_PLL_PARAM_DIV2,
	PM_PLL_PARAM_FBDIV,
	PM_PLL_PARAM_DATA,
	PM_PLL_PARAM_PRE_SRC,
	PM_PLL_PARAM_POST_SRC,
	PM_PLL_PARAM_LOCK_DLY,
	PM_PLL_PARAM_LOCK_CNT,
	PM_PLL_PARAM_LFHF,
	PM_PLL_PARAM_CP,
	PM_PLL_PARAM_RES,
	PM_PLL_PARAM_MAX,
};

enum pm_api_id {
	/* Miscellaneous API functions: */
	PM_GET_API_VERSION = 1, /* Do not change or move */
	PM_SET_CONFIGURATION,
	PM_GET_NODE_STATUS,
	PM_GET_OP_CHARACTERISTIC,
	PM_REGISTER_NOTIFIER,
	/* API for suspending of PUs: */
	PM_REQ_SUSPEND,
	PM_SELF_SUSPEND,
	PM_FORCE_POWERDOWN,
	PM_ABORT_SUSPEND,
	PM_REQ_WAKEUP,
	PM_SET_WAKEUP_SOURCE,
	PM_SYSTEM_SHUTDOWN,
	/* API for managing PM slaves: */
	PM_REQ_NODE,
	PM_RELEASE_NODE,
	PM_SET_REQUIREMENT,
	PM_SET_MAX_LATENCY,
	/* Direct control API functions: */
	PM_RESET_ASSERT,
	PM_RESET_GET_STATUS,
	PM_MMIO_WRITE,
	PM_MMIO_READ,
	PM_INIT_FINALIZE,
	PM_FPGA_LOAD,
	PM_FPGA_GET_STATUS,
	PM_GET_CHIPID,
	PM_SECURE_RSA_AES,
	PM_SECURE_SHA,
	PM_SECURE_RSA,
	PM_PINCTRL_REQUEST,
	PM_PINCTRL_RELEASE,
	PM_PINCTRL_GET_FUNCTION,
	PM_PINCTRL_SET_FUNCTION,
	PM_PINCTRL_CONFIG_PARAM_GET,
	PM_PINCTRL_CONFIG_PARAM_SET,
	PM_IOCTL,
	/* API to query information from firmware */
	PM_QUERY_DATA,
	/* Clock control API functions */
	PM_CLOCK_ENABLE,
	PM_CLOCK_DISABLE,
	PM_CLOCK_GETSTATE,
	PM_CLOCK_SETDIVIDER,
	PM_CLOCK_GETDIVIDER,
	PM_CLOCK_SETPARENT = 43,
	PM_CLOCK_GETPARENT,
	PM_SECURE_IMAGE,
	/* FPGA PL Readback */
	PM_FPGA_READ,
	PM_SECURE_AES,
	/* PLL control API functions */
	PM_PLL_SET_PARAMETER,
	PM_PLL_GET_PARAMETER,
	PM_PLL_SET_MODE,
	PM_PLL_GET_MODE,
	/* PM Register Access API */
	PM_REGISTER_ACCESS,
	PM_EFUSE_ACCESS,
	PM_FPGA_GET_VERSION,
	PM_FPGA_GET_FEATURE_LIST,
	PM_FEATURE_CHECK = 63,
	PM_API_MAX = 74
};

enum pm_abort_reason {
	ABORT_REASON_WKUP_EVENT = 100,
	ABORT_REASON_PU_BUSY,
	ABORT_REASON_NO_PWRDN,
	ABORT_REASON_UNKNOWN,
};

enum pm_opchar_type {
	PM_OPCHAR_TYPE_POWER = 1,
	PM_OPCHAR_TYPE_TEMP,
	PM_OPCHAR_TYPE_LATENCY,
};

/*
 * Subsystem IDs
 */
typedef enum {
	XPM_SUBSYSID_PMC,
	XPM_SUBSYSID_PSM,
	XPM_SUBSYSID_APU,
	XPM_SUBSYSID_RPU0_LOCK,
	XPM_SUBSYSID_RPU0_0,
	XPM_SUBSYSID_RPU0_1,
	XPM_SUBSYSID_DDR0,
	XPM_SUBSYSID_ME,
	XPM_SUBSYSID_PL,
	XPM_SUBSYSID_MAX,
} XPm_SubsystemId;

/* TODO: move pm_ret_status from device specific location to common location */
/**
 * enum pm_ret_status - enum represents the return status codes for a PM
 *                      operation.
 * @PM_RET_SUCCESS: success.
 * @PM_RET_ERROR_ARGS: illegal arguments provided (deprecated).
 * @PM_RET_ERROR_NOTSUPPORTED: feature not supported  (deprecated).
 * @PM_RET_ERROR_NOFEATURE: feature is not available.
 * @PM_RET_ERROR_INVALID_CRC: invalid crc in IPI communication.
 * @PM_RET_ERROR_NOT_ENABLED: feature is not enabled.
 * @PM_RET_ERROR_INTERNAL: internal error.
 * @PM_RET_ERROR_CONFLICT: conflict.
 * @PM_RET_ERROR_ACCESS: access rights violation.
 * @PM_RET_ERROR_INVALID_NODE: invalid node.
 * @PM_RET_ERROR_DOUBLE_REQ: duplicate request for same node.
 * @PM_RET_ERROR_ABORT_SUSPEND: suspend procedure has been aborted.
 * @PM_RET_ERROR_TIMEOUT: timeout in communication with PMU.
 * @PM_RET_ERROR_NODE_USED: node is already in use.
 * @PM_RET_ERROR_NO_FEATURE: indicates that the requested feature is not
 *                           supported.
 */
enum pm_ret_status {
	PM_RET_SUCCESS,
	PM_RET_ERROR_ARGS = 1,
	PM_RET_ERROR_NOTSUPPORTED = 4,
	PM_RET_ERROR_NOFEATURE = 19,
	PM_RET_ERROR_INVALID_CRC = 301,
	PM_RET_ERROR_NOT_ENABLED = 29,
	PM_RET_ERROR_INTERNAL = 2000,
	PM_RET_ERROR_CONFLICT = 2001,
	PM_RET_ERROR_ACCESS = 2002,
	PM_RET_ERROR_INVALID_NODE = 2003,
	PM_RET_ERROR_DOUBLE_REQ = 2004,
	PM_RET_ERROR_ABORT_SUSPEND = 2005,
	PM_RET_ERROR_TIMEOUT = 2006,
	PM_RET_ERROR_NODE_USED = 2007,
	PM_RET_ERROR_NO_FEATURE = 2008
};

/*
 * Qids
 */
enum pm_query_id {
	XPM_QID_INVALID,
	XPM_QID_CLOCK_GET_NAME,
	XPM_QID_CLOCK_GET_TOPOLOGY,
	XPM_QID_CLOCK_GET_FIXEDFACTOR_PARAMS,
	XPM_QID_CLOCK_GET_MUXSOURCES,
	XPM_QID_CLOCK_GET_ATTRIBUTES,
	XPM_QID_PINCTRL_GET_NUM_PINS,
	XPM_QID_PINCTRL_GET_NUM_FUNCTIONS,
	XPM_QID_PINCTRL_GET_NUM_FUNCTION_GROUPS,
	XPM_QID_PINCTRL_GET_FUNCTION_NAME,
	XPM_QID_PINCTRL_GET_FUNCTION_GROUPS,
	XPM_QID_PINCTRL_GET_PIN_GROUPS,
	XPM_QID_CLOCK_GET_NUM_CLOCKS,
	XPM_QID_CLOCK_GET_MAX_DIVISOR,
	XPM_QID_PLD_GET_PARENT,
};
#endif /* PM_DEFS_H */
