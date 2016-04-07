/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <arm_def.h>
#include <assert.h>
#include <debug.h>
#include <platform_def.h>
#include <tzc_dmc500.h>

/*******************************************************************************
 * Initialize the DMC500-TrustZone Controller for ARM standard platforms.
 * Configure both the interfaces on Region 0 with no access, Region 1 with
 * secure access only, and the remaining DRAM regions access from the
 * given Non-Secure masters.
 *
 * When booting an EL3 payload, this is simplified: we configure region 0 with
 * secure access only and do not enable any other region.
 ******************************************************************************/
void arm_tzc_dmc500_setup(tzc_dmc500_driver_data_t *plat_driver_data)
{
	assert(plat_driver_data);

	INFO("Configuring DMC-500 TZ Settings\n");

	tzc_dmc500_driver_init(plat_driver_data);

#ifndef EL3_PAYLOAD_BASE
	/* Region 0 set to no access by default */
	tzc_dmc500_configure_region0(TZC_REGION_S_NONE, 0);

	/* Region 1 set to cover Secure part of DRAM */
	tzc_dmc500_configure_region(1, ARM_AP_TZC_DRAM1_BASE,
		ARM_AP_TZC_DRAM1_END,
		TZC_REGION_S_RDWR,
		0);

	/* Region 2 set to cover Non-Secure access to 1st DRAM address range.*/
	tzc_dmc500_configure_region(2,
		ARM_NS_DRAM1_BASE,
		ARM_NS_DRAM1_END,
		TZC_REGION_S_NONE,
		PLAT_ARM_TZC_NS_DEV_ACCESS);

	/* Region 3 set to cover Non-Secure access to 2nd DRAM address range */
	tzc_dmc500_configure_region(3,
		ARM_DRAM2_BASE,
		ARM_DRAM2_END,
		TZC_REGION_S_NONE,
		PLAT_ARM_TZC_NS_DEV_ACCESS);
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
