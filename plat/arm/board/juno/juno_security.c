/*
 * Copyright (c) 2014-2015, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
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
 * Initialize debug configuration.
 ******************************************************************************/
static void init_debug_cfg(void)
{
#if !DEBUG
	/* Set internal drive selection for SPIDEN. */
	mmio_write_32(SSC_REG_BASE + SSC_DBGCFG_SET,
		1U << SPIDEN_SEL_SET_SHIFT);

	/* Drive SPIDEN LOW to disable invasive debug of secure state. */
	mmio_write_32(SSC_REG_BASE + SSC_DBGCFG_CLR,
		1U << SPIDEN_INT_CLR_SHIFT);
#endif
}

/*******************************************************************************
 * Initialize the secure environment.
 ******************************************************************************/
void plat_arm_security_setup(void)
{
	/* Initialize debug configuration */
	init_debug_cfg();
	/* Initialize the TrustZone Controller */
	arm_tzc400_setup();
	/* Do ARM CSS internal NIC setup */
	css_init_nic400();
	/* Do ARM CSS SoC security setup */
	soc_css_security_setup();
	/* Initialize the SMMU SSD tables */
	init_mmu401();
}
