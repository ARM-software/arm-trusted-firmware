/*
 * Copyright (c) 2024, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef METADATA_H
#define METADATA_H

/*
 * Images, measured during the boot process, have some associated metadata.
 * One of these types of metadata is the image identifier strings. These macros
 * define these strings. They are used across the different measured boot
 * backends.
 * Note that these strings follow the standardization recommendations
 * defined in the Arm Server Base Security Guide (a.k.a. SBSG, Arm DEN 0086),
 * where applicable. They should not be changed in the code.
 * Where the SBSG does not make recommendations, we are free to choose any
 * naming convention.
 * The key thing is to choose meaningful strings so that when the measured boot
 * metadata is used in attestation, the different components can be identified.
 */
#define MBOOT_BL2_IMAGE_STRING		"BL_2"
#define MBOOT_BL31_IMAGE_STRING		"SECURE_RT_EL3"
#if defined(SPD_opteed)
#define MBOOT_BL32_IMAGE_STRING		"SECURE_RT_EL1_OPTEE"
#elif defined(SPD_tspd)
#define MBOOT_BL32_IMAGE_STRING		"SECURE_RT_EL1_TSPD"
#elif defined(SPD_tlkd)
#define MBOOT_BL32_IMAGE_STRING		"SECURE_RT_EL1_TLKD"
#elif defined(SPD_trusty)
#define MBOOT_BL32_IMAGE_STRING		"SECURE_RT_EL1_TRUSTY"
#else
#define MBOOT_BL32_IMAGE_STRING		"SECURE_RT_EL1_UNKNOWN"
#endif /* SPD_opteed */
#define MBOOT_BL32_EXTRA1_IMAGE_STRING	"SECURE_RT_EL1_OPTEE_EXTRA1"
#define MBOOT_BL32_EXTRA2_IMAGE_STRING	"SECURE_RT_EL1_OPTEE_EXTRA2"
#define MBOOT_BL33_IMAGE_STRING		"BL_33"
#define MBOOT_FW_CONFIG_STRING		"FW_CONFIG"
#define MBOOT_HW_CONFIG_STRING		"HW_CONFIG"
#define MBOOT_NT_FW_CONFIG_STRING	"NT_FW_CONFIG"
#define MBOOT_SCP_BL2_IMAGE_STRING	"SYS_CTRL_2"
#define MBOOT_SOC_FW_CONFIG_STRING	"SOC_FW_CONFIG"
#define MBOOT_STM32_STRING		"STM32"
#define MBOOT_TB_FW_CONFIG_STRING	"TB_FW_CONFIG"
#define MBOOT_TOS_FW_CONFIG_STRING	"TOS_FW_CONFIG"
#define MBOOT_RMM_IMAGE_STRING		"RMM"
#define MBOOT_SP1_STRING		"SP1"
#define MBOOT_SP2_STRING		"SP2"
#define MBOOT_SP3_STRING		"SP3"
#define MBOOT_SP4_STRING		"SP4"
#define MBOOT_SP5_STRING		"SP5"
#define MBOOT_SP6_STRING		"SP6"
#define MBOOT_SP7_STRING		"SP7"
#define MBOOT_SP8_STRING		"SP8"

#endif /* METADATA_H */
