/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#include <arch.h>
#include <lib/utils_def.h>

#include <tegra_def.h>

/*
 * Platform binary types for linking
 */
#define PLATFORM_LINKER_FORMAT		"elf64-littleaarch64"
#define PLATFORM_LINKER_ARCH		aarch64

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
#define TEGRA_BOOT_UART_CLK_13_MHZ	U(13000000)
#define TEGRA_BOOT_UART_CLK_408_MHZ	U(408000000)

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
 * Some data must be aligned on the biggest cache line size in the platform.
 * This is known only to the platform as it might have a combination of
 * integrated and external caches.
 ******************************************************************************/
#define CACHE_WRITEBACK_SHIFT		6
#define CACHE_WRITEBACK_GRANULE		(0x40) /* (U(1) << CACHE_WRITEBACK_SHIFT) */

/*******************************************************************************
 * Dummy macros to compile io_storage support
 ******************************************************************************/
#define MAX_IO_DEVICES			U(0)
#define MAX_IO_HANDLES			U(0)

#endif /* PLATFORM_DEF_H */
