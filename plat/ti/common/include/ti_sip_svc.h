/*
 * Copyright (C) 2026 Texas Instruments Incorporated - https://www.ti.com/
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef K3_SIP_SVC_H
#define K3_SIP_SVC_H

/* SMC function IDs for SiP Service queries */
#define SIP_SVC_CALL_COUNT	0x8200ff00U
#define SIP_SVC_UID		0x8200ff01U
#define SIP_SVC_VERSION		0x8200ff03U
#define K3_SIP_SCMI_AGENT0	0x82004000U

/* TI SiP Service Calls version numbers */
#define K3_SIP_SVC_VERSION_MAJOR	0x0U
#define K3_SIP_SVC_VERSION_MINOR	0x1U

/* Number of TI SiP Calls implemented */
#define K3_COMMON_SIP_NUM_CALLS		0x3U

#endif /* K3_SIP_SVC_H */
