/*
 * Copyright (c) 2016-2017, Cavium Inc. All rights reserved.<BR>
 * Copyright (c) 2018 Facebook Inc.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __PLATFORM_DEF_H__
#define __PLATFORM_DEF_H__

#include <arch.h>
#include <common_def.h>
#include <tbbr_img_def.h>
#include <xlat_tables.h>

/*******************************************************************************
 * Generic platform constants
 ******************************************************************************/

#define ARM_BL31_PLAT_PARAM_VAL		0x0f1e2d3c4b5a6978ULL

/* Size of cacheable stacks */
#define PLATFORM_STACK_SIZE		0x1000

#define THUNDER_PRIMARY_CPU		0x0

/* Required platform porting definitions */
#define PLAT_NUM_PWR_DOMAINS		(PLATFORM_CLUSTER_COUNT + \
					PLATFORM_CORE_COUNT + \
					PLATFORM_NODE_COUNT)
#define PLAT_MAX_PWR_LVL		MPIDR_AFFLVL2

#define THUNDER81_MIDR			0x430F0A20
#define THUNDER83_MIDR			0x430F0A30
#define THUNDER88_MIDR			0x430F0A10

/* SATA-related definitions */
#define MAX_SATA_CONTROLLERS		16
#define MAX_SATA_GSER			8

/*******************************************************************************
 * Platform power states
 ******************************************************************************/
#define THUNDER_STATE_ON		0
#define THUNDER_STATE_RET		1
#define THUNDER_STATE_OFF		2
#define PLAT_MAX_RET_STATE		THUNDER_STATE_RET
#define PLAT_MAX_OFF_STATE		THUNDER_STATE_OFF

#define PLATFORM_MAX_NODES		2
#define PLATFORM_NODE_COUNT		(PLATFORM_MAX_NODES)
#define PLATFORM_MAX_CLUSTERS_PER_NODE	3
#define PLATFORM_CLUSTER_COUNT		(PLATFORM_MAX_NODES * \
						PLATFORM_MAX_CLUSTERS_PER_NODE)
#define PLATFORM_CORE_PER_CLUSTER	16
#define PLATFORM_MAX_CPUS_PER_CLUSTER	PLATFORM_CORE_PER_CLUSTER
#define PLATFORM_CORE_COUNT		(PLATFORM_CLUSTER_COUNT * \
						PLATFORM_CORE_PER_CLUSTER)

/* Cavium ThunderX doesn't support AArch32 in EL1/2 */

#define NO_AARCH32_EL2	1

/*******************************************************************************
 * Declarations and constants to access the mailboxes safely. Each mailbox is
 * aligned on the biggest cache line size in the platform. This is known only
 * to the platform as it might have a combination of integrated and external
 * caches. Such alignment ensures that two maiboxes do not sit on the same cache
 * line at any cache level. They could belong to different cpus/clusters &
 * get written while being protected by different locks causing corruption of
 * a valid mailbox address.
 ******************************************************************************/
#define CACHE_WRITEBACK_SHIFT   7
#define CACHE_WRITEBACK_GRANULE (1 << CACHE_WRITEBACK_SHIFT)

#define MAX_IO_DEVICES			5
#define MAX_IO_HANDLES			4

/*******************************************************************************
 * Platform specific page table and MMU setup constants
 ******************************************************************************/
#define ADDR_SPACE_SIZE_SHIFT		48
#define PLAT_VIRT_ADDR_SPACE_SIZE	(1ull << ADDR_SPACE_SIZE_SHIFT)
#define PLAT_PHY_ADDR_SPACE_SIZE	(1ull << ADDR_SPACE_SIZE_SHIFT)
#define MAX_XLAT_TABLES			48
#define MAX_MMAP_REGIONS		256

/* Location of trusted dram on the base thunder */
#define TZDRAM_BASE			0x00000000
#define TZDRAM_SIZE			0x00100000

#define FDT_MAX_SIZE			0x20000
#define FDT_BASE			(TZDRAM_BASE + TZDRAM_SIZE - FDT_MAX_SIZE)

#ifndef SPD_none
#define SPD_SIZE (1 << 16)
#else
#define SPD_SIZE 0
#endif

#define BL31_LIMIT			FDT_BASE
#define BL31_MAX_SIZE			(0x00090000 + MAX_XLAT_TABLES * PAGE_SIZE + SPD_SIZE)
#define BL31_BASE			(BL31_LIMIT - BL31_MAX_SIZE)

#define TSP_IRQ_SEC_PHY_TIMER		29
#define TSP_SEC_MEM_BASE		TZDRAM_BASE
#define TSP_SEC_MEM_SIZE		TZDRAM_SIZE

/* Load address of BL33 in the ThunderX port */
#ifdef PLAT_t83
#define NS_IMAGE_BASE			0x000200000
#else
#define NS_IMAGE_BASE			0x000100000
#endif
#define NS_IMAGE_MAX_SIZE		(0x040000000 - NS_IMAGE_BASE)

#define NS_DMA_MEMORY_SIZE		0x100000

/*
 * PL011 related constants
 */

#define PLAT_CAVIUM_CONSOLE_BAUDRATE      115200

#endif
