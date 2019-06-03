/*
 * Copyright (c) 2019, Xilinx, Inc. All rights reserved.
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

#define PM_GET_CALLBACK_DATA		0xa01
#define PM_GET_TRUSTZONE_VERSION	0xa03

/* PM API Versions */
#define PM_API_BASE_VERSION		1U

/* PM API ids */
#define PM_GET_API_VERSION		1U
#define PM_GET_DEVICE_STATUS		3U
#define PM_GET_OP_CHARACTERISTIC	4U
#define PM_REQ_SUSPEND			6U
#define PM_SELF_SUSPEND			7U
#define PM_FORCE_POWERDOWN		8U
#define PM_ABORT_SUSPEND		9U
#define PM_REQ_WAKEUP			10U
#define PM_SET_WAKEUP_SOURCE		11U
#define PM_SYSTEM_SHUTDOWN		12U
#define PM_REQUEST_DEVICE		13U
#define PM_RELEASE_DEVICE		14U
#define PM_SET_REQUIREMENT		15U
#define PM_RESET_ASSERT			17U
#define PM_RESET_GET_STATUS		18U
#define PM_INIT_FINALIZE		21U
#define PM_GET_CHIPID			24U
#define	PM_PINCTRL_REQUEST		28U
#define	PM_PINCTRL_RELEASE		29U
#define	PM_PINCTRL_GET_FUNCTION		30U
#define	PM_PINCTRL_SET_FUNCTION		31U
#define	PM_PINCTRL_CONFIG_PARAM_GET	32U
#define	PM_PINCTRL_CONFIG_PARAM_SET	33U
#define PM_IOCTL			34U
#define PM_QUERY_DATA			35U
#define PM_CLOCK_ENABLE			36U
#define PM_CLOCK_DISABLE		37U
#define PM_CLOCK_GETSTATE		38U
#define PM_CLOCK_SETDIVIDER		39U
#define PM_CLOCK_GETDIVIDER		40U
#define PM_CLOCK_SETRATE		41U
#define PM_CLOCK_GETRATE		42U
#define PM_CLOCK_SETPARENT		43U
#define PM_CLOCK_GETPARENT		44U
#define PM_PLL_SET_PARAMETER		48U
#define PM_PLL_GET_PARAMETER		49U
#define PM_PLL_SET_MODE			50U
#define PM_PLL_GET_MODE			51U
#define PM_FEATURE_CHECK		63U

/* Loader API ids */
#define PM_LOAD_PDI			0x701U

/* IOCTL IDs for clock driver */
#define IOCTL_SET_PLL_FRAC_MODE		8
#define	IOCTL_GET_PLL_FRAC_MODE		9
#define	IOCTL_SET_PLL_FRAC_DATA		10
#define	IOCTL_GET_PLL_FRAC_DATA		11

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
#endif /* PM_DEFS_H */
