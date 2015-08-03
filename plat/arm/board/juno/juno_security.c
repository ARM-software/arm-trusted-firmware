/*
 * Copyright (c) 2014-2015, ARM Limited and Contributors. All rights reserved.
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

#include <mmio.h>
#include <nic_400.h>
#include <plat_arm.h>
#include <soc_css.h>
#include "juno_def.h"


/*******************************************************************************
 * Set up the MMU-401 SSD tables. The power-on configuration has all stream IDs
 * assigned to Non-Secure except some for the DMA-330. Assign those back to the
 * Non-Secure world as well, otherwise EL1 may end up erroneously generating
 * (untranslated) Secure transactions if it turns the SMMU on.
 ******************************************************************************/
static void init_mmu401(void)
{
	uint32_t reg = mmio_read_32(MMU401_DMA330_BASE + MMU401_SSD_OFFSET);
	reg |= 0x1FF;
	mmio_write_32(MMU401_DMA330_BASE + MMU401_SSD_OFFSET, reg);
}

/*******************************************************************************
 * Program CSS-NIC400 to allow non-secure access to some CSS regions.
 ******************************************************************************/
static void css_init_nic400(void)
{
	/* Note: This is the NIC-400 device on the CSS */
	mmio_write_32(PLAT_SOC_CSS_NIC400_BASE +
		NIC400_ADDR_CTRL_SECURITY_REG(CSS_NIC400_SLAVE_BOOTSECURE),
		~0);
}

/*******************************************************************************
 * Initialize the secure environment.
 ******************************************************************************/
void plat_arm_security_setup(void)
{
	/* Initialize the TrustZone Controller */
	arm_tzc_setup();
	/* Do ARM CSS internal NIC setup */
	css_init_nic400();
	/* Do ARM CSS SoC security setup */
	soc_css_security_setup();
	/* Initialize the SMMU SSD tables*/
	init_mmu401();
}
