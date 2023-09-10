/*
 * Copyright (c) 2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <common/debug.h>
#include <common/runtime_svc.h>

#include <plat/arm/common/arm_sip_svc.h>
#include <plat/common/platform.h>

#if ENABLE_SPMD_LP
#include <services/el3_spmd_logical_sp.h>
#endif

uintptr_t plat_arm_sip_handler(uint32_t smc_fid,
				u_register_t x1,
				u_register_t x2,
				u_register_t x3,
				u_register_t x4,
				void *cookie,
				void *handle,
				u_register_t flags)
{
#if ENABLE_SPMD_LP
	return plat_spmd_logical_sp_smc_handler(smc_fid, x1, x2, x3, x4,
				cookie, handle, flags);
#else
	WARN("Unimplemented ARM SiP Service Call: 0x%x\n", smc_fid);
	SMC_RET1(handle, SMC_UNK);
#endif
}
