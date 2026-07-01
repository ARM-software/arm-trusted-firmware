/*
 * Copyright (c) 2020-2026, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#ifndef __ASSEMBLER__
#include <stdlib.h>
#endif

#include <arch.h>
#include <rza3m_def.h>

/*******************************************************************************
 * Platform binary types for linking
 ******************************************************************************/
#define PLATFORM_LINKER_FORMAT		"elf64-littleaarch64"
#define PLATFORM_LINKER_ARCH		aarch64

/*******************************************************************************
 * Generic platform constants
 ******************************************************************************/
#define PLATFORM_STACK_SIZE		U(0x1000)

#define PLATFORM_SYSTEM_COUNT		U(1)
#define PLATFORM_CLUSTER_COUNT		U(1)
#define PLATFORM_CORE_COUNT		U(1)

#define PLAT_MAX_PWR_LVL		MPIDR_AFFLVL2
#define PLAT_NUM_PWR_DOMAINS		(PLATFORM_CORE_COUNT + \
					 PLATFORM_CLUSTER_COUNT + \
					 PLATFORM_SYSTEM_COUNT)

#define PLAT_MAX_RET_STATE		U(1)
#define PLAT_MAX_OFF_STATE		U(2)
#define PLAT_MAX_PWR_LVL_STATES		U(2)

#define MAX_IO_DEVICES			U(2)
#define MAX_IO_HANDLES			U(2)
#define MAX_IO_BLOCK_DEVICES		U(1)

/*******************************************************************************
 * BL2 specific defines.
 ******************************************************************************/
#if !TRUSTED_BOARD_BOOT
#define BL2_BASE			0x00012000
#define BL2_LIMIT			0x0002F000
#else
#define BL2_BASE			0x00013000
#define BL2_LIMIT			0x0002F000
#endif

/*******************************************************************************
 * BL31 specific defines.
 ******************************************************************************/
#define BL31_BASE			0x44000000
#define BL31_LIMIT			0x44040000

/*******************************************************************************
 * BL32 specific defines.
 ******************************************************************************/
#ifndef SPD_none
#define BL32_BASE			0x44100000
#define BL32_LIMIT			(BL32_BASE + 0x100000)
#endif

/*******************************************************************************
 * BL33
 ******************************************************************************/
#define BL33_BASE			0x50000000
#define BL33_LIMIT			(BL33_BASE + 0x08000000)

/*******************************************************************************
 * BSP
 ******************************************************************************/
#define BSP_BASE			0x50000000
#define BSP_LIMIT			(BSP_BASE + 0x08000000)

/*******************************************************************************
 * Platform specific page table and MMU setup constants
 ******************************************************************************/
#if defined(IMAGE_BL2)
#define MAX_XLAT_TABLES			U(4)
#define MAX_MMAP_REGIONS		U(9)
#elif defined(IMAGE_BL31)
#define MAX_XLAT_TABLES			U(6)
#define MAX_MMAP_REGIONS		U(9)
#endif

#define PLAT_VIRT_ADDR_SPACE_SIZE	(1ULL << 34)
#define PLAT_PHY_ADDR_SPACE_SIZE	(1ULL << 34)

/*******************************************************************************
 * Determining the use of FIP format
 ******************************************************************************/
#define RZ_NOFIP			0
#define RZ_FIP				1

#define RZ_APP_PAYLOAD_OFFSET		0x200

/*******************************************************************************
 * Declarations and constants to access the mailboxes safely. Each mailbox is
 * aligned on the biggest cache line size in the platform. This is known only
 * to the platform as it might have a combination of integrated and external
 * caches. Such alignment ensures that two maiboxes do not sit on the same cache
 * line at any cache level. They could belong to different cpus/clusters &
 * get written while being protected by different locks causing corruption of
 * a valid mailbox address.
 ******************************************************************************/
#define CACHE_WRITEBACK_SHIFT		U(6)
#define CACHE_WRITEBACK_GRANULE		(U(1) << CACHE_WRITEBACK_SHIFT)

#endif /* PLATFORM_DEF_H */
