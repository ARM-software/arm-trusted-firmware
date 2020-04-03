/*
 * Copyright (c) 2016 - 2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <brcm_def.h>
#include <plat_brcm.h>

#if IMAGE_BL2
const mmap_region_t plat_brcm_mmap[] = {
	HSLS_REGION,
	BRCM_MAP_SHARED_RAM,
	BRCM_MAP_NAND_RO,
	BRCM_MAP_QSPI_RO,
#ifdef PERIPH0_REGION
	PERIPH0_REGION,
#endif
#ifdef PERIPH1_REGION
	PERIPH1_REGION,
#endif
#ifdef USE_DDR
	BRCM_MAP_NS_DRAM1,
#if BRCM_BL31_IN_DRAM
	BRCM_MAP_BL31_SEC_DRAM,
#endif
#else
#ifdef BRCM_MAP_EXT_SRAM
	BRCM_MAP_EXT_SRAM,
#endif
#endif
#if defined(USE_CRMU_SRAM) && defined(CRMU_SRAM_BASE)
	CRMU_SRAM_REGION,
#endif
	{0}
};
#endif

#if IMAGE_BL31
const mmap_region_t plat_brcm_mmap[] = {
	HSLS_REGION,
#ifdef PERIPH0_REGION
	PERIPH0_REGION,
#endif
#ifdef PERIPH1_REGION
	PERIPH1_REGION,
#endif
#ifdef PERIPH2_REGION
	PERIPH2_REGION,
#endif
#ifdef USB_REGION
	USB_REGION,
#endif
#ifdef USE_DDR
	BRCM_MAP_NS_DRAM1,
#ifdef BRCM_MAP_NS_SHARED_DRAM
	BRCM_MAP_NS_SHARED_DRAM,
#endif
#else
#ifdef BRCM_MAP_EXT_SRAM
	BRCM_MAP_EXT_SRAM,
#endif
#endif
#if defined(USE_CRMU_SRAM) && defined(CRMU_SRAM_BASE)
	CRMU_SRAM_REGION,
#endif
	{0}
};
#endif

CASSERT((ARRAY_SIZE(plat_brcm_mmap) - 1) <= PLAT_BRCM_MMAP_ENTRIES,
	assert_plat_brcm_mmap_mismatch);
CASSERT((PLAT_BRCM_MMAP_ENTRIES + BRCM_BL_REGIONS) <= MAX_MMAP_REGIONS,
	assert_max_mmap_regions);
