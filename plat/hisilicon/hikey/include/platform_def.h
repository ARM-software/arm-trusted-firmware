/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __PLATFORM_DEF_H__
#define __PLATFORM_DEF_H__

#include <arch.h>
#include "../hikey_def.h"

/* Special value used to verify platform parameters from BL2 to BL3-1 */
#define HIKEY_BL31_PLAT_PARAM_VAL	0x0f1e2d3c4b5a6978ULL

/*
 * Generic platform constants
 */

/* Size of cacheable stacks */
#define PLATFORM_STACK_SIZE		0x800

#define FIRMWARE_WELCOME_STR		"Booting Trusted Firmware\n"

#define PLATFORM_CACHE_LINE_SIZE	64
#define PLATFORM_CLUSTER_COUNT		2
#define PLATFORM_CORE_COUNT_PER_CLUSTER	4
#define PLATFORM_CORE_COUNT		(PLATFORM_CLUSTER_COUNT *	\
					 PLATFORM_CORE_COUNT_PER_CLUSTER)
#define PLAT_MAX_PWR_LVL		MPIDR_AFFLVL2
#define PLAT_NUM_PWR_DOMAINS		(PLATFORM_CORE_COUNT + \
					 PLATFORM_CLUSTER_COUNT + 1)

#define PLAT_MAX_RET_STATE		1
#define PLAT_MAX_OFF_STATE		2

#define MAX_IO_DEVICES			3
#define MAX_IO_HANDLES			4
/* eMMC RPMB and eMMC User Data */
#define MAX_IO_BLOCK_DEVICES		2

/* GIC related constants (no GICR in GIC-400) */
#define PLAT_ARM_GICD_BASE		0xF6801000
#define PLAT_ARM_GICC_BASE		0xF6802000
#define PLAT_ARM_GICH_BASE		0xF6804000
#define PLAT_ARM_GICV_BASE		0xF6806000


/*
 * Platform memory map related constants
 */

/*
 * BL1 is stored in XG2RAM0_HIRQ that is 784KB large (0xF980_0000~0xF98C_4000).
 */
#define ONCHIPROM_PARAM_BASE		(XG2RAM0_BASE + 0x700)
#define LOADER_RAM_BASE			(XG2RAM0_BASE + 0x800)
#define BL1_XG2RAM0_OFFSET		0x1000

/*
 * BL1 specific defines.
 *
 * Both loader and BL1_RO region stay in SRAM since they are used to simulate
 * ROM.
 * Loader is used to switch Hi6220 SoC from 32-bit to 64-bit mode.
 *
 * ++++++++++  0xF980_0000
 * + loader +
 * ++++++++++  0xF980_1000
 * + BL1_RO +
 * ++++++++++  0xF981_0000
 * + BL1_RW +
 * ++++++++++  0xF989_8000
 */
#define BL1_RO_BASE			(XG2RAM0_BASE + BL1_XG2RAM0_OFFSET)
#define BL1_RO_LIMIT			(XG2RAM0_BASE + 0x10000)
#define BL1_RW_BASE			(BL1_RO_LIMIT)	/* 0xf981_0000 */
#define BL1_RW_SIZE			(0x00088000)
#define BL1_RW_LIMIT			(0xF9898000)

/*
 * BL2 specific defines.
 */
#define BL2_BASE			(BL1_RW_BASE + 0x8000)	/* 0xf981_8000 */
#define BL2_LIMIT			(BL2_BASE + 0x40000)

/*
 * SCP_BL2 specific defines.
 * In HiKey, SCP_BL2 means MCU firmware. It's loaded into the temporary buffer
 * at 0x0100_0000. Then BL2 will parse the sections and loaded them into
 * predefined separated buffers.
 */
#define SCP_BL2_BASE			(DDR_BASE + 0x01000000)
#define SCP_BL2_LIMIT			(SCP_BL2_BASE + 0x00100000)
#define SCP_BL2_SIZE			(SCP_BL2_LIMIT - SCP_BL2_BASE)

/*
 * BL31 specific defines.
 */
#define BL31_BASE			BL2_LIMIT /* 0xf985_8000 */
#define BL31_LIMIT			0xF9898000

/*
 * BL3-2 specific defines.
 */

/*
 * The TSP currently executes from TZC secured area of DRAM or SRAM.
 */
#define BL32_SRAM_BASE			BL31_LIMIT
#define BL32_SRAM_LIMIT			(BL31_LIMIT+0x80000) /* 512K */

#define BL32_DRAM_BASE			DDR_SEC_BASE
#define BL32_DRAM_LIMIT			(DDR_SEC_BASE+DDR_SEC_SIZE)

#ifdef SPD_opteed
/* Load pageable part of OP-TEE at end of allocated DRAM space for BL32 */
#define HIKEY_OPTEE_PAGEABLE_LOAD_BASE	(BL32_DRAM_LIMIT - HIKEY_OPTEE_PAGEABLE_LOAD_SIZE) /* 0x3FC0_0000 */
#define HIKEY_OPTEE_PAGEABLE_LOAD_SIZE	0x400000 /* 4MB */
#endif

#if (HIKEY_TSP_RAM_LOCATION_ID == HIKEY_DRAM_ID)
#define TSP_SEC_MEM_BASE		BL32_DRAM_BASE
#define TSP_SEC_MEM_SIZE		(BL32_DRAM_LIMIT - BL32_DRAM_BASE)
#define BL32_BASE			BL32_DRAM_BASE
#define BL32_LIMIT			BL32_DRAM_LIMIT
#elif (HIKEY_TSP_RAM_LOCATION_ID == HIKEY_SRAM_ID)
#define TSP_SEC_MEM_BASE		BL32_SRAM_BASE
#define TSP_SEC_MEM_SIZE		(BL32_SRAM_LIMIT - BL32_SRAM_BASE)
#define BL32_BASE			BL32_SRAM_BASE
#define BL32_LIMIT			BL32_SRAM_LIMIT
#else
#error "Currently unsupported HIKEY_TSP_LOCATION_ID value"
#endif

/* BL32 is mandatory in AArch32 */
#ifndef AARCH32
#ifdef SPD_none
#undef BL32_BASE
#endif /* SPD_none */
#endif

#define NS_BL1U_BASE			(BL2_BASE)
#define NS_BL1U_SIZE			(0x00010000)
#define NS_BL1U_LIMIT			(NS_BL1U_BASE + NS_BL1U_SIZE)

/*
 * Platform specific page table and MMU setup constants
 */
#define ADDR_SPACE_SIZE			(1ull << 32)

#if defined(IMAGE_BL1) || defined(IMAGE_BL32)
#define MAX_XLAT_TABLES			3
#endif

#ifdef IMAGE_BL31
#define MAX_XLAT_TABLES			4
#endif

#ifdef IMAGE_BL2
#if LOAD_IMAGE_V2
#ifdef SPD_opteed
#define MAX_XLAT_TABLES			4
#else
#define MAX_XLAT_TABLES			3
#endif
#else
#define MAX_XLAT_TABLES			3
#endif
#endif

#define MAX_MMAP_REGIONS		16

#define HIKEY_NS_IMAGE_OFFSET		(DDR_BASE + 0x35000000)

/*
 * Declarations and constants to access the mailboxes safely. Each mailbox is
 * aligned on the biggest cache line size in the platform. This is known only
 * to the platform as it might have a combination of integrated and external
 * caches. Such alignment ensures that two maiboxes do not sit on the same cache
 * line at any cache level. They could belong to different cpus/clusters &
 * get written while being protected by different locks causing corruption of
 * a valid mailbox address.
 */
#define CACHE_WRITEBACK_SHIFT		6
#define CACHE_WRITEBACK_GRANULE		(1 << CACHE_WRITEBACK_SHIFT)

#endif /* __PLATFORM_DEF_H__ */
