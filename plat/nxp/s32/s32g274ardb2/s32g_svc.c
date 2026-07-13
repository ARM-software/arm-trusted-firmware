/*
 * Copyright 2026 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <common/debug.h>
#include <common/runtime_svc.h>
#include <drivers/scmi-msg.h>
#include <lib/smccc.h>
#include <smccc_helpers.h>

#include <s32g_scmi.h>

static int32_t s32g_svc_setup(void)
{
	return 0;
}

static uintptr_t s32g_svc_smc_handler(uint32_t smc_fid,
				      u_register_t x1,
				      u_register_t x2,
				      u_register_t x3,
				      u_register_t x4,
				      void *cookie,
				      void *handle,
				      u_register_t flags)
{
	if (smc_fid == S32G_SCMI_SMC_ID) {
		scmi_smt_fastcall_smc_entry(S32G_SCMI_AGENT_OSPM);
		SMC_RET1(handle, 0);
	}

	WARN("Unimplemented SiP Service Call: 0x%x\n", smc_fid);
	SMC_RET1(handle, SMC_UNK);
}

DECLARE_RT_SVC(s32g_svc,
	       OEN_SIP_START,
	       OEN_SIP_END,
	       SMC_TYPE_FAST,
	       s32g_svc_setup,
	       s32g_svc_smc_handler
);
