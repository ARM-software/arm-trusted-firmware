/*
 * Copyright (c) 2016-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ARM_SIP_SVC_H
#define ARM_SIP_SVC_H

#include <lib/utils_def.h>

/* SMC function IDs for SiP Service queries */

#define ARM_SIP_SVC_CALL_COUNT		U(0x8200ff00)
#define ARM_SIP_SVC_UID			U(0x8200ff01)
/*					U(0x8200ff02) is reserved */
#define ARM_SIP_SVC_VERSION		U(0x8200ff03)

/* PMF_SMC_GET_TIMESTAMP_32		0x82000010 */
/* PMF_SMC_GET_TIMESTAMP_64		0xC2000010 */

/* Function ID for requesting state switch of lower EL */
#define ARM_SIP_SVC_EXE_STATE_SWITCH	U(0x82000020)

/* DEBUGFS_SMC_32			0x82000030U */
/* DEBUGFS_SMC_64			0xC2000030U */

/* ARM SiP Service Calls version numbers */
#define ARM_SIP_SVC_VERSION_MAJOR		U(0x0)
#define ARM_SIP_SVC_VERSION_MINOR		U(0x2)

#endif /* ARM_SIP_SVC_H */
