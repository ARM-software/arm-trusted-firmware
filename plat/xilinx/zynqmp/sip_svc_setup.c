/*
 * Copyright (c) 2013-2020, Arm Limited and Contributors. All rights reserved.
 * Copyright (c) 2023, Advanced Micro Devices, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Top level SMC handler for SiP calls. Dispatch PM calls to PM SMC handler. */

#include <inttypes.h>

#include <common/runtime_svc.h>
#include <tools_share/uuid.h>

#include <custom_svc.h>
#include "ipi_mailbox_svc.h"
#include "pm_defs.h"
#include "zynqmp_pm_svc_main.h"

/* SMC function IDs for SiP Service queries */
#define ZYNQMP_SIP_SVC_CALL_COUNT	U(0x8200ff00)
#define ZYNQMP_SIP_SVC_UID		U(0x8200ff01)
#define ZYNQMP_SIP_SVC_VERSION		U(0x8200ff03)

/* SiP Service Calls version numbers */
#define SIP_SVC_VERSION_MAJOR	0
#define SIP_SVC_VERSION_MINOR	1

/* These macros are used to identify PM, IPI calls from the SMC function ID */
#define SIP_FID_MASK	GENMASK(23, 16)
#define XLNX_FID_MASK	GENMASK(23, 12)
#define PM_FID_VALUE	0u
#define IPI_FID_VALUE	0x1000u
#define is_pm_fid(_fid) (((_fid) & XLNX_FID_MASK) == PM_FID_VALUE)
#define is_ipi_fid(_fid) (((_fid) & XLNX_FID_MASK) == IPI_FID_VALUE)

/* SiP Service UUID */
DEFINE_SVC_UUID2(zynqmp_sip_uuid,
	0x5c9b1b2a, 0x0586, 0x2340, 0xa6, 0x1b,
	0xb9, 0x25, 0x82, 0x2d, 0xe3, 0xa5);

/**
 * sip_svc_setup() - Setup SiP Service.
 *
 * Return: On success, the initialization function must return 0.
 *         Any other return value will cause the framework to ignore
 *         the service.
 *
 * Invokes PM setup.
 */
static int32_t sip_svc_setup(void)
{
	/* PM implementation as SiP Service */
	return pm_setup();
}

/**
 * sip_svc_smc_handler() - Top-level SiP Service SMC handler
 * @smc_fid: Function Identifier.
 * @x1: SMC64 Arguments 1 from kernel.
 * @x2: SMC64 Arguments 2 from kernel.
 * @x3: SMC64 Arguments 3 from kernel(upper 32-bits).
 * @x4: SMC64 Arguments 4 from kernel.
 * @cookie: Unused
 * @handle: Pointer to caller's context structure.
 * @flags: SECURE_FLAG or NON_SECURE_FLAG.
 *
 * Handler for all SiP SMC calls. Handles standard SIP requests
 * and calls PM SMC handler if the call is for a PM-API function.
 *
 * Return: Unused.
 */
static uintptr_t sip_svc_smc_handler(uint32_t smc_fid,
			      u_register_t x1,
			      u_register_t x2,
			      u_register_t x3,
			      u_register_t x4,
			      void *cookie,
			      void *handle,
			      u_register_t flags)
{
	VERBOSE("SMCID: 0x%08x, x1: 0x%016" PRIx64 ", x2: 0x%016" PRIx64 ", x3: 0x%016" PRIx64 ", x4: 0x%016" PRIx64 "\n",
		smc_fid, x1, x2, x3, x4);

	if (smc_fid & SIP_FID_MASK) {
		WARN("SMC out of SiP assinged range: 0x%x\n", smc_fid);
		SMC_RET1(handle, SMC_UNK);
	}

	/* Let PM SMC handler deal with PM-related requests */
	if (is_pm_fid(smc_fid)) {
		return pm_smc_handler(smc_fid, x1, x2, x3, x4, cookie, handle,
				      flags);
	}

	/* Let IPI SMC handler deal with IPI-related requests */
	if (is_ipi_fid(smc_fid)) {
		return ipi_smc_handler(smc_fid, x1, x2, x3, x4, cookie, handle,
				      flags);
	}

	switch (smc_fid) {
	case ZYNQMP_SIP_SVC_CALL_COUNT:
		/* PM functions + default functions */
		SMC_RET1(handle, PM_API_MAX + 2);

	case ZYNQMP_SIP_SVC_UID:
		SMC_UUID_RET(handle, zynqmp_sip_uuid);

	case ZYNQMP_SIP_SVC_VERSION:
		SMC_RET2(handle, SIP_SVC_VERSION_MAJOR, SIP_SVC_VERSION_MINOR);

	case ZYNQMP_SIP_SVC_CUSTOM:
	case ZYNQMP_SIP_SVC64_CUSTOM:
		return custom_smc_handler(smc_fid, x1, x2, x3, x4, cookie,
					  handle, flags);

	default:
		WARN("Unimplemented SiP Service Call: 0x%x\n", smc_fid);
		SMC_RET1(handle, SMC_UNK);
	}
}

/* Register PM Service Calls as runtime service */
DECLARE_RT_SVC(
		sip_svc,
		OEN_SIP_START,
		OEN_SIP_END,
		(uint8_t)SMC_TYPE_FAST,
		sip_svc_setup,
		sip_svc_smc_handler);
