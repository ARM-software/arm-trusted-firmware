/*
 * Copyright (c) 2014, ARM Limited and Contributors. All rights reserved.
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

#include <assert.h>
#include <debug.h>
#include <tzc400.h>
#include "fvp_def.h"
#include "fvp_private.h"

/* Used to improve readability for configuring regions. */
#define FILTER_SHIFT(filter)	(1 << filter)

/*
 * For the moment we assume that all security programming is done by the
 * primary core.
 * TODO:
 * Might want to enable interrupt on violations when supported?
 */
void fvp_security_setup(void)
{
	tzc_instance_t controller;

	/*
	 * The Base FVP has a TrustZone address space controller, the Foundation
	 * FVP does not. Trying to program the device on the foundation FVP will
	 * cause an abort.
	 *
	 * If the platform had additional peripheral specific security
	 * configurations, those would be configured here.
	 */

	if (!fvp_get_cfgvar(CONFIG_HAS_TZC))
		return;

	/*
	 * The TrustZone controller controls access to main DRAM. Give
	 * full NS access for the moment to use with OS.
	 */
	INFO("Configuring TrustZone Controller\n");

	/*
	 * The driver does some error checking and will assert.
	 * - Provide base address of device on platform.
	 * - Provide width of ACE-Lite IDs on platform.
	 */
	controller.base = TZC400_BASE;
	controller.aid_width = FVP_AID_WIDTH;
	tzc_init(&controller);

	/*
	 * Currently only filters 0 and 2 are connected on Base FVP.
	 * Filter 0 : CPU clusters (no access to DRAM by default)
	 * Filter 1 : not connected
	 * Filter 2 : LCDs (access to VRAM allowed by default)
	 * Filter 3 : not connected
	 * Programming unconnected filters will have no effect at the
	 * moment. These filter could, however, be connected in future.
	 * So care should be taken not to configure the unused filters.
	 */

	/* Disable all filters before programming. */
	tzc_disable_filters(&controller);

	/*
	 * Allow only non-secure access to all DRAM to supported devices.
	 * Give access to the CPUs and Virtio. Some devices
	 * would normally use the default ID so allow that too. We use
	 * two regions to cover the blocks of physical memory in the FVPs.
	 *
	 * Software executing in the secure state, such as a secure
	 * boot-loader, can access the DRAM by using the NS attributes in
	 * the MMU translation tables and descriptors.
	 */

	/* Set to cover the first block of DRAM */
	tzc_configure_region(&controller, FILTER_SHIFT(0), 1,
			DRAM1_BASE, DRAM1_END - DRAM1_SEC_SIZE,
			TZC_REGION_S_NONE,
			TZC_REGION_ACCESS_RDWR(FVP_NSAID_DEFAULT) |
			TZC_REGION_ACCESS_RDWR(FVP_NSAID_PCI) |
			TZC_REGION_ACCESS_RDWR(FVP_NSAID_AP) |
			TZC_REGION_ACCESS_RDWR(FVP_NSAID_VIRTIO) |
			TZC_REGION_ACCESS_RDWR(FVP_NSAID_VIRTIO_OLD));

	/* Set to cover the secure reserved region */
	tzc_configure_region(&controller, FILTER_SHIFT(0), 3,
			(DRAM1_END - DRAM1_SEC_SIZE) + 1 , DRAM1_END,
			TZC_REGION_S_RDWR,
			0x0);

	/* Set to cover the second block of DRAM */
	tzc_configure_region(&controller, FILTER_SHIFT(0), 2,
			DRAM2_BASE, DRAM2_END, TZC_REGION_S_NONE,
			TZC_REGION_ACCESS_RDWR(FVP_NSAID_DEFAULT) |
			TZC_REGION_ACCESS_RDWR(FVP_NSAID_PCI) |
			TZC_REGION_ACCESS_RDWR(FVP_NSAID_AP) |
			TZC_REGION_ACCESS_RDWR(FVP_NSAID_VIRTIO) |
			TZC_REGION_ACCESS_RDWR(FVP_NSAID_VIRTIO_OLD));

	/*
	 * TODO: Interrupts are not currently supported. The only
	 * options we have are for access errors to occur quietly or to
	 * cause an exception. We choose to cause an exception.
	 */
	tzc_set_action(&controller, TZC_ACTION_ERR);

	/* Enable filters. */
	tzc_enable_filters(&controller);
}
