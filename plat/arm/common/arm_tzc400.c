/*
 * Copyright (c) 2014-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>

#include <common/debug.h>
#include <drivers/arm/tzc400.h>
#include <plat/arm/common/plat_arm.h>

/* Weak definitions may be overridden in specific ARM standard platform */
#pragma weak plat_arm_security_setup


/*******************************************************************************
 * Initialize the TrustZone Controller for ARM standard platforms.
 * When booting an EL3 payload, this is simplified: we configure region 0 with
 * secure access only and do not enable any other region.
 ******************************************************************************/
void arm_tzc400_setup(const arm_tzc_regions_info_t *tzc_regions)
{
#ifndef EL3_PAYLOAD_BASE
	unsigned int region_index = 1U;
	const arm_tzc_regions_info_t *p;
	const arm_tzc_regions_info_t init_tzc_regions[] = {
		ARM_TZC_REGIONS_DEF,
		{0}
	};
#endif

	INFO("Configuring TrustZone Controller\n");

	tzc400_init(PLAT_ARM_TZC_BASE);

	/* Disable filters. */
	tzc400_disable_filters();

#ifndef EL3_PAYLOAD_BASE
	if (tzc_regions == NULL)
		p = init_tzc_regions;
	else
		p = tzc_regions;

	/* Region 0 set to no access by default */
	tzc400_configure_region0(TZC_REGION_S_NONE, 0);

	/* Rest Regions set according to tzc_regions array */
	for (; p->base != 0ULL; p++) {
		tzc400_configure_region(PLAT_ARM_TZC_FILTERS, region_index,
			p->base, p->end, p->sec_attr, p->nsaid_permissions);
		region_index++;
	}

	INFO("Total %u regions set.\n", region_index);

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
	arm_tzc400_setup(NULL);
}
