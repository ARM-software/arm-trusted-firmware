/*
 * Copyright (c) 2024, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef STM32MP_SVC_SETUP_H
#define STM32MP_SVC_SETUP_H

#include <stdbool.h>
#include <stdint.h>

/* Common SMC function IDs for STM32 Service queries across STM32MP paltforms */
#define STM32_SIP_SVC_CALL_COUNT	0x8200ff00
#define STM32_SIP_SVC_UID		0x8200ff01
/*					0x8200ff02 is reserved */
#define STM32_SIP_SVC_VERSION		0x8200ff03

/* STM32 SiP Service Calls version numbers */
#define STM32_SIP_SVC_VERSION_MAJOR	0x0
#define STM32_SIP_SVC_VERSION_MINOR	0x1

/* SMC error codes */
#define STM32_SMC_OK			0x00000000U
#define STM32_SMC_NOT_SUPPORTED		0xFFFFFFFFU
#define STM32_SMC_FAILED		0xFFFFFFFEU
#define STM32_SMC_INVALID_PARAMS	0xFFFFFFFDU

void plat_svc_smc_handler(uint32_t smc_fid, u_register_t x1,
			  u_register_t x2, u_register_t x3,
			  u_register_t x4, uint32_t *ret1,
			  uint32_t *ret2, bool *ret2_enabled,
			  u_register_t flags);

#endif /* STM32MP_SVC_SETUP_H */
