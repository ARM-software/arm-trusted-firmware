/*
 * Copyright (c) 2024-2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <common/debug.h>
#include <common/runtime_svc.h>
#include <lib/debugfs.h>
#include <lib/pmf/pmf.h>
#if PLAT_ARM_ACS_SMC_HANDLER
#include <plat/arm/common/plat_acs_smc_handler.h>
#endif /* PLAT_ARM_ACS_SMC_HANDLER */
#include <services/spm_mm_svc.h>
#include <services/ven_el3_svc.h>
#include <tools_share/uuid.h>

/* vendor-specific EL3 UUID */
DEFINE_SVC_UUID2(ven_el3_svc_uid,
	0xb6011dca, 0x57c4, 0x407e, 0x83, 0xf0,
	0xa7, 0xed, 0xda, 0xf0, 0xdf, 0x6c);

static int ven_el3_svc_setup(void)
{
#if USE_DEBUGFS
	if (debugfs_smc_setup() != 0) {
		return 1;
	}
#endif /* USE_DEBUGFS */

#if ENABLE_PMF
	if (pmf_setup() != 0) {
		return 1;
	}
#endif /* ENABLE_PMF */

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
#if USE_DEBUGFS
	/*
	 * Dispatch debugfs calls to debugfs SMC handler and return its
	 * return value.
	 */
	if (is_debugfs_fid(smc_fid)) {
		return debugfs_smc_handler(smc_fid, x1, x2, x3, x4, cookie,
			handle, flags);
	}
#endif /* USE_DEBUGFS */

#if ENABLE_PMF

	/*
	 * Dispatch PMF calls to PMF SMC handler and return its return
	 * value
	 */
	if (is_pmf_fid(smc_fid)) {
		return pmf_smc_handler(smc_fid, x1, x2, x3, x4, cookie,
				handle, flags);
	}

#endif /* ENABLE_PMF */

#if PLAT_ARM_ACS_SMC_HANDLER
	/*
	 * Dispatch ACS calls to ACS SMC handler and return its return value
	 */
	if (is_acs_fid(smc_fid)) {
		return plat_arm_acs_smc_handler(smc_fid, x1, x2, x3, x4, handle);
	}
#endif /* PLAT_ARM_ACS_SMC_HANDLER */

	switch (smc_fid) {
	case VEN_EL3_SVC_UID:
		/* Return UID to the caller */
		SMC_UUID_RET(handle, ven_el3_svc_uid);
		break;
	case VEN_EL3_SVC_VERSION:
		SMC_RET2(handle, VEN_EL3_SVC_VERSION_MAJOR, VEN_EL3_SVC_VERSION_MINOR);
		break;
#if SPM_MM
	/*
	 * Handle TPM start SMC as mentioned in TCG ACPI specification.
	 */
	case TPM_START_SMC_32:
	case TPM_START_SMC_64:
		return spm_mm_tpm_start_handler(smc_fid, x1, x2, x3, x4, cookie,
						handle, flags);
		break;
#endif
	default:
		WARN("Unimplemented vendor-specific EL3 Service call: 0x%x\n", smc_fid);
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
