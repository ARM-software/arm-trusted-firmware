/*
 * Copyright (c) 2021-2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/build_message.h>
#include <common/debug.h>
#include <plat/common/platform.h>
#include <services/rmm_core_manifest.h>
#include <services/rmmd_svc.h>
#include <services/trp/platform_trp.h>
#include <trp_helpers.h>
#include "trp_private.h"

#include <platform_def.h>

#define RMI_ERROR_REALM			2U
#define RMI_ERROR_NOT_SUPPORTED		6U

#define DIR_BIT_SHIFT			0x8
#define KEYSET_SHIFT			0xC
#define	STREAM_ID_MASK			0xFF
#define STREAM_ID_SHIFT			0x0
#define SUBSTREAM_MASK			0x7
#define SUBSTREAM_SHIFT			0x8

#define KEY_SET				0x0
#define	DIR_VAL				0x0
#define	SUBSTREAM_VAL			0x1
#define	STREAM_ID			0x1

#define ENCODE_STREAM_INFO(key, dir, substream, stream_id)			\
		(((key & 0x1) << KEYSET_SHIFT) |				\
		((dir & 0x1) << DIR_BIT_SHIFT) |				\
		((substream && SUBSTREAM_MASK) << SUBSTREAM_SHIFT) |		\
		((stream_id && STREAM_ID_MASK) << STREAM_ID_SHIFT))

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
	 * Validate boot parameters
	 *
	 * According to the Boot Interface ABI v.0.1,
	 * the parameters received from EL3 are:
	 * x0: CPUID (verified earlier, so not used)
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
	trp_early_platform_setup((struct rmm_manifest *)trp_shared_region_start);
}

int trp_validate_warmboot_args(uint64_t x0, uint64_t x1,
			       uint64_t x2, uint64_t x3)
{
	/*
	 * Validate boot parameters for warm boot
	 *
	 * According to the Boot Interface ABI v.0.1, the parameters
	 * received from EL3 during warm boot are:
	 *
	 * x0: CPUID (verified earlier so not used here)
	 * x1: activation token (ignored)
	 * [x2:x3]: RES0
	 */

	(void)x0;
	(void)x1;

	if ((x2 | x3) != 0UL) {
		ERROR("TRP: extra warmboot arguments not 0: x2=0x%lx, x3=0x%lx\n",
		     x2, x3);
		return E_RMM_BOOT_UNKNOWN;
	}

	return 0;
}

/* Main function for TRP */
void trp_main(void)
{
	NOTICE("TRP: %s\n", build_version_string);
	NOTICE("TRP: %s\n", build_message);
	NOTICE("TRP: Supported RMM-EL3 Interface ABI: v.%u.%u\n",
		TRP_RMM_EL3_ABI_VERS_MAJOR, TRP_RMM_EL3_ABI_VERS_MINOR);
	NOTICE("TRP: Boot Manifest Version: v.%u.%u\n",
		RMMD_GET_MANIFEST_VERSION_MAJOR(trp_boot_manifest_version),
		RMMD_GET_MANIFEST_VERSION_MINOR(trp_boot_manifest_version));
	INFO("TRP: Memory base: 0x%lx\n", (unsigned long)RMM_BASE);
	INFO("TRP: Shared region base address: 0x%lx\n",
			(unsigned long)trp_shared_region_start);
	INFO("TRP: Total size: 0x%lx bytes\n",
			(unsigned long)(RMM_END - RMM_BASE));
	INFO("TRP: RMM-EL3 Interface ABI reported by EL3: v.%u.%u\n",
		TRP_RMM_EL3_VERSION_GET_MAJOR(trp_boot_abi_version),
		TRP_RMM_EL3_VERSION_GET_MINOR(trp_boot_abi_version));
}

/*******************************************************************************
 * Returning RMI version back to Normal World
 ******************************************************************************/
static void trp_ret_rmi_version(unsigned long long rmi_version,
				struct trp_smc_result *smc_ret)
{
	if (rmi_version != RMI_ABI_VERSION) {
		smc_ret->x[0] = RMI_ERROR_INPUT;
	} else {
		smc_ret->x[0] = RMI_SUCCESS;
	}
	VERBOSE("RMM version is %u.%u\n", RMI_ABI_VERSION_MAJOR,
					  RMI_ABI_VERSION_MINOR);
	smc_ret->x[1] = RMI_ABI_VERSION;
	smc_ret->x[2] = RMI_ABI_VERSION;
}

/*******************************************************************************
 * Transitioning granule of NON-SECURE type to REALM type
 ******************************************************************************/
static void trp_asc_mark_realm(unsigned long long x1,
				struct trp_smc_result *smc_ret)
{
	VERBOSE("Delegating granule 0x%llx\n", x1);
	smc_ret->x[0] = trp_smc(set_smc_args(RMM_GTSI_DELEGATE, x1,
				0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL));

	if (smc_ret->x[0] != 0ULL) {
		ERROR("Granule transition from NON-SECURE type to REALM type "
			"failed 0x%llx\n", smc_ret->x[0]);
	}
}

/*******************************************************************************
 * Transitioning granule of REALM type to NON-SECURE type
 ******************************************************************************/
static void trp_asc_mark_nonsecure(unsigned long long x1,
				   struct trp_smc_result *smc_ret)
{
	VERBOSE("Undelegating granule 0x%llx\n", x1);
	smc_ret->x[0] = trp_smc(set_smc_args(RMM_GTSI_UNDELEGATE, x1,
				0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL));

	if (smc_ret->x[0] != 0ULL) {
		ERROR("Granule transition from REALM type to NON-SECURE type "
			"failed 0x%llx\n", smc_ret->x[0]);
	}
}

/*******************************************************************************
 * Test the IDE Key management interface
 ******************************************************************************/
static void trp_ide_keymgmt_interface_fn(unsigned long long x1, unsigned long long x2,
					struct trp_smc_result *smc_ret)
{
	uint64_t ecam_address = 0U, rp_id = 0U, ide_stream_info;
	uint64_t keyqw0, keyqw1, keyqw2, keyqw3;
	uint64_t ifvqw0, ifvqw1;
	int return_value;

#if RMMD_ENABLE_IDE_KEY_PROG
	trp_get_test_rootport(&ecam_address, &rp_id);
#endif /* RMMD_ENABLE_IDE_KEY_PROG */
	/*
	 * Dummy values for testing:
	 * Key set = 0x0
	 * Dir = 0x0
	 * Substream = 0x1
	 * Stream ID = 0x1
	 */
	ide_stream_info  = ENCODE_STREAM_INFO(KEY_SET, DIR_VAL, SUBSTREAM_VAL, STREAM_ID);

	/* Dummy key and IV values for testing */
	keyqw0 = 0xA1B2C3D4E5F60708;
	keyqw1 = 0x1122334455667788;
	keyqw2 = 0xDEADBEEFCAFEBABE;
	keyqw3 = 0x1234567890ABCDEF;
	ifvqw0 = 0xABCDEF0123456789;
	ifvqw1 = 0x9876543210FEDCBA;

	return_value = trp_smc(set_smc_args(RMM_IDE_KEY_PROG, ecam_address, rp_id,
				ide_stream_info, keyqw0, keyqw1, keyqw2, keyqw3, ifvqw0,
				ifvqw1, 0UL, 0UL));

	INFO("return value from RMM_IDE_KEY_PROG = %d\n", return_value);

	return_value = trp_smc(set_smc_args(RMM_IDE_KEY_SET_GO, ecam_address, rp_id,
				ide_stream_info, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL));

	INFO("return value from RMM_IDE_KEY_SET_GO = %d\n", return_value);

	return_value = trp_smc(set_smc_args(RMM_IDE_KEY_SET_STOP, ecam_address, rp_id,
				ide_stream_info, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL));

	INFO("return value from RMM_IDE_KEY_SET_STOP = %d\n", return_value);

	return_value = trp_smc(set_smc_args(RMM_IDE_KM_PULL_RESPONSE, ecam_address, rp_id,
				0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL));

	INFO("return value from RMM_IDE_KEY_SET_STOP = %d\n", return_value);

	smc_ret->x[0] = RMI_ERROR_NOT_SUPPORTED;

}

/*******************************************************************************
 * Main RMI SMC handler function
 ******************************************************************************/
void trp_rmi_handler(unsigned long fid,
		     unsigned long long x1, unsigned long long x2,
		     unsigned long long x3, unsigned long long x4,
		     unsigned long long x5, unsigned long long x6,
		     struct trp_smc_result *smc_ret)
{
	/* Not used in the current implementation */
	(void)x2;
	(void)x3;
	(void)x4;
	(void)x5;
	(void)x6;

	switch (fid) {
	case RMI_RMM_REQ_VERSION:
		trp_ret_rmi_version(x1, smc_ret);
		break;
	case RMI_RMM_GRANULE_DELEGATE:
		trp_asc_mark_realm(x1, smc_ret);
		break;
	case RMI_RMM_GRANULE_UNDELEGATE:
		trp_asc_mark_nonsecure(x1, smc_ret);
		break;
	case RMI_RMM_PDEV_CREATE:
		trp_ide_keymgmt_interface_fn(x1, x2, smc_ret);
		break;
	default:
		ERROR("Invalid SMC code to %s, FID %lx\n", __func__, fid);
		smc_ret->x[0] = SMC_UNK;
	}
}
