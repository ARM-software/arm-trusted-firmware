/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#include <arch.h>
#include <common/tbbr/tbbr_img_def.h>
#include <plat/common/common_def.h>

#define PLATFORM_STACK_SIZE		0x1000

#define PLATFORM_MAX_CPUS_PER_CLUSTER	2
#define PLATFORM_CLUSTER_COUNT		1
#define PLATFORM_CLUSTER0_CORE_COUNT	PLATFORM_MAX_CPUS_PER_CLUSTER
#define PLATFORM_CLUSTER1_CORE_COUNT	0

#define PLATFORM_CORE_COUNT		(PLATFORM_CLUSTER0_CORE_COUNT + \
					 PLATFORM_CLUSTER1_CORE_COUNT)

#define WARP7_PRIMARY_CPU		0

#define PLAT_NUM_PWR_DOMAINS		(PLATFORM_CLUSTER_COUNT + \
					PLATFORM_CORE_COUNT)
#define PLAT_MAX_PWR_LVL		MPIDR_AFFLVL1

#define PLAT_MAX_RET_STATE		1
#define PLAT_MAX_OFF_STATE		2

/* Local power state for power domains in Run state. */
#define PLAT_LOCAL_STATE_RUN		0

/* Local power state for retention. Valid only for CPU power domains */
#define PLAT_LOCAL_STATE_RET		1

/*
 * Local power state for OFF/power-down. Valid for CPU and cluster power
 * domains.
 */
#define PLAT_LOCAL_STATE_OFF		2

/*
 * Macros used to parse state information from State-ID if it is using the
 * recommended encoding for State-ID.
 */
#define PLAT_LOCAL_PSTATE_WIDTH		4
#define PLAT_LOCAL_PSTATE_MASK		((1 << PLAT_LOCAL_PSTATE_WIDTH) - 1)

/*
 * Some data must be aligned on the biggest cache line size in the platform.
 * This is known only to the platform as it might have a combination of
 * integrated and external caches.
 * i.MX7 has a 32 byte cacheline size
 * i.MX 7Solo Applications Processor Reference Manual, Rev. 0.1, 08/2016 pg 244
 */
#define CACHE_WRITEBACK_SHIFT		4
#define CACHE_WRITEBACK_GRANULE		(1 << CACHE_WRITEBACK_SHIFT)

/*
 * Partition memory into secure BootROM, OCRAM_S, non-secure DRAM, secure DRAM
 */
#define BOOT_ROM_BASE			0x00000000
#define BOOT_ROM_SIZE			0x00020000

#define OCRAM_S_BASE			0x00180000
#define OCRAM_S_SIZE			0x00008000

/* Controller maps 2GB, board contains 512 MB. 0x80000000 - 0xa0000000 */
#define DRAM_BASE			0x80000000
#define DRAM_SIZE			0x20000000
#define DRAM_LIMIT			(DRAM_BASE + DRAM_SIZE)

/* Place OPTEE at minus 32 MB from the end of memory. 0x9e000000 - 0xa0000000 */
#define WARP7_OPTEE_SIZE		0x02000000
#define WARP7_OPTEE_BASE		(DRAM_LIMIT - WARP7_OPTEE_SIZE)
#define WARP7_OPTEE_LIMIT		(WARP7_OPTEE_BASE + WARP7_OPTEE_SIZE)

/* Place ATF directly beneath OPTEE. 0x9df00000 - 0x9e000000 */
#define BL2_RAM_SIZE			0x00100000
#define BL2_RAM_BASE			(WARP7_OPTEE_BASE - BL2_RAM_SIZE)
#define BL2_RAM_LIMIT			(BL2_RAM_BASE + BL2_RAM_SIZE)

/* Optional Mailbox. Only relevant on i.MX7D. 0x9deff000 - 0x9df00000*/
#define SHARED_RAM_SIZE			0x00001000
#define SHARED_RAM_BASE			(BL2_RAM_BASE - SHARED_RAM_SIZE)
#define SHARED_RAM_LIMIT		(SHARED_RAM_BASE + SHARED_RAM_SIZE)

/* Define the absolute location of u-boot 0x87800000 - 0x87900000 */
#define WARP7_UBOOT_SIZE		0x00100000
#define WARP7_UBOOT_BASE		(DRAM_BASE + 0x7800000)
#define WARP7_UBOOT_LIMIT		(WARP7_UBOOT_BASE + WARP7_UBOOT_SIZE)

/* Define FIP image absolute location 0x80000000 - 0x80100000 */
#define WARP7_FIP_SIZE			0x00100000
#define WARP7_FIP_BASE			(DRAM_BASE)
#define WARP7_FIP_LIMIT			(WARP7_FIP_BASE + WARP7_FIP_SIZE)

/* Define FIP image location at 1MB offset */
#define WARP7_FIP_MMC_BASE		(1024 * 1024)

/* Define the absolute location of DTB 0x83000000 - 0x83100000 */
#define WARP7_DTB_SIZE			0x00100000
#define WARP7_DTB_BASE			(DRAM_BASE + 0x03000000)
#define WARP7_DTB_LIMIT			(WARP7_DTB_BASE + WARP7_DTB_SIZE)

/* Define the absolute location of DTB Overlay 0x83100000 - 0x83101000 */
#define WARP7_DTB_OVERLAY_SIZE		0x00001000
#define WARP7_DTB_OVERLAY_BASE		WARP7_DTB_LIMIT
#define WARP7_DTB_OVERLAY_LIMIT		(WARP7_DTB_OVERLAY_BASE + \
					 WARP7_DTB_OVERLAY_SIZE)

/*
 * BL2 specific defines.
 *
 * Put BL2 just below BL3-1. BL2_BASE is calculated using the current BL2 debug
 * size plus a little space for growth.
 */
#define BL2_BASE		BL2_RAM_BASE
#define BL2_LIMIT		(BL2_RAM_BASE + BL2_RAM_SIZE)

/*
 * BL3-2/OPTEE
 */
# define BL32_BASE		WARP7_OPTEE_BASE
# define BL32_LIMIT		(WARP7_OPTEE_BASE + WARP7_OPTEE_SIZE)

/*
 * BL3-3/U-BOOT
 */
#define BL33_BASE		WARP7_UBOOT_BASE
#define BL33_LIMIT		(WARP7_UBOOT_BASE + WARP7_UBOOT_SIZE)

/*
 * ATF's view of memory
 *
 * 0xa0000000 +-----------------+
 *            |       DDR       | BL32/OPTEE
 * 0x9e000000 +-----------------+
 *            |       DDR       | BL23 ATF
 * 0x9df00000 +-----------------+
 *            |       DDR       | Shared MBOX RAM
 * 0x9de00000 +-----------------+
 *            |       DDR       | Unallocated
 * 0x87900000 +-----------------+
 *            |       DDR       | BL33/U-BOOT
 * 0x87800000 +-----------------+
 *            |       DDR       | Unallocated
 * 0x83101000 +-----------------+
 *            |       DDR       | DTB Overlay
 * 0x83100000 +-----------------+
 *            |       DDR       | DTB
 * 0x83000000 +-----------------+
 *            |       DDR       | Unallocated
 * 0x80100000 +-----------------+
 *            |       DDR       | FIP
 * 0x80000000 +-----------------+
 *            |     SOC I/0     |
 * 0x00a00000 +-----------------+
 *            |      OCRAM      | Not used
 * 0x00900000 +-----------------+
 *            |     SOC I/0     |
 * 0x00188000 +-----------------+
 *            |     OCRAM_S     | Not used
 * 0x00180000 +-----------------+
 *            |     SOC I/0     |
 * 0x00020000 +-----------------+
 *            |     BootROM     | BL1
 * 0x00000000 +-----------------+
 */

#define PLAT_PHY_ADDR_SPACE_SIZE	(1ULL << 32)
#define PLAT_VIRT_ADDR_SPACE_SIZE	(1ULL << 32)
#define MAX_MMAP_REGIONS		10
#define MAX_XLAT_TABLES			6
#define MAX_IO_DEVICES			2
#define MAX_IO_HANDLES			3
#define MAX_IO_BLOCK_DEVICES		1U

/* UART defines */
#if PLAT_WARP7_UART == 1
#define PLAT_WARP7_UART_BASE		MXC_UART1_BASE
#elif PLAT_WARP7_UART == 6
#define IMX_UART_DTE
#define PLAT_WARP7_UART_BASE		MXC_UART6_BASE
#else
#error "define PLAT_WARP7_UART=1 or PLAT_WARP7_UART=6"
#endif

#define PLAT_WARP7_BOOT_UART_BASE	PLAT_WARP7_UART_BASE
#define PLAT_WARP7_BOOT_UART_CLK_IN_HZ	24000000
#define PLAT_WARP7_CONSOLE_BAUDRATE	115200

/* MMC defines */
#ifndef PLAT_WARP7_SD
#define PLAT_WARP7_SD 3
#endif

#if PLAT_WARP7_SD == 1
#define PLAT_WARP7_BOOT_MMC_BASE	USDHC1_BASE
#endif /* PLAT_WARP7_SD == 1 */

#if PLAT_WARP7_SD == 2
#define PLAT_WARP7_BOOT_MMC_BASE	USDHC2_BASE
#endif /* PLAT_WARP7_SD == 2 */

#if PLAT_WARP7_SD == 3
#define PLAT_WARP7_BOOT_MMC_BASE	USDHC3_BASE
#endif /* PLAT_WARP7_SD == 3 */

/*
 * GIC related constants
 */
#define GICD_BASE			0x31001000
#define GICC_BASE			0x31002000
#define GICR_BASE			0

/*
 * System counter
 */
#define SYS_COUNTER_FREQ_IN_TICKS	8000000		/* 8 MHz */

#endif /* PLATFORM_DEF_H */
