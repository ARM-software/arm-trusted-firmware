/*
 * Copyright (c) 2016-2019, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef STM32MP1_SMC_H
#define STM32MP1_SMC_H

/*
 * SMC function IDs for STM32 Service queries
 * STM32 SMC services use the space between 0x82000000 and 0x8200FFFF
 * like this is defined in SMC calling Convention by ARM
 * for SiP (silicon Partner)
 * https://developer.arm.com/docs/den0028/latest
 */

/* Secure Service access from Non-secure */

/*
 * STM32_SMC_BSEC call API
 *
 * Argument a0: (input) SMCC ID
 *		(output) status return code
 * Argument a1: (input) Service ID (STM32_SMC_BSEC_xxx)
 * Argument a2: (input) OTP index
 *		(output) OTP read value, if applicable
 * Argument a3: (input) OTP value if applicable
 */
#define STM32_SMC_BSEC			0x82001003

/* SMC function IDs for SiP Service queries */
#define STM32_SIP_SVC_CALL_COUNT	0x8200ff00
#define STM32_SIP_SVC_UID		0x8200ff01
/*					0x8200ff02 is reserved */
#define STM32_SIP_SVC_VERSION		0x8200ff03

/* STM32 SiP Service Calls version numbers */
#define STM32_SIP_SVC_VERSION_MAJOR	0x0
#define STM32_SIP_SVC_VERSION_MINOR	0x1

/* Number of STM32 SiP Calls implemented */
#define STM32_COMMON_SIP_NUM_CALLS	4

/* Service for BSEC */
#define STM32_SMC_READ_SHADOW		0x01
#define STM32_SMC_PROG_OTP		0x02
#define STM32_SMC_WRITE_SHADOW		0x03
#define STM32_SMC_READ_OTP		0x04

#endif /* STM32MP1_SMC_H */
