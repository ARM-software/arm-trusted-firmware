/*
 * Copyright (c) 2016-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <common/debug.h>
#include <common/runtime_svc.h>
#include <drivers/arm/ethosn.h>
#include <lib/debugfs.h>
#include <lib/pmf/pmf.h>
#include <plat/arm/common/arm_sip_svc.h>
#include <plat/arm/common/plat_arm.h>
#include <tools_share/uuid.h>

/* ARM SiP Service UUID */
DEFINE_SVC_UUID2(arm_sip_svc_uid,
	0x556d75e2, 0x6033, 0xb54b, 0xb5, 0x75,
	0x62, 0x79, 0xfd, 0x11, 0x37, 0xff);

static int arm_sip_setup(void)
{
#if ENABLE_PMF
	if (pmf_setup() != 0) {
		return 1;
	}
#endif /* ENABLE_PMF */

#if USE_DEBUGFS

	if (debugfs_smc_setup() != 0) {
		return 1;
	}

#endif /* USE_DEBUGFS */

#if ETHOSN_NPU_DRIVER

	if (ethosn_smc_setup() != 0) {
		return 1;
	}

#endif /* ETHOSN_NPU_DRIVER */

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
	int call_count = 0;

#if ENABLE_PMF
	/*
	 * Dispatch PMF calls to PMF SMC handler and return its return
	 * value
	 */
	if (is_pmf_fid_deprecated(smc_fid)) {
		NOTICE("PMF Interface usage from arm-sip range is deprecated. \
			Please migrate smc call to Vendor-specific el3 range.\n");
		return pmf_smc_handler(smc_fid, x1, x2, x3, x4, cookie,
				handle, flags);
	}

#endif /* ENABLE_PMF */

#if USE_DEBUGFS
	if (is_debugfs_fid_deprecated(smc_fid)) {
		NOTICE("Debugfs Interface usage from arm-sip range is deprecated. \
			Please migrate smc call to vendor-specific el3 range.\n");
		return debugfs_smc_handler(smc_fid, x1, x2, x3, x4, cookie,
					   handle, flags);
	}

#endif /* USE_DEBUGFS */

#if ETHOSN_NPU_DRIVER

	if (is_ethosn_fid(smc_fid)) {
		return ethosn_smc_handler(smc_fid, x1, x2, x3, x4, cookie,
					  handle, flags);
	}

#endif /* ETHOSN_NPU_DRIVER */

	switch (smc_fid) {
	case ARM_SIP_SVC_EXE_STATE_SWITCH: {
		/* Execution state can be switched only if EL3 is AArch64 */
#ifdef __aarch64__
		/* Allow calls from non-secure only */
		if (!is_caller_non_secure(flags))
			SMC_RET1(handle, STATE_SW_E_DENIED);

		/*
		 * Pointers used in execution state switch are all 32 bits wide
		 */
		return (uintptr_t) arm_execution_state_switch(smc_fid,
				(uint32_t) x1, (uint32_t) x2, (uint32_t) x3,
				(uint32_t) x4, handle);
#else
		/* State switch denied */
		SMC_RET1(handle, STATE_SW_E_DENIED);
#endif /* __aarch64__ */
		}

	case ARM_SIP_SVC_CALL_COUNT:
		/* PMF calls */
		call_count += PMF_NUM_SMC_CALLS;

#if ETHOSN_NPU_DRIVER
		/* ETHOSN calls */
		call_count += ETHOSN_NUM_SMC_CALLS;
#endif          /* ETHOSN_NPU_DRIVER */

		/* State switch call */
		call_count += 1;

		SMC_RET1(handle, call_count);

	case ARM_SIP_SVC_UID:
		/* Return UID to the caller */
		SMC_UUID_RET(handle, arm_sip_svc_uid);

	case ARM_SIP_SVC_VERSION:
		/* Return the version of current implementation */
		SMC_RET2(handle, ARM_SIP_SVC_VERSION_MAJOR, ARM_SIP_SVC_VERSION_MINOR);

	default:
		break;
	}

	/*
	 * Fall back to allow Arm platform specific handler.
	 * TODO: Refactor needed to move out generic handlers from this file and
	 * only keep Arm Platform specific handlers here.
	 */
	return plat_arm_sip_handler(smc_fid, x1, x2, x3, x4,
					cookie, handle, flags);
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
