/*
 * Copyright (c) 2023, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __PLATFORM_DEF_H__
#define __PLATFORM_DEF_H__

#include <arch.h>
#include <common_def.h>
#include <rk3568_def.h>

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
#elif IMAGE_BL1
#define PLATFORM_STACK_SIZE 0x440
#elif IMAGE_BL2
#define PLATFORM_STACK_SIZE 0x400
#elif IMAGE_BL31
#define PLATFORM_STACK_SIZE 0x800
#elif IMAGE_BL32
#define PLATFORM_STACK_SIZE 0x440
#endif

#define FIRMWARE_WELCOME_STR		"Booting Trusted Firmware\n"

#define PLATFORM_SYSTEM_COUNT		1
#define PLATFORM_CLUSTER_COUNT		1
#define PLATFORM_CLUSTER0_CORE_COUNT	4

#define PLATFORM_CLUSTER1_CORE_COUNT	0
#define PLATFORM_CORE_COUNT		(PLATFORM_CLUSTER1_CORE_COUNT +	\
					 PLATFORM_CLUSTER0_CORE_COUNT)

#define PLATFORM_NUM_AFFS		(PLATFORM_SYSTEM_COUNT +	\
					 PLATFORM_CLUSTER_COUNT +	\
					 PLATFORM_CORE_COUNT)

#define PLAT_MAX_PWR_LVL		MPIDR_AFFLVL2

#define PLAT_RK_CLST_TO_CPUID_SHIFT	8

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
#define TZRAM_SIZE		(0x100000)

/*******************************************************************************
 * BL31 specific defines.
 ******************************************************************************/
/*
 * Put BL3-1 at the top of the Trusted RAM
 */
#define BL31_BASE		(TZRAM_BASE + 0x40000)
#define BL31_LIMIT		(TZRAM_BASE + TZRAM_SIZE)

/*******************************************************************************
 * Platform specific page table and MMU setup constants
 ******************************************************************************/
#define PLAT_PHY_ADDR_SPACE_SIZE	(1ULL << 32)
#define PLAT_VIRT_ADDR_SPACE_SIZE	(1ULL << 32)

#define ADDR_SPACE_SIZE			(1ull << 32)
#define MAX_XLAT_TABLES			18
#define MAX_MMAP_REGIONS		27

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
#define PLAT_RK_GICD_BASE	PLAT_GICD_BASE
#define PLAT_RK_GICC_BASE	PLAT_GICC_BASE
#define PLAT_RK_GICR_BASE	PLAT_GICR_BASE

/*
 * Define a list of Group 1 Secure and Group 0 interrupts as per GICv3
 * terminology. On a GICv2 system or mode, the lists will be merged and treated
 * as Group 0 interrupts.
 */

#define PLAT_RK_GICV3_G1S_IRQS						\
	INTR_PROP_DESC(RK_IRQ_SEC_PHY_TIMER, GIC_HIGHEST_SEC_PRIORITY, \
		       INTR_GROUP1S, GIC_INTR_CFG_LEVEL)

#define PLAT_RK_GICV3_G0_IRQS						\
	INTR_PROP_DESC(RK_IRQ_SEC_SGI_6, GIC_HIGHEST_SEC_PRIORITY,	\
		       INTR_GROUP0, GIC_INTR_CFG_LEVEL)

#define PLAT_RK_UART_BASE		FPGA_UART_BASE
#define PLAT_RK_UART_CLOCK		FPGA_UART_CLOCK
#define PLAT_RK_UART_BAUDRATE	FPGA_BAUDRATE

#define PLAT_RK_PRIMARY_CPU	0x0

#define ATAGS_PHYS_SIZE		0x2000
#define ATAGS_PHYS_BASE		(0x200000 - ATAGS_PHYS_SIZE)/* [2M-8K, 2M] */

#endif /* __PLATFORM_DEF_H__ */
