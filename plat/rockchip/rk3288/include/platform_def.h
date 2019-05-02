/*
 * Copyright (c) 2014-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#include <arch.h>
#include <lib/utils_def.h>
#include <plat/common/common_def.h>

#include <bl32_param.h>
#include <rk3288_def.h>

/*******************************************************************************
 * Platform binary types for linking
 ******************************************************************************/
#define PLATFORM_LINKER_FORMAT		"elf32-littlearm"
#define PLATFORM_LINKER_ARCH		arm

/*******************************************************************************
 * Generic platform constants
 ******************************************************************************/

/* Size of cacheable stacks */
#if defined(IMAGE_BL1)
#define PLATFORM_STACK_SIZE 0x440
#elif defined(IMAGE_BL2)
#define PLATFORM_STACK_SIZE 0x400
#elif defined(IMAGE_BL32)
#define PLATFORM_STACK_SIZE 0x800
#endif

#define FIRMWARE_WELCOME_STR		"Booting Trusted Firmware\n"

#define PLATFORM_MAX_AFFLVL		MPIDR_AFFLVL2
#define PLATFORM_SYSTEM_COUNT		1
#define PLATFORM_CLUSTER_COUNT		1
#define PLATFORM_CLUSTER0_CORE_COUNT	4
#define PLATFORM_CORE_COUNT		(PLATFORM_CLUSTER0_CORE_COUNT)
#define PLATFORM_MAX_CPUS_PER_CLUSTER	4
#define PLATFORM_NUM_AFFS		(PLATFORM_SYSTEM_COUNT +	\
					 PLATFORM_CLUSTER_COUNT +	\
					 PLATFORM_CORE_COUNT)

#define PLAT_RK_CLST_TO_CPUID_SHIFT	6

#define PLAT_MAX_PWR_LVL		MPIDR_AFFLVL2

/*
 * This macro defines the deepest retention state possible. A higher state
 * id will represent an invalid or a power down state.
 */
#define PLAT_MAX_RET_STATE		U(1)

/*
 * This macro defines the deepest power down states possible. Any state ID
 * higher than this is invalid.
 */
#define PLAT_MAX_OFF_STATE		U(2)

/*******************************************************************************
 * Platform specific page table and MMU setup constants
 ******************************************************************************/
#define PLAT_VIRT_ADDR_SPACE_SIZE	(1ULL << 32)
#define PLAT_PHY_ADDR_SPACE_SIZE	(1ULL << 32)
#define MAX_XLAT_TABLES			8
#define MAX_MMAP_REGIONS		18

/*******************************************************************************
 * Declarations and constants to access the mailboxes safely. Each mailbox is
 * aligned on the biggest cache line size in the platform. This is known only
 * to the platform as it might have a combination of integrated and external
 * caches. Such alignment ensures that two maiboxes do not sit on the same cache
 * line at any cache level. They could belong to different cpus/clusters &
 * get written while being protected by different locks causing corruption of
 * a valid mailbox address.
 ******************************************************************************/
#define CACHE_WRITEBACK_SHIFT		6
#define CACHE_WRITEBACK_GRANULE		(1 << CACHE_WRITEBACK_SHIFT)

/*
 * Define GICD and GICC and GICR base
 */
#define PLAT_RK_GICD_BASE		RK3288_GICD_BASE
#define PLAT_RK_GICC_BASE		RK3288_GICC_BASE

#define PLAT_RK_UART_BASE		UART2_BASE
#define PLAT_RK_UART_CLOCK		RK3288_UART_CLOCK
#define PLAT_RK_UART_BAUDRATE		RK3288_BAUDRATE

/* ClusterId is always 0x5 on rk3288, filter it */
#define PLAT_RK_MPIDR_CLUSTER_MASK	0
#define PLAT_RK_PRIMARY_CPU		0x0

#define PSRAM_DO_DDR_RESUME		0
#define PSRAM_CHECK_WAKEUP_CPU		0

#endif /* PLATFORM_DEF_H */
