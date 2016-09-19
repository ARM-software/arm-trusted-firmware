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

#include <arm_sip_svc.h>
#include <debug.h>
#include <pmf.h>
#include <runtime_svc.h>
#include <stdint.h>
#include <uuid.h>


/* ARM SiP Service UUID */
DEFINE_SVC_UUID(arm_sip_svc_uid,
		0xe2756d55, 0x3360, 0x4bb5, 0xbf, 0xf3,
		0x62, 0x79, 0xfd, 0x11, 0x37, 0xff);

static int arm_sip_setup(void)
{
	if (pmf_setup() != 0)
		return 1;
	return 0;
}

/*
 * This function handles ARM defined SiP Calls
 */
static uintptr_t arm_sip_handler(unsigned int smc_fid,
			u_register_t x1,
			u_register_t x2,
			u_register_t x3,
			u_register_t x4,
			void *cookie,
			void *handle,
			u_register_t flags)
{
	/*
	 * Dispatch PMF calls to PMF SMC handler and return its return
	 * value
	 */
	if (is_pmf_fid(smc_fid)) {
		return pmf_smc_handler(smc_fid, x1, x2, x3, x4, cookie,
				handle, flags);
	}

	switch (smc_fid) {
	case ARM_SIP_SVC_CALL_COUNT:
		/*
		 * Return the number of SiP Service Calls. PMF is the only
		 * SiP service implemented; so return number of PMF calls
		 */
		SMC_RET1(handle, PMF_NUM_SMC_CALLS);

	case ARM_SIP_SVC_UID:
		/* Return UID to the caller */
		SMC_UUID_RET(handle, arm_sip_svc_uid);

	case ARM_SIP_SVC_VERSION:
		/* Return the version of current implementation */
		SMC_RET2(handle, ARM_SIP_SVC_VERSION_MAJOR, ARM_SIP_SVC_VERSION_MINOR);

	default:
		WARN("Unimplemented ARM SiP Service Call: 0x%x \n", smc_fid);
		SMC_RET1(handle, SMC_UNK);
	}

}


/* Define a runtime service descriptor for fast SMC calls */
DECLARE_RT_SVC(
	arm_sip_svc,
	OEN_SIP_START,
	OEN_SIP_END,
	SMC_TYPE_FAST,
	arm_sip_setup,
	arm_sip_handler
);
