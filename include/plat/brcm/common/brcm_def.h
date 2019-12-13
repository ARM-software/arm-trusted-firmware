/*
 * Copyright (c) 2016 - 2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BRCM_DEF_H
#define BRCM_DEF_H

#include <arch.h>
#include <common/tbbr/tbbr_img_def.h>
#include <lib/utils_def.h>
#include <lib/xlat_tables/xlat_tables.h>
#include <plat/common/common_def.h>

#include <platform_def.h>

#define PLAT_PHY_ADDR_SPACE_SIZE	BIT_64(32)
#define PLAT_VIRT_ADDR_SPACE_SIZE	BIT_64(32)

#define BL11_DAUTH_ID			0x796C51ab
#define BL11_DAUTH_BASE			BL11_RW_BASE

/* We keep a table at the end of ROM for function pointers */
#define ROM_TABLE_SIZE			32
#define BL1_ROM_TABLE			(BL1_RO_LIMIT - ROM_TABLE_SIZE)

/*
 * The top 16MB of DRAM1 is configured as secure access only using the TZC
 *   - SCP TZC DRAM: If present, DRAM reserved for SCP use
 *   - AP TZC DRAM: The remaining TZC secured DRAM reserved for AP use
 */
#define BRCM_TZC_DRAM1_SIZE		ULL(0x01000000)

#define BRCM_SCP_TZC_DRAM1_BASE		(BRCM_DRAM1_BASE +		\
					 BRCM_DRAM1_SIZE -		\
					 BRCM_SCP_TZC_DRAM1_SIZE)
#define BRCM_SCP_TZC_DRAM1_SIZE		PLAT_BRCM_SCP_TZC_DRAM1_SIZE

#define BRCM_AP_TZC_DRAM1_BASE		(BRCM_DRAM1_BASE +		\
					 BRCM_DRAM1_SIZE -		\
					 BRCM_TZC_DRAM1_SIZE)
#define BRCM_AP_TZC_DRAM1_SIZE		(BRCM_TZC_DRAM1_SIZE -		\
					 BRCM_SCP_TZC_DRAM1_SIZE)

#define BRCM_NS_DRAM1_BASE		BRCM_DRAM1_BASE
#define BRCM_NS_DRAM1_SIZE		(BRCM_DRAM1_SIZE -		\
					 BRCM_TZC_DRAM1_SIZE)

#ifdef BRCM_SHARED_DRAM_BASE
#define BRCM_NS_SHARED_DRAM_BASE	BRCM_SHARED_DRAM_BASE
#define BRCM_NS_SHARED_DRAM_SIZE	BRCM_SHARED_DRAM_SIZE
#endif

#define BRCM_MAP_SHARED_RAM		MAP_REGION_FLAT(		\
						BRCM_SHARED_RAM_BASE,	\
						BRCM_SHARED_RAM_SIZE,	\
						MT_DEVICE | MT_RW | MT_SECURE)

#define BRCM_MAP_NS_DRAM1		MAP_REGION_FLAT(		\
						BRCM_NS_DRAM1_BASE,	\
						BRCM_NS_DRAM1_SIZE,	\
						MT_MEMORY | MT_RW | MT_NS)

#ifdef BRCM_SHARED_DRAM_BASE
#define BRCM_MAP_NS_SHARED_DRAM		MAP_REGION_FLAT(		 \
						BRCM_NS_SHARED_DRAM_BASE, \
						BRCM_NS_SHARED_DRAM_SIZE, \
						MT_MEMORY | MT_RW | MT_NS)
#endif

#ifdef BRCM_EXT_SRAM_BASE
#define BRCM_MAP_EXT_SRAM		MAP_REGION_FLAT(		\
						BRCM_EXT_SRAM_BASE,	\
						BRCM_EXT_SRAM_SIZE,	\
						MT_DEVICE | MT_RW | MT_SECURE)
#endif

#define BRCM_MAP_NAND_RO		MAP_REGION_FLAT(NAND_BASE_ADDR,\
						NAND_SIZE,	\
						MT_MEMORY | MT_RO | MT_SECURE)

#define BRCM_MAP_QSPI_RO		MAP_REGION_FLAT(QSPI_BASE_ADDR,\
						QSPI_SIZE,	\
						MT_MEMORY | MT_RO | MT_SECURE)

#define HSLS_REGION	MAP_REGION_FLAT(HSLS_BASE_ADDR, \
					HSLS_SIZE, \
					MT_DEVICE | MT_RW | MT_SECURE)

#define CCN_REGION	MAP_REGION_FLAT(PLAT_BRCM_CCN_BASE, \
					CCN_SIZE, \
					MT_DEVICE | MT_RW | MT_SECURE)

#define GIC500_REGION	MAP_REGION_FLAT(GIC500_BASE, \
					GIC500_SIZE, \
					MT_DEVICE | MT_RW | MT_SECURE)
#ifdef PERIPH0_BASE
#define PERIPH0_REGION	MAP_REGION_FLAT(PERIPH0_BASE, \
					PERIPH0_SIZE, \
					MT_DEVICE | MT_RW | MT_SECURE)
#endif

#ifdef PERIPH1_BASE
#define PERIPH1_REGION	MAP_REGION_FLAT(PERIPH1_BASE, \
					PERIPH1_SIZE, \
					MT_DEVICE | MT_RW | MT_SECURE)
#endif

#ifdef PERIPH2_BASE
#define PERIPH2_REGION	MAP_REGION_FLAT(PERIPH2_BASE, \
					PERIPH2_SIZE, \
					MT_DEVICE | MT_RW | MT_SECURE)
#endif

#if BRCM_BL31_IN_DRAM
#if IMAGE_BL2
#define BRCM_MAP_BL31_SEC_DRAM		MAP_REGION_FLAT(		\
						BL31_BASE,		\
						PLAT_BRCM_MAX_BL31_SIZE,\
						MT_DEVICE | MT_RW | MT_SECURE)
#else
#define BRCM_MAP_BL31_SEC_DRAM		MAP_REGION_FLAT(		\
						BL31_BASE,		\
						PLAT_BRCM_MAX_BL31_SIZE,\
						MT_MEMORY | MT_RW | MT_SECURE)
#endif
#endif

#if defined(USB_BASE) && defined(DRIVER_USB_ENABLE)
#define USB_REGION			MAP_REGION_FLAT(  \
						USB_BASE, \
						USB_SIZE, \
						MT_DEVICE | MT_RW | MT_SECURE)
#endif

#ifdef USE_CRMU_SRAM
#define CRMU_SRAM_REGION		MAP_REGION_FLAT(		\
						CRMU_SRAM_BASE,		\
						CRMU_SRAM_SIZE,		\
						MT_DEVICE | MT_RW | MT_SECURE)
#endif
/*
 * The number of regions like RO(code), coherent and data required by
 * different BL stages which need to be mapped in the MMU.
 */
#if USE_COHERENT_MEM
#define BRCM_BL_REGIONS			3
#else
#define BRCM_BL_REGIONS			2
#endif

#endif /* BRCM_DEF_H */
