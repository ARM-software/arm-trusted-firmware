/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <assert.h>
#include <debug.h>
#include <platform.h>
#include <pmf.h>
#include <smcc_helpers.h>

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

		switch (smc_fid) {
		case PMF_SMC_GET_TIMESTAMP_32:
			/*
			 * Return error code and the captured
			 * time-stamp to the caller.
			 * x0 --> error code.
			 * x1 - x2 --> time-stamp value.
			 */
			rc = pmf_get_timestamp_smc(x1, x2, x3, &ts_value);
			SMC_RET3(handle, rc, (uint32_t)ts_value,
					(uint32_t)(ts_value >> 32));

		default:
			break;
		}
	} else {
		switch (smc_fid) {
		case PMF_SMC_GET_TIMESTAMP_64:
			/*
			 * Return error code and the captured
			 * time-stamp to the caller.
			 * x0 --> error code.
			 * x1 --> time-stamp value.
			 */
			rc = pmf_get_timestamp_smc(x1, x2, x3, &ts_value);
			SMC_RET2(handle, rc, ts_value);

		default:
			break;
		}
	}

	WARN("Unimplemented PMF Call: 0x%x \n", smc_fid);
	SMC_RET1(handle, SMC_UNK);
}
