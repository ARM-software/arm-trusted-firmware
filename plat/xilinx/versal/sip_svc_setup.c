/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Top level SMC handler for SiP calls. Dispatch PM calls to PM SMC handler. */

#include <common/debug.h>
#include <common/runtime_svc.h>
#include <tools_share/uuid.h>

/* SMC function IDs for SiP Service queries */
#define VERSAL_SIP_SVC_CALL_COUNT	0x8200ff00
#define VERSAL_SIP_SVC_UID		0x8200ff01
#define VERSAL_SIP_SVC_VERSION		0x8200ff03

/* SiP Service Calls version numbers */
#define SIP_SVC_VERSION_MAJOR	0
#define SIP_SVC_VERSION_MINOR	1

/* These macros are used to identify PM calls from the SMC function ID */
#define PM_FID_MASK	0xf000u
#define PM_FID_VALUE	0u
#define is_pm_fid(_fid) (((_fid) & PM_FID_MASK) == PM_FID_VALUE)

/* SiP Service UUID */
DEFINE_SVC_UUID2(versal_sip_uuid,
		0x2ab9e4ec, 0x93b9, 0x11e7, 0xa0, 0x19,
		0xdf, 0xe0, 0xdb, 0xad, 0x0a, 0xe0);

/**
 * sip_svc_setup() - Setup SiP Service
 *
 * Invokes PM setup
 */
static int32_t sip_svc_setup(void)
{
	return 0;
}

/**
 * sip_svc_smc_handler() - Top-level SiP Service SMC handler
 *
 * Handler for all SiP SMC calls. Handles standard SIP requests
 * and calls PM SMC handler if the call is for a PM-API function.
 */
uintptr_t sip_svc_smc_handler(uint32_t smc_fid,
			     u_register_t x1,
			     u_register_t x2,
			     u_register_t x3,
			     u_register_t x4,
			     void *cookie,
			     void *handle,
			     u_register_t flags)
{
	/* Let PM SMC handler deal with PM-related requests */
	switch (smc_fid) {
	case VERSAL_SIP_SVC_CALL_COUNT:
		/* PM functions + default functions */
		SMC_RET1(handle, 2);

	case VERSAL_SIP_SVC_UID:
		SMC_UUID_RET(handle, versal_sip_uuid);

	case VERSAL_SIP_SVC_VERSION:
		SMC_RET2(handle, SIP_SVC_VERSION_MAJOR, SIP_SVC_VERSION_MINOR);

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
		SMC_TYPE_FAST,
		sip_svc_setup,
		sip_svc_smc_handler);
