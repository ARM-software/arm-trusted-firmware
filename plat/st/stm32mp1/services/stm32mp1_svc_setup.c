/*
 * Copyright (c) 2014-2021, STMicroelectronics - All Rights Reserved
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

#include "bsec_svc.h"

/* STM32 SiP Service UUID */
DEFINE_SVC_UUID2(stm32_sip_svc_uid,
		 0xa778aa50, 0xf49b, 0x144a, 0x8a, 0x5e,
		 0x26, 0x4d, 0x59, 0x94, 0xc2, 0x14);

/* Setup STM32MP1 Standard Services */
static int32_t stm32mp1_svc_setup(void)
{
	/*
	 * PSCI is the only specification implemented as a Standard Service.
	 * Invoke PSCI setup from here.
	 */
	return 0;
}

/*
 * Top-level Standard Service SMC handler. This handler will in turn dispatch
 * calls to PSCI SMC handler.
 */
static uintptr_t stm32mp1_svc_smc_handler(uint32_t smc_fid, u_register_t x1,
					  u_register_t x2, u_register_t x3,
					  u_register_t x4, void *cookie,
					  void *handle, u_register_t flags)
{
	uint32_t ret1 = 0U, ret2 = 0U;
	bool ret_uid = false, ret2_enabled = false;

	switch (smc_fid) {
	case STM32_SIP_SVC_CALL_COUNT:
		ret1 = STM32_COMMON_SIP_NUM_CALLS;
		break;

	case STM32_SIP_SVC_UID:
		/* Return UUID to the caller */
		ret_uid = true;
		break;

	case STM32_SIP_SVC_VERSION:
		/* Return the version of current implementation */
		ret1 = STM32_SIP_SVC_VERSION_MAJOR;
		ret2 = STM32_SIP_SVC_VERSION_MINOR;
		ret2_enabled = true;
		break;

	case STM32_SMC_BSEC:
		ret1 = bsec_main(x1, x2, x3, &ret2);
		ret2_enabled = true;
		break;

	case STM32_SIP_SMC_SCMI_AGENT0:
		scmi_smt_fastcall_smc_entry(0);
		break;
	case STM32_SIP_SMC_SCMI_AGENT1:
		scmi_smt_fastcall_smc_entry(1);
		break;

	default:
		WARN("Unimplemented STM32MP1 Service Call: 0x%x\n", smc_fid);
		ret1 = STM32_SMC_NOT_SUPPORTED;
		break;
	}

	if (ret_uid) {
		SMC_UUID_RET(handle, stm32_sip_svc_uid);
	}

	if (ret2_enabled) {
		SMC_RET2(handle, ret1, ret2);
	}

	SMC_RET1(handle, ret1);
}

/* Register Standard Service Calls as runtime service */
DECLARE_RT_SVC(stm32mp1_sip_svc,
	       OEN_SIP_START,
	       OEN_SIP_END,
	       SMC_TYPE_FAST,
	       stm32mp1_svc_setup,
	       stm32mp1_svc_smc_handler
);
