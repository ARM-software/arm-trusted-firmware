/*
 * Copyright (c) 2016-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <platform_def.h>

#include <common/debug.h>
#include <drivers/arm/tzc_dmc500.h>
#include <plat/arm/common/plat_arm.h>

/*******************************************************************************
 * Initialize the DMC500-TrustZone Controller for ARM standard platforms.
 * When booting an EL3 payload, this is simplified: we configure region 0 with
 * secure access only and do not enable any other region.
 ******************************************************************************/
void arm_tzc_dmc500_setup(tzc_dmc500_driver_data_t *plat_driver_data,
			const arm_tzc_regions_info_t *tzc_regions)
{
#ifndef EL3_PAYLOAD_BASE
	unsigned int region_index = 1U;
	const arm_tzc_regions_info_t *p;
	const arm_tzc_regions_info_t init_tzc_regions[] = {
		ARM_TZC_REGIONS_DEF,
		{0}
	};
#endif

	assert(plat_driver_data);

	INFO("Configuring DMC-500 TZ Settings\n");

	tzc_dmc500_driver_init(plat_driver_data);

#ifndef EL3_PAYLOAD_BASE
	if (tzc_regions == NULL)
		p = init_tzc_regions;
	else
		p = tzc_regions;

	/* Region 0 set to no access by default */
	tzc_dmc500_configure_region0(TZC_REGION_S_NONE, 0);

	/* Rest Regions set according to tzc_regions array */
	for (; p->base != 0ULL; p++) {
		tzc_dmc500_configure_region(region_index, p->base, p->end,
					    p->sec_attr, p->nsaid_permissions);
		region_index++;
	}

	INFO("Total %u regions set.\n", region_index);

#else
	/* Allow secure access only to DRAM for EL3 payloads */
	tzc_dmc500_configure_region0(TZC_REGION_S_RDWR, 0);
#endif
	/*
	 * Raise an exception if a NS device tries to access secure memory
	 * TODO: Add interrupt handling support.
	 */
	tzc_dmc500_set_action(TZC_ACTION_RV_LOWERR);

	/*
	 * Flush the configuration settings to have an affect. Validate
	 * flush by checking FILTER_EN is set on region 1 attributes
	 * register.
	 */
	tzc_dmc500_config_complete();

	/*
	 * Wait for the flush to complete.
	 * TODO: Have a timeout for this loop
	 */
	while (tzc_dmc500_verify_complete())
		;
}
