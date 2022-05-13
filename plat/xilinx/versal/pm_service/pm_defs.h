/*
 * Copyright (c) 2019-2022, Xilinx, Inc. All rights reserved.
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

#define PERIPH_DEVID(IDX)	NODEID(XPM_NODECLASS_DEVICE, \
				       XPM_NODESUBCL_DEV_PERIPH, \
				       XPM_NODETYPE_DEV_PERIPH, (IDX))

#define PM_GET_CALLBACK_DATA		0xa01U
#define PM_GET_TRUSTZONE_VERSION	0xa03U
#define TF_A_PM_REGISTER_SGI		0xa04U

/* PM API Versions */
#define PM_API_BASE_VERSION		1U
#define PM_API_VERSION_2                2U

#define PM_API_QUERY_DATA_VERSION	2U

/* PM API ids */
#define PM_REGISTER_NOTIFIER		5U
#define PM_REQ_SUSPEND			6U
#define PM_SELF_SUSPEND			7U
#define PM_FORCE_POWERDOWN		8U
#define PM_ABORT_SUSPEND		9U
#define PM_REQ_WAKEUP			10U
#define PM_SET_WAKEUP_SOURCE		11U
#define PM_SYSTEM_SHUTDOWN		12U
#define PM_IOCTL			34U
#define PM_QUERY_DATA			35U
#define PM_PLL_SET_PARAMETER		48U
#define PM_PLL_GET_PARAMETER		49U
#define PM_PLL_SET_MODE			50U
#define PM_PLL_GET_MODE			51U
#define PM_FEATURE_CHECK		63U

/* Loader API ids */
#define PM_LOAD_PDI			0x701U

/* IOCTL IDs for clock driver */
#define IOCTL_SET_PLL_FRAC_MODE		8U
#define	IOCTL_GET_PLL_FRAC_MODE		9U
#define	IOCTL_SET_PLL_FRAC_DATA		10U
#define	IOCTL_GET_PLL_FRAC_DATA		11U
#define	IOCTL_SET_SGI			25U

/* Parameter ID for PLL IOCTLs */
/* Fractional data portion for PLL */
#define PM_PLL_PARAM_DATA	2

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

/**
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

/**
 * @PM_RET_SUCCESS:		success
 * @PM_RET_ERROR_ARGS:		illegal arguments provided (deprecated)
 * @PM_RET_ERROR_NOTSUPPORTED:	feature not supported  (deprecated)
 * @PM_RET_ERROR_NOFEATURE:	feature is not available
 * @PM_RET_ERROR_INTERNAL:	internal error
 * @PM_RET_ERROR_CONFLICT:	conflict
 * @PM_RET_ERROR_ACCESS:	access rights violation
 * @PM_RET_ERROR_INVALID_NODE:	invalid node
 * @PM_RET_ERROR_DOUBLE_REQ:	duplicate request for same node
 * @PM_RET_ERROR_ABORT_SUSPEND:	suspend procedure has been aborted
 * @PM_RET_ERROR_TIMEOUT:	timeout in communication with PMU
 * @PM_RET_ERROR_NODE_USED:	node is already in use
 */
enum pm_ret_status {
	PM_RET_SUCCESS,
	PM_RET_ERROR_ARGS = 1,
	PM_RET_ERROR_NOTSUPPORTED = 4,
	PM_RET_ERROR_NOFEATURE = 19,
	PM_RET_ERROR_INTERNAL = 2000,
	PM_RET_ERROR_CONFLICT = 2001,
	PM_RET_ERROR_ACCESS = 2002,
	PM_RET_ERROR_INVALID_NODE = 2003,
	PM_RET_ERROR_DOUBLE_REQ = 2004,
	PM_RET_ERROR_ABORT_SUSPEND = 2005,
	PM_RET_ERROR_TIMEOUT = 2006,
	PM_RET_ERROR_NODE_USED = 2007
};

/**
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
