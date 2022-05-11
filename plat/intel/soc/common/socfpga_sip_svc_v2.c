/*
 * Copyright (c) 2022, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <common/debug.h>
#include <common/runtime_svc.h>
#include <lib/mmio.h>

#include "socfpga_sip_svc.h"

uintptr_t sip_smc_handler_v2(uint32_t smc_fid,
				u_register_t x1,
				u_register_t x2,
				u_register_t x3,
				u_register_t x4,
				void *cookie,
				void *handle,
				u_register_t flags)
{
	uint32_t retval = 0;
	int status = INTEL_SIP_SMC_STATUS_OK;

	switch (smc_fid) {
	case INTEL_SIP_SMC_V2_GET_SVC_VERSION:
		SMC_RET4(handle, INTEL_SIP_SMC_STATUS_OK, x1,
				SIP_SVC_VERSION_MAJOR,
				SIP_SVC_VERSION_MINOR);

	case INTEL_SIP_SMC_V2_REG_READ:
		status = intel_secure_reg_read(x2, &retval);
		SMC_RET4(handle, status, x1, retval, x2);

	case INTEL_SIP_SMC_V2_REG_WRITE:
		status = intel_secure_reg_write(x2, (uint32_t)x3, &retval);
		SMC_RET4(handle, status, x1, retval, x2);

	case INTEL_SIP_SMC_V2_REG_UPDATE:
		status = intel_secure_reg_update(x2, (uint32_t)x3,
				(uint32_t)x4, &retval);
		SMC_RET4(handle, status, x1, retval, x2);

	case INTEL_SIP_SMC_V2_HPS_SET_BRIDGES:
		status = intel_hps_set_bridges(x2, x3);
		SMC_RET2(handle, status, x1);

	default:
		ERROR("%s: unhandled SMC V2 (0x%x)\n", __func__, smc_fid);
		SMC_RET1(handle, SMC_UNK);
	}
}
