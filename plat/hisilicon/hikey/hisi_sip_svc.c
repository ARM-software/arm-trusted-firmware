/*
 * Copyright (c) 2016-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <common/debug.h>
#include <common/runtime_svc.h>
#include <lib/pmf/pmf.h>
#include <tools_share/uuid.h>

#include <hisi_sip_svc.h>

/* Hisi SiP Service UUID */
DEFINE_SVC_UUID2(hisi_sip_svc_uid,
	0x74df99e5, 0x8276, 0xaa40, 0x9f, 0xf8,
	0xc0, 0x85, 0x52, 0xbc, 0x39, 0x3f);

static int hisi_sip_setup(void)
{
	if (pmf_setup() != 0)
		return 1;
	return 0;
}

/*
 * This function handles Hisi defined SiP Calls
 */
static uintptr_t hisi_sip_handler(unsigned int smc_fid,
			u_register_t x1,
			u_register_t x2,
			u_register_t x3,
			u_register_t x4,
			void *cookie,
			void *handle,
			u_register_t flags)
{
	int call_count = 0;

	/*
	 * Dispatch PMF calls to PMF SMC handler and return its return
	 * value
	 */
	if (is_pmf_fid(smc_fid)) {
		return pmf_smc_handler(smc_fid, x1, x2, x3, x4, cookie,
				handle, flags);
	}

	switch (smc_fid) {
	case HISI_SIP_SVC_CALL_COUNT:
		/* PMF calls */
		call_count += PMF_NUM_SMC_CALLS;

		/* State switch call */
		call_count += 1;

		SMC_RET1(handle, call_count);

	case HISI_SIP_SVC_UID:
		/* Return UID to the caller */
		SMC_UUID_RET(handle, hisi_sip_svc_uid);

	case HISI_SIP_SVC_VERSION:
		/* Return the version of current implementation */
		SMC_RET2(handle, HISI_SIP_SVC_VERSION_MAJOR, HISI_SIP_SVC_VERSION_MINOR);

	default:
		WARN("Unimplemented HISI SiP Service Call: 0x%x \n", smc_fid);
		SMC_RET1(handle, SMC_UNK);
	}

}


/* Define a runtime service descriptor for fast SMC calls */
DECLARE_RT_SVC(
	hisi_sip_svc,
	OEN_SIP_START,
	OEN_SIP_END,
	SMC_TYPE_FAST,
	hisi_sip_setup,
	hisi_sip_handler
);
