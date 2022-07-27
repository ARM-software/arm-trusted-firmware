/*
 * Copyright (c) 2024, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef STM32MP2_SMC_H
#define STM32MP2_SMC_H

#define STM32_COMMON_SIP_NUM_CALLS			1U

/*
 * STM32_SIP_SMC_STGEN_SET_RATE call API
 * This service is opened to secure world only.
 *
 * Argument a0: (input) SMCC ID
 *		(output) status return code
 * Argument a1: (input) Frequency to set (given by sender)
 */
#define STM32_SIP_SMC_STGEN_SET_RATE                    0x82000000

#endif /* STM32MP2_SMC_H */
