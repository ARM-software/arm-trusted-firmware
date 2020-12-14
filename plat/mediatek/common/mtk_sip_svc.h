/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef MTK_SIP_SVC_H
#define MTK_SIP_SVC_H

#include <stdint.h>

/* SMC function IDs for SiP Service queries */
#define SIP_SVC_CALL_COUNT		0x8200ff00
#define SIP_SVC_UID			0x8200ff01
/*					0x8200ff02 is reserved */
#define SIP_SVC_VERSION			0x8200ff03

/* Mediatek SiP Service Calls version numbers */
#define MTK_SIP_SVC_VERSION_MAJOR	0x0
#define MTK_SIP_SVC_VERSION_MINOR	0x1

#define SMC_AARCH64_BIT		0x40000000

/* Number of Mediatek SiP Calls implemented */
#define MTK_COMMON_SIP_NUM_CALLS	4

/* Mediatek SiP Service Calls function IDs */
#define MTK_SIP_SET_AUTHORIZED_SECURE_REG	0x82000001

/* For MTK SMC from Secure OS */
/* 0x82000000 - 0x820000FF & 0xC2000000 - 0xC20000FF */
#define MTK_SIP_KERNEL_BOOT_AARCH32		0x82000200
#define MTK_SIP_KERNEL_BOOT_AARCH64		0xC2000200

/* VCORE */
#define MTK_SIP_VCORE_CONTROL_ARCH32		0x82000506
#define MTK_SIP_VCORE_CONTROL_ARCH64		0xC2000506

/* Mediatek SiP Calls error code */
enum {
	MTK_SIP_E_SUCCESS = 0,
	MTK_SIP_E_INVALID_PARAM = -1,
	MTK_SIP_E_NOT_SUPPORTED = -2,
	MTK_SIP_E_INVALID_RANGE = -3,
	MTK_SIP_E_PERMISSION_DENY = -4,
	MTK_SIP_E_LOCK_FAIL = -5
};

/*
 * This function should be implemented in Mediatek SOC directory. It fullfills
 * MTK_SIP_SET_AUTHORIZED_SECURE_REG SiP call by checking the sreg with the
 * predefined secure register list, if a match was found, set val to sreg.
 *
 * Return MTK_SIP_E_SUCCESS on success, and MTK_SIP_E_INVALID_PARAM on failure.
 */
uint64_t mt_sip_set_authorized_sreg(uint32_t sreg, uint32_t val);

#endif /* MTK_SIP_SVC_H */
