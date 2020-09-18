/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2020, NVIDIA Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#include <arch.h>
#include <lib/utils_def.h>

#include <tegra_def.h>

/*******************************************************************************
 * Check and error if SEPARATE_CODE_AND_RODATA is not set to 1
 ******************************************************************************/
#if !SEPARATE_CODE_AND_RODATA
#error "SEPARATE_CODE_AND_RODATA should be set to 1"
#endif

/*
 * Platform binary types for linking
 */
#define PLATFORM_LINKER_FORMAT		"elf64-littleaarch64"
#define PLATFORM_LINKER_ARCH		aarch64

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
					 PLATFORM_CLUSTER_COUNT + U(1))

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

/*******************************************************************************
 * Platforms macros to support SDEI
 ******************************************************************************/
#define TEGRA_SDEI_SGI_PRIVATE		U(8)

/*******************************************************************************
 * Platform macros to support exception handling framework
 ******************************************************************************/
#define PLAT_PRI_BITS			U(3)
#define PLAT_RAS_PRI			U(0x10)
#define PLAT_SDEI_CRITICAL_PRI		U(0x20)
#define PLAT_SDEI_NORMAL_PRI		U(0x30)
#define PLAT_TEGRA_WDT_PRIO		U(0x40)

#define PLAT_EHF_DESC			EHF_PRI_DESC(PLAT_PRI_BITS,\
						     PLAT_TEGRA_WDT_PRIO)

/*******************************************************************************
 * SDEI events
 ******************************************************************************/
/* SDEI dynamic private event numbers */
#define TEGRA_SDEI_DP_EVENT_0		U(100)
#define TEGRA_SDEI_DP_EVENT_1		U(101)
#define TEGRA_SDEI_DP_EVENT_2		U(102)

/* SDEI dynamic shared event numbers */
#define TEGRA_SDEI_DS_EVENT_0		U(200)
#define TEGRA_SDEI_DS_EVENT_1		U(201)
#define TEGRA_SDEI_DS_EVENT_2		U(202)

/* SDEI explicit events */
#define TEGRA_SDEI_EP_EVENT_0		U(300)
#define TEGRA_SDEI_EP_EVENT_1		U(301)
#define TEGRA_SDEI_EP_EVENT_2		U(302)
#define TEGRA_SDEI_EP_EVENT_3		U(303)
#define TEGRA_SDEI_EP_EVENT_4		U(304)
#define TEGRA_SDEI_EP_EVENT_5		U(305)
#define TEGRA_SDEI_EP_EVENT_6		U(306)
#define TEGRA_SDEI_EP_EVENT_7		U(307)
#define TEGRA_SDEI_EP_EVENT_8		U(308)
#define TEGRA_SDEI_EP_EVENT_9		U(309)
#define TEGRA_SDEI_EP_EVENT_10		U(310)
#define TEGRA_SDEI_EP_EVENT_11		U(311)

#endif /* PLATFORM_DEF_H */
