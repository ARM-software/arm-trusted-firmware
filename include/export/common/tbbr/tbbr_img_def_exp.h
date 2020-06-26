/*
 * Copyright (c) 2019-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ARM_TRUSTED_FIRMWARE_EXPORT_COMMON_TBBR_TBBR_IMG_DEF_EXP_H
#define ARM_TRUSTED_FIRMWARE_EXPORT_COMMON_TBBR_TBBR_IMG_DEF_EXP_H

/* EXPORT HEADER -- See include/export/README for details! -- EXPORT HEADER */

#include "../../lib/utils_def_exp.h"

/* Firmware Image Package */
#define FIP_IMAGE_ID			U(0)

/* Trusted Boot Firmware BL2 */
#define BL2_IMAGE_ID			U(1)

/* SCP Firmware SCP_BL2 */
#define SCP_BL2_IMAGE_ID		U(2)

/* EL3 Runtime Firmware BL31 */
#define BL31_IMAGE_ID			U(3)

/* Secure Payload BL32 (Trusted OS) */
#define BL32_IMAGE_ID			U(4)

/* Non-Trusted Firmware BL33 */
#define BL33_IMAGE_ID			U(5)

/* Certificates */
#define TRUSTED_BOOT_FW_CERT_ID		U(6)
#define TRUSTED_KEY_CERT_ID		U(7)

#define SCP_FW_KEY_CERT_ID		U(8)
#define SOC_FW_KEY_CERT_ID		U(9)
#define TRUSTED_OS_FW_KEY_CERT_ID	U(10)
#define NON_TRUSTED_FW_KEY_CERT_ID	U(11)

#define SCP_FW_CONTENT_CERT_ID		U(12)
#define SOC_FW_CONTENT_CERT_ID		U(13)
#define TRUSTED_OS_FW_CONTENT_CERT_ID	U(14)
#define NON_TRUSTED_FW_CONTENT_CERT_ID	U(15)

/* Non-Trusted ROM Firmware NS_BL1U */
#define NS_BL1U_IMAGE_ID		U(16)

/* Trusted FWU Certificate */
#define FWU_CERT_ID			U(17)

/* Trusted FWU SCP Firmware SCP_BL2U */
#define SCP_BL2U_IMAGE_ID		U(18)

/* Trusted FWU Boot Firmware BL2U */
#define BL2U_IMAGE_ID			U(19)

/* Non-Trusted FWU Firmware NS_BL2U */
#define NS_BL2U_IMAGE_ID		U(20)

/* Secure Payload BL32_EXTRA1 (Trusted OS Extra1) */
#define BL32_EXTRA1_IMAGE_ID		U(21)

/* Secure Payload BL32_EXTRA2 (Trusted OS Extra2) */
#define BL32_EXTRA2_IMAGE_ID		U(22)

/* HW_CONFIG (e.g. Kernel DT) */
#define HW_CONFIG_ID			U(23)

/* TB_FW_CONFIG */
#define TB_FW_CONFIG_ID			U(24)

/* SOC_FW_CONFIG */
#define SOC_FW_CONFIG_ID		U(25)

/* TOS_FW_CONFIG */
#define TOS_FW_CONFIG_ID		U(26)

/* NT_FW_CONFIG */
#define NT_FW_CONFIG_ID			U(27)

/* GPT Partition */
#define GPT_IMAGE_ID			U(28)

/* Binary with STM32 header */
#define STM32_IMAGE_ID			U(29)

/* Encrypted image identifier */
#define ENC_IMAGE_ID			U(30)

/* FW_CONFIG */
#define FW_CONFIG_ID			U(31)

/* Max Images */
#define MAX_IMAGE_IDS			U(32)

#endif /* ARM_TRUSTED_FIRMWARE_EXPORT_COMMON_TBBR_TBBR_IMG_DEF_EXP_H */
