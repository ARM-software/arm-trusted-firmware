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

/* Processor core device IDs */
#define APU_DEVID(IDX)	NODEID(XPM_NODECLASS_DEVICE, XPM_NODESUBCL_DEV_CORE, \
			       XPM_NODETYPE_DEV_CORE_APU, (IDX))

#define XPM_DEVID_ACPU_0	APU_DEVID(XPM_NODEIDX_DEV_ACPU_0)
#define XPM_DEVID_ACPU_1	APU_DEVID(XPM_NODEIDX_DEV_ACPU_1)

/*********************************************************************
 * Enum definitions
 ********************************************************************/

/**
 * @PM_RET_SUCCESS:		success
 * @PM_RET_ERROR_ARGS:		illegal arguments provided (deprecated)
 * @PM_RET_ERROR_NOTSUPPORTED:	feature not supported  (deprecated)
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
