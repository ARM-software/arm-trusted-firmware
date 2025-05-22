/*
 * Copyright (c) 2025, Texas Instruments. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef K3_SIP_SVC_H
#define K3_SIP_SVC_H

#include <stdint.h>

/* SMC function IDs for SiP Service queries */
#define SIP_SVC_CALL_COUNT      0x8200ff00
#define SIP_SVC_UID         0x8200ff01
#define SIP_SVC_VERSION         0x8200ff03
#define K3_SIP_SCMI_AGENT0      0x82004000
#define K3_SIP_OTP_WRITEBUFF    0xC2000000
#define K3_SIP_OTP_WRITE        0xC2000001
#define K3_SIP_OTP_READ         0xC2000002

/* TI SiP Service Calls version numbers */
#define K3_SIP_SVC_VERSION_MAJOR    0x0
#define K3_SIP_SVC_VERSION_MINOR    0x1

/* Number of TI SiP Calls implemented */
#define K3_COMMON_SIP_NUM_CALLS     0x6

/* TI Fuse writebuff SMC handler */
int ti_fuse_writebuff_handler(u_register_t x1);

#endif /* K3_SIP_SVC_H */
