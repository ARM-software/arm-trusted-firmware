/*
 * Copyright (c) 2014-2024, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <stdint.h>

#include <common/debug.h>
#include <common/runtime_svc.h>
#include <drivers/scmi-msg.h>
#include <lib/psci/psci.h>
#include <tools_share/uuid.h>

#include <stm32mp1_smc.h>
#include <stm32mp_svc_setup.h>

#include "bsec_svc.h"

/*
 * Platform Standard Service SMC handler. This handler will dispatch
 * calls to features handlers.
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
	case STM32_SMC_BSEC:
		*ret1 = bsec_main(x1, x2, x3, ret2);
		*ret2_enabled = true;
		break;

	case STM32_SIP_SMC_SCMI_AGENT0:
		scmi_smt_fastcall_smc_entry(0);
		break;
	case STM32_SIP_SMC_SCMI_AGENT1:
		scmi_smt_fastcall_smc_entry(1);
		break;

	default:
		WARN("Unimplemented STM32MP1 Service Call: 0x%x\n", smc_fid);
		*ret1 = STM32_SMC_NOT_SUPPORTED;
		break;
	}
}
