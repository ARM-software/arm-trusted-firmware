/*
 * Copyright (c) 2016-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __ARM_SIP_SVC_H__
#define __ARM_SIP_SVC_H__

/* SMC function IDs for SiP Service queries */

#define ARM_SIP_SVC_CALL_COUNT		0x8200ff00
#define ARM_SIP_SVC_UID			0x8200ff01
/*					0x8200ff02 is reserved */
#define ARM_SIP_SVC_VERSION		0x8200ff03

/* Function ID for requesting state switch of lower EL */
#define ARM_SIP_SVC_EXE_STATE_SWITCH	0x82000020

/* ARM SiP Service Calls version numbers */
#define ARM_SIP_SVC_VERSION_MAJOR		0x0
#define ARM_SIP_SVC_VERSION_MINOR		0x2

#endif /* __ARM_SIP_SVC_H__ */
