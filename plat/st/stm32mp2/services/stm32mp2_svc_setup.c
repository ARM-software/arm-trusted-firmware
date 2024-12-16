/*
 * Copyright (c) 2024, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <stdint.h>

#include <common/debug.h>
#include <common/runtime_svc.h>
#include <lib/mmio.h>

#include <stm32mp2_smc.h>
#include <stm32mp_svc_setup.h>

#include "stgen_svc.h"

/*
 * Platform-level Standard Service SIP SMC handler. This handler will dispatch
 * the SMC to the correct feature handler.
 */
void plat_svc_smc_handler(uint32_t smc_fid, u_register_t x1,
			  u_register_t x2, u_register_t x3,
			  u_register_t x4, uint32_t *ret1,
			  uint32_t *ret2, bool *ret2_enabled,
			  u_register_t flags)
{
	switch (smc_fid) {
	case STM32_SIP_SVC_CALL_COUNT:
		*ret1 = STM32_COMMON_SIP_NUM_CALLS;
		break;
	case STM32_SIP_SMC_STGEN_SET_RATE:
		if (!is_caller_secure(flags)) {
			*ret1 = STM32_SMC_FAILED;
			break;
		}

		*ret1 = stgen_svc_handler();
		break;
	default:
		WARN("Unimplemented STM32MP2 Service Call: 0x%x\n", smc_fid);
		*ret1 = STM32_SMC_NOT_SUPPORTED;
		break;
	}
}
