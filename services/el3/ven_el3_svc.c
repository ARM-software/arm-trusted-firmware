/*
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <common/debug.h>
#include <common/runtime_svc.h>
#include <services/ven_el3_svc.h>
#include <tools_share/uuid.h>

/* vendor-specific EL3 UUID */
DEFINE_SVC_UUID2(ven_el3_svc_uid,
	0xb6011dca, 0x57c4, 0x407e, 0x83, 0xf0,
	0xa7, 0xed, 0xda, 0xf0, 0xdf, 0x6c);

static int ven_el3_svc_setup(void)
{
	return 0;
}

/*
 * This function handles Arm defined vendor-specific EL3 Service Calls.
 */
static uintptr_t ven_el3_svc_handler(unsigned int smc_fid,
			u_register_t x1,
			u_register_t x2,
			u_register_t x3,
			u_register_t x4,
			void *cookie,
			void *handle,
			u_register_t flags)
{
	switch (smc_fid) {
	case VEN_EL3_SVC_UID:
		/* Return UID to the caller */
		SMC_UUID_RET(handle, ven_el3_svc_uid);
		break;
	case VEN_EL3_SVC_VERSION:
		SMC_RET2(handle, VEN_EL3_SVC_VERSION_MAJOR, VEN_EL3_SVC_VERSION_MINOR);
		break;
	default:
		WARN("Unimplemented Vendor specific EL3 Service call: 0x%x\n", smc_fid);
		SMC_RET1(handle, SMC_UNK);
		break;
	}
}

/* Define a runtime service descriptor for fast SMC calls */
DECLARE_RT_SVC(
	ven_el3_svc,
	OEN_VEN_EL3_START,
	OEN_VEN_EL3_END,
	SMC_TYPE_FAST,
	ven_el3_svc_setup,
	ven_el3_svc_handler
);
