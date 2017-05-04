/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <arch.h>
#include <arch_helpers.h>
#include <assert.h>
#include <debug.h>
#include <oem_svc.h>
#include <platform.h>
#include <runtime_svc.h>
#include <stdint.h>
#include <uuid.h>

/* OEM Service UUID */
DEFINE_SVC_UUID(oem_svc_uid,
		0xb943add0, 0x069d, 0x11e4, 0x91, 0x91,
		0x08, 0x00, 0x20, 0x0c, 0x9a, 0x66);


/* Setup OEM Services */
static int32_t oem_svc_setup(void)
{
	/*
	 * Invoke related module setup from here
	 */

	return 0;
}

/*******************************************************************************
 * OEM top level handler for servicing SMCs.
 ******************************************************************************/
uint64_t oem_smc_handler(uint32_t smc_fid,
			uint64_t x1,
			uint64_t x2,
			uint64_t x3,
			uint64_t x4,
			void *cookie,
			void *handle,
			uint64_t flags)
{
	uint64_t rc;

	switch (smc_fid) {
	default:
		rc = SMC_UNK;
		WARN("Unimplemented OEM Call: 0x%x\n", smc_fid);
	}

	SMC_RET1(handle, rc);
}

/*
 * Top-level OEM Service SMC handler. This handler will in turn dispatch
 * calls to related SMC handler
 */
uint64_t oem_svc_smc_handler(uint32_t smc_fid,
			 uint64_t x1,
			 uint64_t x2,
			 uint64_t x3,
			 uint64_t x4,
			 void *cookie,
			 void *handle,
			 uint64_t flags)
{
	/*
	 * Dispatch OEM calls to OEM Common handler and return its return value
	 */
	if (is_oem_fid(smc_fid)) {
		return oem_smc_handler(smc_fid, x1, x2, x3, x4, cookie,
					handle, flags);
	}

	switch (smc_fid) {
	case OEM_SVC_CALL_COUNT:
		/*
		 * Return the number of OEM Service Calls.
		 */
		SMC_RET1(handle, OEM_SVC_NUM_CALLS);

	case OEM_SVC_UID:
		/* Return UID to the caller */
		SMC_UUID_RET(handle, oem_svc_uid);

	case OEM_SVC_VERSION:
		/* Return the version of current implementation */
		SMC_RET2(handle, OEM_VERSION_MAJOR, OEM_VERSION_MINOR);

	default:
		WARN("Unimplemented OEM Service Call: 0x%x\n", smc_fid);
		SMC_RET1(handle, SMC_UNK);
	}
}

/* Register OEM Service Calls as runtime service */
DECLARE_RT_SVC(
		oem_svc,
		OEN_OEM_START,
		OEN_OEM_END,
		SMC_TYPE_FAST,
		oem_svc_setup,
		oem_svc_smc_handler
);
