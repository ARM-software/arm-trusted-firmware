/*
 * Copyright (c) 2014-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <drivers/arm/nic_400.h>
#include <lib/mmio.h>
#include <platform_def.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/arm/soc/common/soc_css.h>
#include <plat/common/platform.h>

#include "juno_tzmp1_def.h"

#ifdef JUNO_TZMP1
/*
 * Protect buffer for VPU/GPU/DPU memory usage with hardware protection
 * enabled. Propose 224MB video output, 96 MB video input and 32MB video
 * private.
 *
 * Ind	Memory Range			Caption			  S_ATTR  NS_ATTR
 * 1	0x080000000 - 0x0E7FFFFFF	ARM_NS_DRAM1		  NONE	  RDWR | MEDIA_RW
 * 2	0x0E8000000 - 0x0F5FFFFFF	JUNO_MEDIA_TZC_PROT_DRAM1 NONE	  MEDIA_RW | AP_WR
 * 3	0x0F6000000 - 0x0FBFFFFFF	JUNO_VPU_TZC_PROT_DRAM1	  RDWR	  VPU_PROT_RW
 * 4	0x0FC000000 - 0x0FDFFFFFF	JUNO_VPU_TZC_PRIV_DRAM1	  RDWR	  VPU_PRIV_RW
 * 5	0x0FE000000 - 0x0FEFFFFFF	JUNO_AP_TZC_SHARE_DRAM1	  NONE	  RDWR | MEDIA_RW
 * 6	0x0FF000000 - 0x0FFFFFFFF	ARM_AP_TZC_DRAM1	  RDWR	  NONE
 * 7	0x880000000 - 0x9FFFFFFFF	ARM_DRAM2		  NONE	  RDWR | MEDIA_RW
 *
 * Memory regions are neighbored to save limited TZC regions. Calculation
 * started from ARM_TZC_SHARE_DRAM1 since it is known and fixed for both
 * protected-enabled and protected-disabled settings.
 *
 * Video private buffer aheads of ARM_TZC_SHARE_DRAM1
 */

static const arm_tzc_regions_info_t juno_tzmp1_tzc_regions[] = {
	{ARM_AP_TZC_DRAM1_BASE, ARM_AP_TZC_DRAM1_END, TZC_REGION_S_RDWR, 0},
	{JUNO_NS_DRAM1_PT1_BASE, JUNO_NS_DRAM1_PT1_END,
			TZC_REGION_S_NONE, JUNO_MEDIA_TZC_NS_DEV_ACCESS},
	{JUNO_MEDIA_TZC_PROT_DRAM1_BASE, JUNO_MEDIA_TZC_PROT_DRAM1_END,
			TZC_REGION_S_NONE, JUNO_MEDIA_TZC_PROT_ACCESS},
	{JUNO_VPU_TZC_PROT_DRAM1_BASE, JUNO_VPU_TZC_PROT_DRAM1_END,
			TZC_REGION_S_RDWR, JUNO_VPU_TZC_PROT_ACCESS},
	{JUNO_VPU_TZC_PRIV_DRAM1_BASE, JUNO_VPU_TZC_PRIV_DRAM1_END,
			TZC_REGION_S_RDWR, JUNO_VPU_TZC_PRIV_ACCESS},
	{JUNO_AP_TZC_SHARE_DRAM1_BASE, JUNO_AP_TZC_SHARE_DRAM1_END,
			TZC_REGION_S_NONE, JUNO_MEDIA_TZC_NS_DEV_ACCESS},
	{ARM_DRAM2_BASE, ARM_DRAM2_END,
			TZC_REGION_S_NONE, JUNO_MEDIA_TZC_NS_DEV_ACCESS},
	{},
};

/*******************************************************************************
 * Program dp650 to configure NSAID value for protected mode.
 ******************************************************************************/
static void init_dp650(void)
{
	mmio_write_32(DP650_BASE + DP650_PROT_NSAID_OFFSET,
		      DP650_PROT_NSAID_CONFIG);
}

/*******************************************************************************
 * Program v550 to configure NSAID value for protected mode.
 ******************************************************************************/
static void init_v550(void)
{
	/*
	 * bits[31:28] is for PRIVATE,
	 * bits[27:24] is for OUTBUF,
	 * bits[23:20] is for PROTECTED.
	 */
	mmio_write_32(V550_BASE + V550_PROTCTRL_OFFSET, V550_PROTCTRL_CONFIG);
}

#endif /* JUNO_TZMP1 */

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
#ifdef JUNO_TZMP1
	arm_tzc400_setup(juno_tzmp1_tzc_regions);
	INFO("TZC protected shared memory base address for TZMP usecase: %p\n",
	     (void *)JUNO_AP_TZC_SHARE_DRAM1_BASE);
	INFO("TZC protected shared memory end address for TZMP usecase: %p\n",
	     (void *)JUNO_AP_TZC_SHARE_DRAM1_END);
#else
	arm_tzc400_setup(NULL);
#endif
	/* Do ARM CSS internal NIC setup */
	css_init_nic400();
	/* Do ARM CSS SoC security setup */
	soc_css_security_setup();
	/* Initialize the SMMU SSD tables */
	init_mmu401();
#ifdef JUNO_TZMP1
	init_dp650();
	init_v550();
#endif
}

#if TRUSTED_BOARD_BOOT
int plat_get_mbedtls_heap(void **heap_addr, size_t *heap_size)
{
	return get_mbedtls_heap_helper(heap_addr, heap_size);
}
#endif
