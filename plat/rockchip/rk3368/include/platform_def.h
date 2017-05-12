/*
 * Copyright (c) 2014-2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __PLATFORM_DEF_H__
#define __PLATFORM_DEF_H__

#include <arch.h>
#include <common_def.h>
#include <rk3368_def.h>

#define DEBUG_XLAT_TABLE 0

/*******************************************************************************
 * Platform binary types for linking
 ******************************************************************************/
#define PLATFORM_LINKER_FORMAT		"elf64-littleaarch64"
#define PLATFORM_LINKER_ARCH		aarch64

/*******************************************************************************
 * Generic platform constants
 ******************************************************************************/

/* Size of cacheable stacks */
#if DEBUG_XLAT_TABLE
#define PLATFORM_STACK_SIZE 0x800
#elif defined(IMAGE_BL1)
#define PLATFORM_STACK_SIZE 0x440
#elif defined(IMAGE_BL2)
#define PLATFORM_STACK_SIZE 0x400
#elif defined(IMAGE_BL31)
#define PLATFORM_STACK_SIZE 0x800
#elif defined(IMAGE_BL32)
#define PLATFORM_STACK_SIZE 0x440
#endif

#define FIRMWARE_WELCOME_STR		"Booting Trusted Firmware\n"

#define PLATFORM_MAX_AFFLVL		MPIDR_AFFLVL2
#define PLATFORM_SYSTEM_COUNT		1
#define PLATFORM_CLUSTER_COUNT		2
#define PLATFORM_CLUSTER0_CORE_COUNT	4
#define PLATFORM_CLUSTER1_CORE_COUNT	4
#define PLATFORM_CORE_COUNT		(PLATFORM_CLUSTER1_CORE_COUNT +	\
					 PLATFORM_CLUSTER0_CORE_COUNT)
#define PLATFORM_MAX_CPUS_PER_CLUSTER	4
#define PLATFORM_NUM_AFFS		(PLATFORM_SYSTEM_COUNT +	\
					 PLATFORM_CLUSTER_COUNT +	\
					 PLATFORM_CORE_COUNT)

#define PLAT_RK_CLST_TO_CPUID_SHIFT	8

#define PLAT_MAX_PWR_LVL		MPIDR_AFFLVL2

/*
 * This macro defines the deepest retention state possible. A higher state
 * id will represent an invalid or a power down state.
 */
#define PLAT_MAX_RET_STATE		1

/*
 * This macro defines the deepest power down states possible. Any state ID
 * higher than this is invalid.
 */
#define PLAT_MAX_OFF_STATE		2

/*******************************************************************************
 * Platform memory map related constants
 ******************************************************************************/
/* TF txet, ro, rw, Size: 512KB */
#define TZRAM_BASE		(0x0)
#define TZRAM_SIZE		(0x80000)

/*******************************************************************************
 * BL31 specific defines.
 ******************************************************************************/
/*
 * Put BL3-1 at the top of the Trusted RAM
 */
#define BL31_BASE		(TZRAM_BASE + 0x10000)
#define BL31_LIMIT	(TZRAM_BASE + TZRAM_SIZE)

/*******************************************************************************
 * Platform specific page table and MMU setup constants
 ******************************************************************************/
#define ADDR_SPACE_SIZE		(1ull << 32)
#define MAX_XLAT_TABLES		8
#define MAX_MMAP_REGIONS	16

/*******************************************************************************
 * Declarations and constants to access the mailboxes safely. Each mailbox is
 * aligned on the biggest cache line size in the platform. This is known only
 * to the platform as it might have a combination of integrated and external
 * caches. Such alignment ensures that two maiboxes do not sit on the same cache
 * line at any cache level. They could belong to different cpus/clusters &
 * get written while being protected by different locks causing corruption of
 * a valid mailbox address.
 ******************************************************************************/
#define CACHE_WRITEBACK_SHIFT	6
#define CACHE_WRITEBACK_GRANULE	(1 << CACHE_WRITEBACK_SHIFT)

/*
 * Define GICD and GICC and GICR base
 */
#define PLAT_RK_GICD_BASE	RK3368_GICD_BASE
#define PLAT_RK_GICC_BASE	RK3368_GICC_BASE

/*
 * Define a list of Group 1 Secure and Group 0 interrupts as per GICv3
 * terminology. On a GICv2 system or mode, the lists will be merged and treated
 * as Group 0 interrupts.
 */
#define PLAT_RK_G1S_IRQS	RK_G1S_IRQS

#define PLAT_RK_UART_BASE	RK3368_UART2_BASE
#define PLAT_RK_UART_CLOCK	RK3368_UART_CLOCK
#define PLAT_RK_UART_BAUDRATE	RK3368_BAUDRATE

#define PLAT_RK_CCI_BASE	CCI400_BASE

#define PLAT_RK_PRIMARY_CPU	0x0

#define PSRAM_DO_DDR_RESUME	0
#define PSRAM_CHECK_WAKEUP_CPU	0

#endif /* __PLATFORM_DEF_H__ */
