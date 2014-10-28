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

#include <debug.h>
#include <tzc400.h>
#include "juno_def.h"

/*******************************************************************************
 * Initialize the TrustZone Controller. Configure Region 0 with Secure RW access
 * and allow Non-Secure masters full access
 ******************************************************************************/
static void init_tzc400(void)
{
	tzc_init(TZC400_BASE);

	/* Disable filters. */
	tzc_disable_filters();

	/* Region 1 set to cover Non-Secure DRAM at 0x8000_0000. Apply the
	 * same configuration to all filters in the TZC. */
	tzc_configure_region(REG_ATTR_FILTER_BIT_ALL, 1,
			DRAM_NS_BASE, DRAM_NS_BASE + DRAM_NS_SIZE - 1,
			TZC_REGION_S_NONE,
			TZC_REGION_ACCESS_RDWR(TZC400_NSAID_CCI400)	|
			TZC_REGION_ACCESS_RDWR(TZC400_NSAID_PCIE)	|
			TZC_REGION_ACCESS_RDWR(TZC400_NSAID_HDLCD0)	|
			TZC_REGION_ACCESS_RDWR(TZC400_NSAID_HDLCD1)	|
			TZC_REGION_ACCESS_RDWR(TZC400_NSAID_USB)	|
			TZC_REGION_ACCESS_RDWR(TZC400_NSAID_DMA330)	|
			TZC_REGION_ACCESS_RDWR(TZC400_NSAID_THINLINKS)	|
			TZC_REGION_ACCESS_RDWR(TZC400_NSAID_AP)		|
			TZC_REGION_ACCESS_RDWR(TZC400_NSAID_GPU)	|
			TZC_REGION_ACCESS_RDWR(TZC400_NSAID_CORESIGHT));

	/* Region 2 set to cover Secure DRAM */
	tzc_configure_region(REG_ATTR_FILTER_BIT_ALL, 2,
			DRAM_SEC_BASE, DRAM_SEC_BASE + DRAM_SEC_SIZE - 1,
			TZC_REGION_S_RDWR,
			0);

	/* Region 3 set to cover DRAM used by SCP for DDR retraining */
	tzc_configure_region(REG_ATTR_FILTER_BIT_ALL, 3,
			DRAM_SCP_BASE, DRAM_SCP_BASE + DRAM_SCP_SIZE - 1,
			TZC_REGION_S_NONE,
			TZC_REGION_ACCESS_RDWR(TZC400_NSAID_SCP));

	/* Region 4 set to cover Non-Secure DRAM at 0x8_8000_0000 */
	tzc_configure_region(REG_ATTR_FILTER_BIT_ALL, 4,
			DRAM2_BASE, DRAM2_BASE + DRAM2_SIZE - 1,
			TZC_REGION_S_NONE,
			TZC_REGION_ACCESS_RDWR(TZC400_NSAID_CCI400)	|
			TZC_REGION_ACCESS_RDWR(TZC400_NSAID_PCIE)	|
			TZC_REGION_ACCESS_RDWR(TZC400_NSAID_HDLCD0)	|
			TZC_REGION_ACCESS_RDWR(TZC400_NSAID_HDLCD1)	|
			TZC_REGION_ACCESS_RDWR(TZC400_NSAID_USB)	|
			TZC_REGION_ACCESS_RDWR(TZC400_NSAID_DMA330)	|
			TZC_REGION_ACCESS_RDWR(TZC400_NSAID_THINLINKS)	|
			TZC_REGION_ACCESS_RDWR(TZC400_NSAID_AP)		|
			TZC_REGION_ACCESS_RDWR(TZC400_NSAID_GPU)	|
			TZC_REGION_ACCESS_RDWR(TZC400_NSAID_CORESIGHT));

	/* Raise an exception if a NS device tries to access secure memory */
	tzc_set_action(TZC_ACTION_ERR);

	/* Enable filters. */
	tzc_enable_filters();
}

/*******************************************************************************
 * Initialize the secure environment. At this moment only the TrustZone
 * Controller is initialized.
 ******************************************************************************/
void plat_security_setup(void)
{
	/* Initialize the TrustZone Controller */
	init_tzc400();
}
