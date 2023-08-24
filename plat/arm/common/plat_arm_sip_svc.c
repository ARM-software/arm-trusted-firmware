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
#if PLAT_TEST_SPM
	bool secure_origin;

	/* Determine which security state this SMC originated from */
	secure_origin = is_caller_secure(flags);

	switch (smc_fid) {
	case ARM_SIP_SET_INTERRUPT_PENDING:
		if (!secure_origin) {
			SMC_RET1(handle, SMC_UNK);
		}

		VERBOSE("SiP Call- Set interrupt pending %d\n", (uint32_t)x1);
		plat_ic_set_interrupt_pending(x1);

		SMC_RET1(handle, SMC_OK);
		break; /* Not reached */
	default:
		break;
	}
#endif

#if ENABLE_SPMD_LP
	return plat_spmd_logical_sp_smc_handler(smc_fid, x1, x2, x3, x4,
				cookie, handle, flags);
#else
	WARN("Unimplemented ARM SiP Service Call: 0x%x\n", smc_fid);
	SMC_RET1(handle, SMC_UNK);
#endif
}
