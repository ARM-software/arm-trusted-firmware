/*
 * Copyright (c) 2014-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arm_def.h>
#include <arm_spm_def.h>
#include <debug.h>
#include <platform_def.h>
#include <tzc400.h>


/* Weak definitions may be overridden in specific ARM standard platform */
#pragma weak plat_arm_security_setup


/*******************************************************************************
 * Initialize the TrustZone Controller for ARM standard platforms.
 * Configure:
 *   - Region 0 with no access;
 *   - Region 1 with secure access only;
 *   - the remaining DRAM regions access from the given Non-Secure masters.
 *
 * When booting an EL3 payload, this is simplified: we configure region 0 with
 * secure access only and do not enable any other region.
 ******************************************************************************/
void arm_tzc400_setup(void)
{
	INFO("Configuring TrustZone Controller\n");

	tzc400_init(PLAT_ARM_TZC_BASE);

	/* Disable filters. */
	tzc400_disable_filters();

#ifndef EL3_PAYLOAD_BASE

	/* Region 0 set to no access by default */
	tzc400_configure_region0(TZC_REGION_S_NONE, 0);

	/* Region 1 set to cover Secure part of DRAM */
	tzc400_configure_region(PLAT_ARM_TZC_FILTERS, 1,
			ARM_AP_TZC_DRAM1_BASE, ARM_EL3_TZC_DRAM1_END,
			TZC_REGION_S_RDWR,
			0);

	/* Region 2 set to cover Non-Secure access to 1st DRAM address range.
	 * Apply the same configuration to given filters in the TZC. */
	tzc400_configure_region(PLAT_ARM_TZC_FILTERS, 2,
			ARM_NS_DRAM1_BASE, ARM_NS_DRAM1_END,
			ARM_TZC_NS_DRAM_S_ACCESS,
			PLAT_ARM_TZC_NS_DEV_ACCESS);

	/* Region 3 set to cover Non-Secure access to 2nd DRAM address range */
	tzc400_configure_region(PLAT_ARM_TZC_FILTERS, 3,
			ARM_DRAM2_BASE, ARM_DRAM2_END,
			ARM_TZC_NS_DRAM_S_ACCESS,
			PLAT_ARM_TZC_NS_DEV_ACCESS);

#if ENABLE_SPM
	/*
	 * Region 4 set to cover Non-Secure access to the communication buffer
	 * shared with the Secure world.
	 */
	tzc400_configure_region(PLAT_ARM_TZC_FILTERS,
				4,
				ARM_SP_IMAGE_NS_BUF_BASE,
				(ARM_SP_IMAGE_NS_BUF_BASE +
				 ARM_SP_IMAGE_NS_BUF_SIZE) - 1,
				TZC_REGION_S_NONE,
				PLAT_ARM_TZC_NS_DEV_ACCESS);
#endif

#else /* if defined(EL3_PAYLOAD_BASE) */

	/* Allow Secure and Non-secure access to DRAM for EL3 payloads */
	tzc400_configure_region0(TZC_REGION_S_RDWR, PLAT_ARM_TZC_NS_DEV_ACCESS);

#endif /* EL3_PAYLOAD_BASE */

	/*
	 * Raise an exception if a NS device tries to access secure memory
	 * TODO: Add interrupt handling support.
	 */
	tzc400_set_action(TZC_ACTION_ERR);

	/* Enable filters. */
	tzc400_enable_filters();
}

void plat_arm_security_setup(void)
{
	arm_tzc400_setup();
}
