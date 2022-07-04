/*
 * Copyright (c) 2021-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


#include <common/debug.h>
#include <plat/common/platform.h>
#include <services/rmm_core_manifest.h>
#include <services/rmmd_svc.h>
#include <services/trp/platform_trp.h>
#include <trp_helpers.h>
#include "trp_private.h"

#include <platform_def.h>

/* Parameters received from the previous image */
static unsigned int trp_boot_abi_version;
static uintptr_t trp_shared_region_start;

/* Parameters received from boot manifest */
uint32_t trp_boot_manifest_version;

/*******************************************************************************
 * Setup function for TRP.
 ******************************************************************************/
void trp_setup(uint64_t x0,
	       uint64_t x1,
	       uint64_t x2,
	       uint64_t x3)
{
	/*
	 * Validate boot parameters.
	 *
	 * According to the Boot Interface ABI v.0.1, the
	 * parameters recived from EL3 are:
	 * x0: CPUID (verified earlier so not used)
	 * x1: Boot Interface version
	 * x2: PLATFORM_CORE_COUNT
	 * x3: Pointer to the shared memory area.
	 */

	(void)x0;

	if (TRP_RMM_EL3_VERSION_GET_MAJOR(x1) != TRP_RMM_EL3_ABI_VERS_MAJOR) {
		trp_boot_abort(E_RMM_BOOT_VERSION_MISMATCH);
	}

	if ((void *)x3 == NULL) {
		trp_boot_abort(E_RMM_BOOT_INVALID_SHARED_BUFFER);
	}

	if (x2 > TRP_PLATFORM_CORE_COUNT) {
		trp_boot_abort(E_RMM_BOOT_CPUS_OUT_OF_RANGE);
	}

	trp_boot_abi_version = x1;
	trp_shared_region_start = x3;
	flush_dcache_range((uintptr_t)&trp_boot_abi_version,
			   sizeof(trp_boot_abi_version));
	flush_dcache_range((uintptr_t)&trp_shared_region_start,
			   sizeof(trp_shared_region_start));

	/* Perform early platform-specific setup */
	trp_early_platform_setup((rmm_manifest_t *)trp_shared_region_start);
}

/* Main function for TRP */
void trp_main(void)
{
	NOTICE("TRP: %s\n", version_string);
	NOTICE("TRP: %s\n", build_message);
	NOTICE("TRP: Supported RMM-EL3 Interface ABI: v.%u.%u\n",
		TRP_RMM_EL3_ABI_VERS_MAJOR, TRP_RMM_EL3_ABI_VERS_MINOR);
	NOTICE("TRP: Boot Manifest Version : v.%u.%u\n",
		RMMD_GET_MANIFEST_VERSION_MAJOR(trp_boot_manifest_version),
		RMMD_GET_MANIFEST_VERSION_MINOR(trp_boot_manifest_version));
	INFO("TRP: Memory base : 0x%lx\n", (unsigned long)RMM_BASE);
	INFO("TRP: Base address for the shared region : 0x%lx\n",
			(unsigned long)trp_shared_region_start);
	INFO("TRP: Total size : 0x%lx bytes\n", (unsigned long)(RMM_END
								- RMM_BASE));
	INFO("TRP: RMM-EL3 Interface ABI reported by EL3: v.%u.%u\n",
		TRP_RMM_EL3_VERSION_GET_MAJOR(trp_boot_abi_version),
		TRP_RMM_EL3_VERSION_GET_MINOR(trp_boot_abi_version));
}

/*******************************************************************************
 * Returning RMI version back to Normal World
 ******************************************************************************/
static trp_args_t *trp_ret_rmi_version(void)
{
	VERBOSE("RMM version is %u.%u\n", RMI_ABI_VERSION_MAJOR,
					  RMI_ABI_VERSION_MINOR);
	return set_smc_args(RMM_RMI_REQ_COMPLETE, RMI_ABI_VERSION,
			    0, 0, 0, 0, 0, 0);
}

/*******************************************************************************
 * Transitioning granule of NON-SECURE type to REALM type
 ******************************************************************************/
static trp_args_t *trp_asc_mark_realm(unsigned long long x1)
{
	unsigned long long ret;

	VERBOSE("Delegating granule 0x%llx\n", x1);
	ret = trp_smc(set_smc_args(RMM_GTSI_DELEGATE, x1, 0, 0, 0, 0, 0, 0));

	if (ret != 0ULL) {
		ERROR("Granule transition from NON-SECURE type to REALM type "
			"failed 0x%llx\n", ret);
	}
	return set_smc_args(RMM_RMI_REQ_COMPLETE, ret, 0, 0, 0, 0, 0, 0);
}

/*******************************************************************************
 * Transitioning granule of REALM type to NON-SECURE type
 ******************************************************************************/
static trp_args_t *trp_asc_mark_nonsecure(unsigned long long x1)
{
	unsigned long long ret;

	VERBOSE("Undelegating granule 0x%llx\n", x1);
	ret = trp_smc(set_smc_args(RMM_GTSI_UNDELEGATE, x1, 0, 0, 0, 0, 0, 0));

	if (ret != 0ULL) {
		ERROR("Granule transition from REALM type to NON-SECURE type "
			"failed 0x%llx\n", ret);
	}
	return set_smc_args(RMM_RMI_REQ_COMPLETE, ret, 0, 0, 0, 0, 0, 0);
}

/*******************************************************************************
 * Main RMI SMC handler function
 ******************************************************************************/
trp_args_t *trp_rmi_handler(unsigned long fid, unsigned long long x1)
{
	switch (fid) {
	case RMI_RMM_REQ_VERSION:
		return trp_ret_rmi_version();
	case RMI_RMM_GRANULE_DELEGATE:
		return trp_asc_mark_realm(x1);
	case RMI_RMM_GRANULE_UNDELEGATE:
		return trp_asc_mark_nonsecure(x1);
	default:
		ERROR("Invalid SMC code to %s, FID %lu\n", __func__, fid);
	}
	return set_smc_args(SMC_UNK, 0, 0, 0, 0, 0, 0, 0);
}
