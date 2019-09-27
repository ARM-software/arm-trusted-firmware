/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <bl31/interrupt_mgmt.h>
#include <common/bl_common.h>
#include <common/ep_info.h>
#include <lib/mmio.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <platform_def.h>
#include <stdint.h>

/*******************************************************************************
 * Platform memory map regions
 ******************************************************************************/
#define MAP_NSDRAM0		MAP_REGION_FLAT(AML_NSDRAM0_BASE,		\
						AML_NSDRAM0_SIZE,		\
						MT_MEMORY | MT_RW | MT_NS)

#define MAP_NS_SHARE_MEM	MAP_REGION_FLAT(AML_NS_SHARE_MEM_BASE,		\
						AML_NS_SHARE_MEM_SIZE,		\
						MT_MEMORY | MT_RW | MT_NS)

#define MAP_SEC_SHARE_MEM	MAP_REGION_FLAT(AML_SEC_SHARE_MEM_BASE,		\
						AML_SEC_SHARE_MEM_SIZE,		\
						MT_MEMORY | MT_RW | MT_SECURE)

#define MAP_SEC_DEVICE0		MAP_REGION_FLAT(AML_SEC_DEVICE0_BASE,		\
						AML_SEC_DEVICE0_SIZE,		\
						MT_DEVICE | MT_RW)

#define MAP_HDCP_RX		MAP_REGION_FLAT(AML_HDCP_RX_BASE,		\
						AML_HDCP_RX_SIZE,		\
						MT_DEVICE | MT_RW | MT_SECURE)

#define MAP_HDCP_TX		MAP_REGION_FLAT(AML_HDCP_TX_BASE,		\
						AML_HDCP_TX_SIZE,		\
						MT_DEVICE | MT_RW | MT_SECURE)

#define MAP_GIC_DEVICE		MAP_REGION_FLAT(AML_GIC_DEVICE_BASE,		\
						AML_GIC_DEVICE_SIZE,		\
						MT_DEVICE | MT_RW | MT_SECURE)

#define MAP_SEC_DEVICE1		MAP_REGION_FLAT(AML_SEC_DEVICE1_BASE,		\
						AML_SEC_DEVICE1_SIZE,		\
						MT_DEVICE | MT_RW | MT_SECURE)

#define MAP_SEC_DEVICE2		MAP_REGION_FLAT(AML_SEC_DEVICE2_BASE,		\
						AML_SEC_DEVICE2_SIZE,		\
						MT_DEVICE | MT_RW | MT_SECURE)

#define MAP_TZRAM		MAP_REGION_FLAT(AML_TZRAM_BASE,			\
						AML_TZRAM_SIZE,			\
						MT_DEVICE | MT_RW | MT_SECURE)

static const mmap_region_t g12a_mmap[] = {
	MAP_NSDRAM0,
	MAP_NS_SHARE_MEM,
	MAP_SEC_SHARE_MEM,
	MAP_SEC_DEVICE0,
	MAP_HDCP_RX,
	MAP_HDCP_TX,
	MAP_GIC_DEVICE,
	MAP_SEC_DEVICE1,
	MAP_SEC_DEVICE2,
	MAP_TZRAM,
	{0}
};

/*******************************************************************************
 * Per-image regions
 ******************************************************************************/
#define MAP_BL31	MAP_REGION_FLAT(BL31_BASE,					\
					BL31_END - BL31_BASE,				\
					MT_MEMORY | MT_RW | MT_SECURE)

#define MAP_BL_CODE	MAP_REGION_FLAT(BL_CODE_BASE,					\
					BL_CODE_END - BL_CODE_BASE,			\
					MT_CODE | MT_SECURE)

#define MAP_BL_RO_DATA	MAP_REGION_FLAT(BL_RO_DATA_BASE,				\
					BL_RO_DATA_END - BL_RO_DATA_BASE,		\
					MT_RO_DATA | MT_SECURE)

#define MAP_BL_COHERENT	MAP_REGION_FLAT(BL_COHERENT_RAM_BASE,				\
					BL_COHERENT_RAM_END - BL_COHERENT_RAM_BASE, 	\
					MT_DEVICE | MT_RW | MT_SECURE)

/*******************************************************************************
 * Function that sets up the translation tables.
 ******************************************************************************/
void aml_setup_page_tables(void)
{
#if IMAGE_BL31
	const mmap_region_t g12a_bl_mmap[] = {
		MAP_BL31,
		MAP_BL_CODE,
		MAP_BL_RO_DATA,
#if USE_COHERENT_MEM
		MAP_BL_COHERENT,
#endif
		{0}
	};
#endif

	mmap_add(g12a_bl_mmap);

	mmap_add(g12a_mmap);

	init_xlat_tables();
}

/*******************************************************************************
 * Function that returns the system counter frequency
 ******************************************************************************/
unsigned int plat_get_syscnt_freq2(void)
{
	mmio_clrbits_32(AML_SYS_CPU_CFG7, ~0xFDFFFFFF);
	mmio_clrbits_32(AML_AO_TIMESTAMP_CNTL, ~0xFFFFFE00);

	return AML_OSC24M_CLK_IN_HZ;
}
