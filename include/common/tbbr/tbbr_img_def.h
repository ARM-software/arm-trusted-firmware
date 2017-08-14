/*
 * Copyright (c) 2015-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __TBBR_IMG_DEF_H__
#define __TBBR_IMG_DEF_H__

/* Firmware Image Package */
#define FIP_IMAGE_ID			0

/* Trusted Boot Firmware BL2 */
#define BL2_IMAGE_ID			1

/* SCP Firmware SCP_BL2 */
#define SCP_BL2_IMAGE_ID		2

/* EL3 Runtime Firmware BL31 */
#define BL31_IMAGE_ID			3

/* Secure Payload BL32 (Trusted OS) */
#define BL32_IMAGE_ID			4

/* Non-Trusted Firmware BL33 */
#define BL33_IMAGE_ID			5

/* Certificates */
#define TRUSTED_BOOT_FW_CERT_ID		6
#define TRUSTED_KEY_CERT_ID		7

#define SCP_FW_KEY_CERT_ID		8
#define SOC_FW_KEY_CERT_ID		9
#define TRUSTED_OS_FW_KEY_CERT_ID	10
#define NON_TRUSTED_FW_KEY_CERT_ID	11

#define SCP_FW_CONTENT_CERT_ID		12
#define SOC_FW_CONTENT_CERT_ID		13
#define TRUSTED_OS_FW_CONTENT_CERT_ID	14
#define NON_TRUSTED_FW_CONTENT_CERT_ID	15

/* Non-Trusted ROM Firmware NS_BL1U */
#define NS_BL1U_IMAGE_ID		16

/* Trusted FWU Certificate */
#define FWU_CERT_ID			17

/* Trusted FWU SCP Firmware SCP_BL2U */
#define SCP_BL2U_IMAGE_ID		18

/* Trusted FWU Boot Firmware BL2U */
#define BL2U_IMAGE_ID			19

/* Non-Trusted FWU Firmware NS_BL2U */
#define NS_BL2U_IMAGE_ID		20

/* Secure Payload BL32_EXTRA1 (Trusted OS Extra1) */
#define BL32_EXTRA1_IMAGE_ID		21

/* Secure Payload BL32_EXTRA2 (Trusted OS Extra2) */
#define BL32_EXTRA2_IMAGE_ID		22

#endif /* __TBBR_IMG_DEF_H__ */
