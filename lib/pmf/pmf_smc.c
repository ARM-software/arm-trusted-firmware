/*
 * Copyright (c) 2016-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <common/debug.h>
#include <lib/pmf/pmf.h>
#include <plat/common/platform.h>
#include <smccc_helpers.h>

/*
 * This function is responsible for handling all PMF SMC calls.
 */
uintptr_t pmf_smc_handler(unsigned int smc_fid,
			u_register_t x1,
			u_register_t x2,
			u_register_t x3,
			u_register_t x4,
			void *cookie,
			void *handle,
			u_register_t flags)
{
	int rc;
	unsigned long long ts_value;

	if (((smc_fid >> FUNCID_CC_SHIFT) & FUNCID_CC_MASK) == SMC_32) {

		x1 = (uint32_t)x1;
		x2 = (uint32_t)x2;
		x3 = (uint32_t)x3;

		if (smc_fid == PMF_SMC_GET_TIMESTAMP_32) {
			/*
			 * Return error code and the captured
			 * time-stamp to the caller.
			 * x0 --> error code.
			 * x1 - x2 --> time-stamp value.
			 */
			rc = pmf_get_timestamp_smc((unsigned int)x1, x2,
					(unsigned int)x3, &ts_value);
			SMC_RET3(handle, rc, (uint32_t)ts_value,
					(uint32_t)(ts_value >> 32));
		}
	} else {
		if (smc_fid == PMF_SMC_GET_TIMESTAMP_64) {
			/*
			 * Return error code and the captured
			 * time-stamp to the caller.
			 * x0 --> error code.
			 * x1 --> time-stamp value.
			 */
			rc = pmf_get_timestamp_smc((unsigned int)x1, x2,
					(unsigned int)x3, &ts_value);
			SMC_RET2(handle, rc, ts_value);
		}
	}

	WARN("Unimplemented PMF Call: 0x%x \n", smc_fid);
	SMC_RET1(handle, SMC_UNK);
}
