/*
 * Copyright (c) 2015-2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#include <arch.h>
#include <common/tbbr/tbbr_img_def.h>
#include <plat/common/common_def.h>

#include <brcm_def.h>
#include "sr_def.h"
#include <cmn_plat_def.h>

/*
 * Most platform porting definitions provided by included headers
 */
#define PLAT_BRCM_SCP_TZC_DRAM1_SIZE	ULL(0x0)

/*
 * Required by standard platform porting definitions
 */
#define PLATFORM_CLUSTER0_CORE_COUNT	2
#define PLATFORM_CLUSTER1_CORE_COUNT	2
#define PLATFORM_CLUSTER2_CORE_COUNT	2
#define PLATFORM_CLUSTER3_CORE_COUNT	2

#define BRCM_SYSTEM_COUNT 1
#define BRCM_CLUSTER_COUNT 4

#define PLATFORM_CORE_COUNT	(PLATFORM_CLUSTER0_CORE_COUNT + \
					PLATFORM_CLUSTER1_CORE_COUNT+ \
					PLATFORM_CLUSTER2_CORE_COUNT+ \
					PLATFORM_CLUSTER3_CORE_COUNT)

#define PLAT_NUM_PWR_DOMAINS	(BRCM_SYSTEM_COUNT + \
				 BRCM_CLUSTER_COUNT + \
				 PLATFORM_CORE_COUNT)

#define PLAT_MAX_PWR_LVL	MPIDR_AFFLVL2

/* TBD-STINGRAY */
#define CACHE_WRITEBACK_SHIFT       6
/*
 * Some data must be aligned on the biggest cache line size in the platform.
 * This is known only to the platform as it might have a combination of
 * integrated and external caches.
 */
#define CACHE_WRITEBACK_GRANULE         (1 << CACHE_WRITEBACK_SHIFT)

/* TBD-STINGRAY */
#define PLATFORM_MAX_AFFLVL             MPIDR_AFFLVL1

#define BL1_PLATFORM_STACK_SIZE 0x3300
#define BL2_PLATFORM_STACK_SIZE 0xc000
#define BL11_PLATFORM_STACK_SIZE 0x2b00
#define DEFAULT_PLATFORM_STACK_SIZE 0x400
#if IMAGE_BL1
# define PLATFORM_STACK_SIZE BL1_PLATFORM_STACK_SIZE
#else
#if IMAGE_BL2
#ifdef USE_BL1_RW
# define PLATFORM_STACK_SIZE BL2_PLATFORM_STACK_SIZE
#else
# define PLATFORM_STACK_SIZE BL1_PLATFORM_STACK_SIZE
#endif
#else
#if IMAGE_BL11
# define PLATFORM_STACK_SIZE BL11_PLATFORM_STACK_SIZE
#else
# define PLATFORM_STACK_SIZE DEFAULT_PLATFORM_STACK_SIZE
#endif
#endif
#endif

#define PLAT_BRCM_TRUSTED_SRAM_BASE	0x66D00000
#define PLAT_BRCM_TRUSTED_SRAM_SIZE	0x00040000

#ifdef RUN_BL1_FROM_QSPI /* BL1 XIP from QSPI */
# define PLAT_BRCM_TRUSTED_ROM_BASE	QSPI_BASE_ADDR
#elif RUN_BL1_FROM_NAND /* BL1 XIP from NAND */
# define PLAT_BRCM_TRUSTED_ROM_BASE	NAND_BASE_ADDR
#else /* BL1 executed in ROM */
# define PLAT_BRCM_TRUSTED_ROM_BASE	ROM_BASE_ADDR
#endif
#define PLAT_BRCM_TRUSTED_ROM_SIZE	0x00040000

/*******************************************************************************
 * BL1 specific defines.
 ******************************************************************************/
#define BL1_RO_BASE		        PLAT_BRCM_TRUSTED_ROM_BASE
#define BL1_RO_LIMIT			(PLAT_BRCM_TRUSTED_ROM_BASE \
					+ PLAT_BRCM_TRUSTED_ROM_SIZE)

/*
 * Put BL1 RW at the beginning of the Trusted SRAM.
 */
#define BL1_RW_BASE			(BRCM_BL_RAM_BASE)
#define BL1_RW_LIMIT			(BL1_RW_BASE + 0x12000)

#define BL11_RW_BASE		BL1_RW_LIMIT
#define BL11_RW_LIMIT		(PLAT_BRCM_TRUSTED_SRAM_BASE + \
				PLAT_BRCM_TRUSTED_SRAM_SIZE)

/*******************************************************************************
 * BL2 specific defines.
 ******************************************************************************/
#if RUN_BL2_FROM_QSPI /* BL2 XIP from QSPI */
#define BL2_BASE			QSPI_BASE_ADDR
#define BL2_LIMIT			(BL2_BASE + 0x40000)
#define BL2_RW_BASE		BL1_RW_LIMIT
#define BL2_RW_LIMIT		(PLAT_BRCM_TRUSTED_SRAM_BASE + \
				PLAT_BRCM_TRUSTED_SRAM_SIZE)
#elif RUN_BL2_FROM_NAND /* BL2 XIP from NAND */
#define BL2_BASE			NAND_BASE_ADDR
#define BL2_LIMIT			(BL2_BASE + 0x40000)
#define BL2_RW_BASE		BL1_RW_LIMIT
#define BL2_RW_LIMIT		(PLAT_BRCM_TRUSTED_SRAM_BASE + \
				PLAT_BRCM_TRUSTED_SRAM_SIZE)
#else
#define BL2_BASE			(BL1_RW_LIMIT + PAGE_SIZE)
#define BL2_LIMIT			(BRCM_BL_RAM_BASE + BRCM_BL_RAM_SIZE)
#endif

/*
 * BL1 persistent area in internal SRAM
 * This area will increase as more features gets into BL1
 */
#define BL1_PERSISTENT_DATA_SIZE 0x2000

/* To reduce BL2 runtime footprint, we can re-use some BL1_RW area */
#define BL1_RW_RECLAIM_BASE (PLAT_BRCM_TRUSTED_SRAM_BASE + \
			     BL1_PERSISTENT_DATA_SIZE)

/*******************************************************************************
 * BL3-1 specific defines.
 ******************************************************************************/
/* Max Size of BL31 (in DRAM) */
#define PLAT_BRCM_MAX_BL31_SIZE		0x30000

#ifdef USE_DDR
#define BL31_BASE			BRCM_AP_TZC_DRAM1_BASE

#define BL31_LIMIT			(BRCM_AP_TZC_DRAM1_BASE + \
					PLAT_BRCM_MAX_BL31_SIZE)
#else
/* Put BL3-1 at the end of external on-board SRAM connected as NOR flash */
#define BL31_BASE			(NOR_BASE_ADDR + NOR_SIZE - \
					PLAT_BRCM_MAX_BL31_SIZE)

#define BL31_LIMIT			(NOR_BASE_ADDR + NOR_SIZE)
#endif

#define SECURE_DDR_END_ADDRESS		BL31_LIMIT

#ifdef NEED_SCP_BL2
#define SCP_BL2_BASE			BL31_BASE
#define PLAT_MAX_SCP_BL2_SIZE	0x9000
#define PLAT_SCP_COM_SHARED_MEM_BASE (CRMU_SHARED_SRAM_BASE)
/* dummy defined */
#define PLAT_BRCM_MHU_BASE		0x0
#endif

#define SECONDARY_CPU_SPIN_BASE_ADDR	BRCM_SHARED_RAM_BASE

/* Generic system timer counter frequency */
#ifndef SYSCNT_FREQ
#define SYSCNT_FREQ			(125 * 1000 * 1000)
#endif

/*
 * Enable the BL32 definitions, only when optee os is selected as secure
 * payload (BL32).
 */
#ifdef SPD_opteed
/*
 * Reserved Memory Map : SHMEM & TZDRAM.
 *
 * +--------+----------+ 0x8D000000
 * | SHMEM (NS)         | 16MB
 * +-------------------+ 0x8E000000
 * |        | TEE_RAM(S)| 4MB
 * + TZDRAM +----------+ 0x8E400000
 * |        | TA_RAM(S) | 12MB
 * +-------------------+ 0x8F000000
 * | BL31 Binary (S)    | 192KB
 * +-------------------+ 0x8F030000
 */

#define BL32_VA_SIZE		(4 * 1024 * 1024)
#define BL32_BASE		(0x8E000000)
#define BL32_LIMIT		(BL32_BASE + BL32_VA_SIZE)
#define TSP_SEC_MEM_BASE	BL32_BASE
#define TSP_SEC_MEM_SIZE	BL32_VA_SIZE
#endif

#ifdef SPD_opteed
	#define SECURE_DDR_BASE_ADDRESS BL32_BASE
#else
	#define SECURE_DDR_BASE_ADDRESS BL31_BASE
#endif
/*******************************************************************************
 * Platform specific page table and MMU setup constants
 ******************************************************************************/

#define MAX_XLAT_TABLES		7

#define PLAT_BRCM_MMAP_ENTRIES	10

#define MAX_MMAP_REGIONS		(PLAT_BRCM_MMAP_ENTRIES +	\
					 BRCM_BL_REGIONS)

#ifdef USE_DDR
#ifdef BL33_OVERRIDE_LOAD_ADDR
#define PLAT_BRCM_NS_IMAGE_OFFSET	BL33_OVERRIDE_LOAD_ADDR
#else
/*
 * BL3-3 image starting offset.
 * Putting start of DRAM as of now.
 */
#define PLAT_BRCM_NS_IMAGE_OFFSET	0x80000000
#endif /* BL33_OVERRIDE_LOAD_ADDR */
#else
/*
 * BL3-3 image starting offset.
 * Putting start of external on-board SRAM as of now.
 */
#define PLAT_BRCM_NS_IMAGE_OFFSET	NOR_BASE_ADDR
#endif /* USE_DDR */
/******************************************************************************
 * Required platform porting definitions common to all BRCM platforms
 *****************************************************************************/

#define MAX_IO_DEVICES			5
#define MAX_IO_HANDLES			6

#define PRIMARY_CPU		0

/* GIC Parameter */
#define PLAT_BRCM_GICD_BASE	GIC500_BASE
#define PLAT_BRCM_GICR_BASE	(GIC500_BASE + 0x200000)

/* Define secure interrupt as per Group here */
#define PLAT_BRCM_G1S_IRQ_PROPS(grp) \
	INTR_PROP_DESC(BRCM_IRQ_SEC_SGI_1, GIC_HIGHEST_SEC_PRIORITY, (grp), \
			GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(BRCM_IRQ_SEC_SPI_0, GIC_HIGHEST_SEC_PRIORITY, (grp), \
			GIC_INTR_CFG_EDGE)

#define PLAT_BRCM_G0_IRQ_PROPS(grp) \
	INTR_PROP_DESC(BRCM_IRQ_SEC_SGI_0, PLAT_SDEI_NORMAL_PRI, (grp), \
			GIC_INTR_CFG_EDGE), \

/*
 *CCN 502 related constants.
 */
#define PLAT_BRCM_CLUSTER_COUNT 4  /* Number of RN-F Masters */
#define PLAT_BRCM_CLUSTER_TO_CCN_ID_MAP	CLUSTER0_NODE_ID, CLUSTER1_NODE_ID, CLUSTER2_NODE_ID, CLUSTER3_NODE_ID
#define CCN_SIZE		0x1000000
#define CLUSTER0_NODE_ID	1
#define CLUSTER1_NODE_ID	7
#define CLUSTER2_NODE_ID	9
#define CLUSTER3_NODE_ID	15

#endif
