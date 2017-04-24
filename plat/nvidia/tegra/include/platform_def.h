/*
 * Copyright (c) 2015-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __PLATFORM_DEF_H__
#define __PLATFORM_DEF_H__

#include <arch.h>
#include <common_def.h>
#include <tegra_def.h>
#include <utils_def.h>

/*******************************************************************************
 * Generic platform constants
 ******************************************************************************/

/* Size of cacheable stacks */
#ifdef IMAGE_BL31
#define PLATFORM_STACK_SIZE 		U(0x400)
#endif

#define TEGRA_PRIMARY_CPU		U(0x0)

#define PLAT_MAX_PWR_LVL		MPIDR_AFFLVL2
#define PLATFORM_CORE_COUNT		(PLATFORM_CLUSTER_COUNT * \
					 PLATFORM_MAX_CPUS_PER_CLUSTER)
#define PLAT_NUM_PWR_DOMAINS		(PLATFORM_CORE_COUNT + \
					 PLATFORM_CLUSTER_COUNT + 1)

/*******************************************************************************
 * Platform console related constants
 ******************************************************************************/
#define TEGRA_CONSOLE_BAUDRATE		U(115200)
#define TEGRA_BOOT_UART_CLK_IN_HZ	U(408000000)

/*******************************************************************************
 * Platform memory map related constants
 ******************************************************************************/
/* Size of trusted dram */
#define TZDRAM_SIZE			U(0x00400000)
#define TZDRAM_END			(TZDRAM_BASE + TZDRAM_SIZE)

/*******************************************************************************
 * BL31 specific defines.
 ******************************************************************************/
#define BL31_SIZE			U(0x40000)
#define BL31_BASE			TZDRAM_BASE
#define BL31_LIMIT			(TZDRAM_BASE + BL31_SIZE - 1)
#define BL32_BASE			(TZDRAM_BASE + BL31_SIZE)
#define BL32_LIMIT			TZDRAM_END

/*******************************************************************************
 * Platform specific page table and MMU setup constants
 ******************************************************************************/
#define PLAT_PHY_ADDR_SPACE_SIZE	(ULL(1) << 35)
#define PLAT_VIRT_ADDR_SPACE_SIZE	(ULL(1) << 35)

/*******************************************************************************
 * Some data must be aligned on the biggest cache line size in the platform.
 * This is known only to the platform as it might have a combination of
 * integrated and external caches.
 ******************************************************************************/
#define CACHE_WRITEBACK_SHIFT		6
#define CACHE_WRITEBACK_GRANULE		(U(1) << CACHE_WRITEBACK_SHIFT)

#endif /* __PLATFORM_DEF_H__ */
