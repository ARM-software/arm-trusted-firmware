/*
 * Copyright (c) 2016-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __HISI_SIP_SVC_H__
#define __HISI_SIP_SVC_H__

/* SMC function IDs for SiP Service queries */

#define HISI_SIP_SVC_CALL_COUNT		0x8200ff00
#define HISI_SIP_SVC_UID			0x8200ff01
/*					0x8200ff02 is reserved */
#define HISI_SIP_SVC_VERSION			0x8200ff03

/* HISI SiP Service Calls version numbers */
#define HISI_SIP_SVC_VERSION_MAJOR		0x0
#define HISI_SIP_SVC_VERSION_MINOR		0x1

#endif /* __ARM_SIP_SVC_H__ */
