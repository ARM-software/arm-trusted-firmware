/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ROCKCHIP_SIP_SVC_H
#define ROCKCHIP_SIP_SVC_H

/* SMC function IDs for SiP Service queries */
#define SIP_SVC_CALL_COUNT		0x8200ff00
#define SIP_SVC_UID			0x8200ff01
#define SIP_SVC_VERSION			0x8200ff03

/* rockchip SiP Service Calls version numbers */
#define RK_SIP_SVC_VERSION_MAJOR	0x0
#define RK_SIP_SVC_VERSION_MINOR	0x1

/* Number of ROCKCHIP SiP Calls implemented */
#define RK_COMMON_SIP_NUM_CALLS		0x3

enum {
	RK_SIP_E_SUCCESS = 0,
	RK_SIP_E_INVALID_PARAM = -1
};

#endif /* ROCKCHIP_SIP_SVC_H */
