/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:	BSD-3-Clause
 * https://spdx.org/licenses
 */

#ifndef MARVELL_DEF_H
#define MARVELL_DEF_H

#include <platform_def.h>

#include <arch.h>
#include <common/tbbr/tbbr_img_def.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <plat/common/common_def.h>

/****************************************************************************
 * Definitions common to all MARVELL standard platforms
 ****************************************************************************
 */
/* Special value used to verify platform parameters from BL2 to BL31 */
#define MARVELL_BL31_PLAT_PARAM_VAL		0x0f1e2d3c4b5a6978ULL

#define PLAT_MARVELL_NORTHB_COUNT		1

#define PLAT_MARVELL_CLUSTER_COUNT		1

#define MARVELL_CACHE_WRITEBACK_SHIFT		6

/*
 * Macros mapping the MPIDR Affinity levels to MARVELL Platform Power levels.
 * The power levels have a 1:1 mapping with the MPIDR affinity levels.
 */
#define MARVELL_PWR_LVL0		MPIDR_AFFLVL0
#define MARVELL_PWR_LVL1		MPIDR_AFFLVL1
#define MARVELL_PWR_LVL2		MPIDR_AFFLVL2

/*
 *  Macros for local power states in Marvell platforms encoded by State-ID field
 *  within the power-state parameter.
 */
/* Local power state for power domains in Run state. */
#define MARVELL_LOCAL_STATE_RUN	0
/* Local power state for retention. Valid only for CPU power domains */
#define MARVELL_LOCAL_STATE_RET	1
/* Local power state for OFF/power-down.
 * Valid for CPU and cluster power domains
 */
#define MARVELL_LOCAL_STATE_OFF	2

/* The first 4KB of Trusted SRAM are used as shared memory */
#define MARVELL_TRUSTED_SRAM_BASE	PLAT_MARVELL_ATF_BASE
#define MARVELL_SHARED_RAM_BASE		MARVELL_TRUSTED_SRAM_BASE
#define MARVELL_SHARED_RAM_SIZE		0x00001000	/* 4 KB */

/* The remaining Trusted SRAM is used to load the BL images */
#define MARVELL_BL_RAM_BASE		(MARVELL_SHARED_RAM_BASE + \
					 MARVELL_SHARED_RAM_SIZE)
#define MARVELL_BL_RAM_SIZE		(PLAT_MARVELL_TRUSTED_SRAM_SIZE - \
					 MARVELL_SHARED_RAM_SIZE)

#define MARVELL_DRAM_BASE		ULL(0x0)
#define MARVELL_DRAM_SIZE		ULL(0x20000000)
#define MARVELL_DRAM_END		(MARVELL_DRAM_BASE + \
					 MARVELL_DRAM_SIZE - 1)

#define MARVELL_IRQ_SEC_PHY_TIMER		29

#define MARVELL_IRQ_SEC_SGI_0		8
#define MARVELL_IRQ_SEC_SGI_1		9
#define MARVELL_IRQ_SEC_SGI_2		10
#define MARVELL_IRQ_SEC_SGI_3		11
#define MARVELL_IRQ_SEC_SGI_4		12
#define MARVELL_IRQ_SEC_SGI_5		13
#define MARVELL_IRQ_SEC_SGI_6		14
#define MARVELL_IRQ_SEC_SGI_7		15

#define MARVELL_MAP_SHARED_RAM		MAP_REGION_FLAT(		 \
						MARVELL_SHARED_RAM_BASE, \
						MARVELL_SHARED_RAM_SIZE, \
						MT_MEMORY | MT_RW | MT_SECURE)

#define MARVELL_MAP_DRAM		MAP_REGION_FLAT(		\
						MARVELL_DRAM_BASE,	\
						MARVELL_DRAM_SIZE,	\
						MT_MEMORY | MT_RW | MT_NS)


/*
 * The number of regions like RO(code), coherent and data required by
 * different BL stages which need to be mapped in the MMU.
 */
#if USE_COHERENT_MEM
#define MARVELL_BL_REGIONS		3
#else
#define MARVELL_BL_REGIONS		2
#endif

#define MAX_MMAP_REGIONS		(PLAT_MARVELL_MMAP_ENTRIES + \
					 MARVELL_BL_REGIONS)

#define MARVELL_CONSOLE_BAUDRATE	115200

/****************************************************************************
 * Required platform porting definitions common to all MARVELL std. platforms
 ****************************************************************************
 */
#define PLAT_PHY_ADDR_SPACE_SIZE	(1ULL << 32)
#define PLAT_VIRT_ADDR_SPACE_SIZE	(1ULL << 32)

/*
 * This macro defines the deepest retention state possible. A higher state
 * id will represent an invalid or a power down state.
 */
#define PLAT_MAX_RET_STATE		MARVELL_LOCAL_STATE_RET

/*
 * This macro defines the deepest power down states possible. Any state ID
 * higher than this is invalid.
 */
#define PLAT_MAX_OFF_STATE		MARVELL_LOCAL_STATE_OFF


#define PLATFORM_CORE_COUNT		PLAT_MARVELL_CLUSTER_CORE_COUNT

/*
 * Some data must be aligned on the biggest cache line size in the platform.
 * This is known only to the platform as it might have a combination of
 * integrated and external caches.
 */
#define CACHE_WRITEBACK_GRANULE		(1 << MARVELL_CACHE_WRITEBACK_SHIFT)


/*****************************************************************************
 * BL1 specific defines.
 * BL1 RW data is relocated from ROM to RAM at runtime so we need 2 sets of
 * addresses.
 *****************************************************************************
 */
#define BL1_RO_BASE		PLAT_MARVELL_TRUSTED_ROM_BASE
#define BL1_RO_LIMIT		(PLAT_MARVELL_TRUSTED_ROM_BASE	\
					+ PLAT_MARVELL_TRUSTED_ROM_SIZE)
/*
 * Put BL1 RW at the top of the Trusted SRAM.
 */
#define BL1_RW_BASE		(MARVELL_BL_RAM_BASE +		\
					MARVELL_BL_RAM_SIZE -	\
					PLAT_MARVELL_MAX_BL1_RW_SIZE)
#define BL1_RW_LIMIT		(MARVELL_BL_RAM_BASE + MARVELL_BL_RAM_SIZE)

/*****************************************************************************
 * BL2 specific defines.
 *****************************************************************************
 */
/*
 * Put BL2 just below BL31.
 */
#define BL2_BASE		(BL31_BASE - PLAT_MARVELL_MAX_BL2_SIZE)
#define BL2_LIMIT		BL31_BASE

/*****************************************************************************
 * BL31 specific defines.
 *****************************************************************************
 */
/*
 * Put BL31 at the top of the Trusted SRAM.
 */
#define BL31_BASE		(MARVELL_BL_RAM_BASE + \
					MARVELL_BL_RAM_SIZE - \
					PLAT_MARVEL_MAX_BL31_SIZE)
#define BL31_PROGBITS_LIMIT	BL1_RW_BASE
#define BL31_LIMIT			(MARVELL_BL_RAM_BASE +	\
					 MARVELL_BL_RAM_SIZE)


#endif /* MARVELL_DEF_H */
