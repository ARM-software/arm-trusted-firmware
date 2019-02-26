/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __PLATFORM_DEF_H__
#define __PLATFORM_DEF_H__

#include <arch.h>
#include <common/bl_common.h>
#include <common/interrupt_props.h>
#include <common/tbbr/tbbr_img_def.h>
#include <drivers/arm/gic_common.h>
#include <plat/common/common_def.h>


#define PLAT_CPUID_RELEASE			0xffe1b000
#define PLAT_S10_SEC_ENTRY			0xffe1b008

/* Define next boot image name and offset */
#define PLAT_NS_IMAGE_OFFSET			0x50000
#define PLAT_HANDOFF_OFFSET			0xFFE3F000

/*******************************************************************************
 * Platform binary types for linking
 ******************************************************************************/
#define PLATFORM_LINKER_FORMAT			"elf64-littleaarch64"
#define PLATFORM_LINKER_ARCH			aarch64

/* Stratix 10 supports up to 124GB RAM */
#define PLAT_PHY_ADDR_SPACE_SIZE		(1ULL << 39)
#define PLAT_VIRT_ADDR_SPACE_SIZE		(1ULL << 39)


/*******************************************************************************
 * Generic platform constants
 ******************************************************************************/
#define PLAT_PRIMARY_CPU			0
#define PLAT_SECONDARY_ENTRY_BASE		0x01f78bf0

/* Size of cacheable stacks */
#define PLATFORM_STACK_SIZE			0x2000

/* PSCI related constant */
#define PLAT_NUM_POWER_DOMAINS		5
#define PLAT_MAX_PWR_LVL		1
#define PLAT_MAX_RET_STATE		1
#define PLAT_MAX_OFF_STATE		2
#define PLATFORM_SYSTEM_COUNT			1
#define PLATFORM_CLUSTER_COUNT			1
#define PLATFORM_CLUSTER0_CORE_COUNT		4
#define PLATFORM_CLUSTER1_CORE_COUNT		0
#define PLATFORM_CORE_COUNT		(PLATFORM_CLUSTER1_CORE_COUNT + \
					PLATFORM_CLUSTER0_CORE_COUNT)
#define PLATFORM_MAX_CPUS_PER_CLUSTER		4

/* Interrupt related constant */

#define INTEL_S10_IRQ_SEC_PHY_TIMER		29

#define INTEL_S10_IRQ_SEC_SGI_0		8
#define INTEL_S10_IRQ_SEC_SGI_1		9
#define INTEL_S10_IRQ_SEC_SGI_2		10
#define INTEL_S10_IRQ_SEC_SGI_3		11
#define INTEL_S10_IRQ_SEC_SGI_4		12
#define INTEL_S10_IRQ_SEC_SGI_5		13
#define INTEL_S10_IRQ_SEC_SGI_6		14
#define INTEL_S10_IRQ_SEC_SGI_7		15

#define TSP_IRQ_SEC_PHY_TIMER		INTEL_S10_IRQ_SEC_PHY_TIMER
#define TSP_SEC_MEM_BASE		BL32_BASE
#define TSP_SEC_MEM_SIZE		(BL32_LIMIT - BL32_BASE + 1)
/*******************************************************************************
 * Platform memory map related constants
 ******************************************************************************/
#define DRAM_BASE				(0x0)
#define DRAM_SIZE				(0x80000000)

#define OCRAM_BASE				(0xFFE00000)
#define OCRAM_SIZE				(0x00040000)

#define MEM64_BASE				(0x0100000000)
#define MEM64_SIZE				(0x1F00000000)

#define DEVICE1_BASE				(0x80000000)
#define DEVICE1_SIZE				(0x60000000)

#define DEVICE2_BASE				(0xF7000000)
#define DEVICE2_SIZE				(0x08E00000)

#define DEVICE3_BASE				(0xFFFC0000)
#define DEVICE3_SIZE				(0x00008000)

#define DEVICE4_BASE				(0x2000000000)
#define DEVICE4_SIZE				(0x0100000000)

/*******************************************************************************
 * BL31 specific defines.
 ******************************************************************************/
/*
 * Put BL3-1 at the top of the Trusted SRAM (just below the shared memory, if
 * present). BL31_BASE is calculated using the current BL3-1 debug size plus a
 * little space for growth.
 */


#define FIRMWARE_WELCOME_STR		"Booting Trusted Firmware\n"

#define BL1_RO_BASE	(0xffe00000)
#define BL1_RO_LIMIT	(0xffe0f000)
#define BL1_RW_BASE	(0xffe10000)
#define BL1_RW_LIMIT	(0xffe1ffff)
#define BL1_RW_SIZE	(0x14000)

#define BL2_BASE	(0xffe00000)
#define BL2_LIMIT	(0xffe1b000)

#define BL31_BASE	(0xffe1c000)
#define BL31_LIMIT	(0xffe3bfff)

/*******************************************************************************
 * Platform specific page table and MMU setup constants
 ******************************************************************************/
#define MAX_XLAT_TABLES			8
#define MAX_MMAP_REGIONS		16

/*******************************************************************************
 * Declarations and constants to access the mailboxes safely. Each mailbox is
 * aligned on the biggest cache line size in the platform. This is known only
 * to the platform as it might have a combination of integrated and external
 * caches. Such alignment ensures that two maiboxes do not sit on the same cache
 * line at any cache level. They could belong to different cpus/clusters &
 * get written while being protected by different locks causing corruption of
 * a valid mailbox address.
 ******************************************************************************/
#define CACHE_WRITEBACK_SHIFT			6
#define CACHE_WRITEBACK_GRANULE		(1 << CACHE_WRITEBACK_SHIFT)

#define PLAT_GIC_BASE			(0xFFFC0000)
#define PLAT_GICC_BASE			(PLAT_GIC_BASE + 0x2000)
#define PLAT_GICD_BASE			(PLAT_GIC_BASE + 0x1000)
#define PLAT_GICR_BASE			0

/*******************************************************************************
 * UART related constants
 ******************************************************************************/
#define PLAT_UART0_BASE		(0xFFC02000)
#define PLAT_UART1_BASE		(0xFFC02100)

#define CRASH_CONSOLE_BASE	PLAT_UART0_BASE

#define PLAT_BAUDRATE			(115200)
#define PLAT_UART_CLOCK		(100000000)

/*******************************************************************************
 * System counter frequency related constants
 ******************************************************************************/
#define PLAT_SYS_COUNTER_FREQ_IN_TICKS	(400000000)
#define PLAT_SYS_COUNTER_FREQ_IN_MHZ	(400)

#define PLAT_INTEL_S10_GICD_BASE	PLAT_GICD_BASE
#define PLAT_INTEL_S10_GICC_BASE	PLAT_GICC_BASE

/*
 * Define a list of Group 1 Secure and Group 0 interrupts as per GICv3
 * terminology. On a GICv2 system or mode, the lists will be merged and treated
 * as Group 0 interrupts.
 */
#define PLAT_INTEL_S10_G1S_IRQ_PROPS(grp) \
	INTR_PROP_DESC(INTEL_S10_IRQ_SEC_PHY_TIMER, GIC_HIGHEST_SEC_PRIORITY, \
			grp, GIC_INTR_CFG_LEVEL), \
	INTR_PROP_DESC(INTEL_S10_IRQ_SEC_SGI_0, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(INTEL_S10_IRQ_SEC_SGI_1, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(INTEL_S10_IRQ_SEC_SGI_2, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(INTEL_S10_IRQ_SEC_SGI_3, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(INTEL_S10_IRQ_SEC_SGI_4, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(INTEL_S10_IRQ_SEC_SGI_5, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(INTEL_S10_IRQ_SEC_SGI_6, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(INTEL_S10_IRQ_SEC_SGI_7, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_EDGE)

#define PLAT_INTEL_S10_G0_IRQ_PROPS(grp)

#define MAX_IO_HANDLES                   4
#define MAX_IO_DEVICES                  4
#define MAX_IO_BLOCK_DEVICES             2


#endif /* __PLATFORM_DEF_H__ */

