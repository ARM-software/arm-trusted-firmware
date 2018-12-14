/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <common/debug.h>
#include <common/runtime_svc.h>
#include <lib/mmio.h>
#include <tools_share/uuid.h>

#include <plat_sip_calls.h>
#include <rockchip_sip_svc.h>

/* Rockchip SiP Service UUID */
DEFINE_SVC_UUID2(rk_sip_svc_uid,
	0xe2c76fe8, 0x3e31, 0xe611, 0xb7, 0x0d,
	0x8f, 0x88, 0xee, 0x74, 0x7b, 0x72);

#pragma weak rockchip_plat_sip_handler
uintptr_t rockchip_plat_sip_handler(uint32_t smc_fid,
				    u_register_t x1,
				    u_register_t x2,
				    u_register_t x3,
				    u_register_t x4,
				    void *cookie,
				    void *handle,
				    u_register_t flags)
{
	ERROR("%s: unhandled SMC (0x%x)\n", __func__, smc_fid);
	SMC_RET1(handle, SMC_UNK);
}

/*
 * This function is responsible for handling all SiP calls from the NS world
 */
uintptr_t sip_smc_handler(uint32_t smc_fid,
			  u_register_t x1,
			  u_register_t x2,
			  u_register_t x3,
			  u_register_t x4,
			  void *cookie,
			  void *handle,
			  u_register_t flags)
{
	uint32_t ns;

	/* Determine which security state this SMC originated from */
	ns = is_caller_non_secure(flags);
	if (!ns)
		SMC_RET1(handle, SMC_UNK);

	switch (smc_fid) {
	case SIP_SVC_CALL_COUNT:
		/* Return the number of Rockchip SiP Service Calls. */
		SMC_RET1(handle,
			 RK_COMMON_SIP_NUM_CALLS + RK_PLAT_SIP_NUM_CALLS);

	case SIP_SVC_UID:
		/* Return UID to the caller */
		SMC_UUID_RET(handle, rk_sip_svc_uid);

	case SIP_SVC_VERSION:
		/* Return the version of current implementation */
		SMC_RET2(handle, RK_SIP_SVC_VERSION_MAJOR,
			RK_SIP_SVC_VERSION_MINOR);

	default:
		return rockchip_plat_sip_handler(smc_fid, x1, x2, x3, x4,
			cookie, handle, flags);
	}
}

/* Define a runtime service descriptor for fast SMC calls */
DECLARE_RT_SVC(
	rockchip_sip_svc,
	OEN_SIP_START,
	OEN_SIP_END,
	SMC_TYPE_FAST,
	NULL,
	sip_smc_handler
);
