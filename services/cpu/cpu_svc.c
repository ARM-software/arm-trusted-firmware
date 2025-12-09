/*
 * Copyright (c) 2026, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <common/debug.h>
#include <common/runtime_svc.h>
#include <services/cpu_svc.h>
#include <tools_share/uuid.h>

/* cpu UUID */
DEFINE_SVC_UUID2(cpu_svc_uid,
	0xc3f2c4d1, 0x4e3a, 0x4b8f, 0x9c, 0x62,
	0x6c, 0x7a, 0x1c, 0xdb, 0xcb, 0x91);

static int cpu_svc_setup(void)
{
	return 0;
}

/*
 * This function handles Arm defined cpu Service Calls.
 */
static uintptr_t cpu_svc_handler(unsigned int smc_fid,
			u_register_t x1,
			u_register_t x2,
			u_register_t x3,
			u_register_t x4,
			void *cookie,
			void *handle,
			u_register_t flags)
{

	switch (smc_fid) {
	case CPU_SVC_UID:
		/* Return UID to the caller */
		SMC_UUID_RET(handle, cpu_svc_uid);
		break;
	case CPU_SVC_VERSION:
		SMC_RET2(handle, CPU_SVC_VERSION_MAJOR, CPU_SVC_VERSION_MINOR);
		break;
	default:
		WARN("Unimplemented CPU Service call: 0x%x\n", smc_fid);
		SMC_RET1(handle, SMC_UNK);
		break;
	}
}

/* Define a runtime service descriptor for fast SMC calls */
DECLARE_RT_SVC(
	cpu_svc,
	OEN_CPU_START,
	OEN_CPU_END,
	SMC_TYPE_FAST,
	cpu_svc_setup,
	cpu_svc_handler
);
