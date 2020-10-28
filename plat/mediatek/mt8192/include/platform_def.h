/*
 * Copyright (c) 2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H


#define PLAT_PRIMARY_CPU   0x0

#define MT_GIC_BASE        0x0c000000
#define PLAT_MT_CCI_BASE   0x0c500000
#define MCUCFG_BASE        0x0c530000

#define IO_PHYS            0x10000000

/* Aggregate of all devices for MMU mapping */
#define MTK_DEV_RNG0_BASE    IO_PHYS
#define MTK_DEV_RNG0_SIZE    0x10000000
#define MTK_DEV_RNG1_BASE    (IO_PHYS + 0x10000000)
#define MTK_DEV_RNG1_SIZE    0x10000000
#define MTK_DEV_RNG2_BASE    0x0c000000
#define MTK_DEV_RNG2_SIZE    0x600000

#define GPIO_BASE        (IO_PHYS + 0x00005000)
#define IOCFG_RM_BASE    (IO_PHYS + 0x01C20000)
#define IOCFG_BM_BASE    (IO_PHYS + 0x01D10000)
#define IOCFG_BL_BASE    (IO_PHYS + 0x01D30000)
#define IOCFG_BR_BASE    (IO_PHYS + 0x01D40000)
#define IOCFG_LM_BASE    (IO_PHYS + 0x01E20000)
#define IOCFG_LB_BASE    (IO_PHYS + 0x01E70000)
#define IOCFG_RT_BASE    (IO_PHYS + 0x01EA0000)
#define IOCFG_LT_BASE    (IO_PHYS + 0x01F20000)
#define IOCFG_TL_BASE    (IO_PHYS + 0x01F30000)
/*******************************************************************************
 * UART related constants
 ******************************************************************************/
#define UART0_BASE    (IO_PHYS + 0x01002000)
#define UART1_BASE    (IO_PHYS + 0x01003000)

#define UART_BAUDRATE 115200

/*******************************************************************************
 * System counter frequency related constants
 ******************************************************************************/
#define SYS_COUNTER_FREQ_IN_TICKS    13000000
#define SYS_COUNTER_FREQ_IN_MHZ      13

/*******************************************************************************
 * GIC-400 & interrupt handling related constants
 ******************************************************************************/

/* Base MTK_platform compatible GIC memory map */
#define BASE_GICD_BASE        MT_GIC_BASE
#define MT_GIC_RDIST_BASE     (MT_GIC_BASE + 0x40000)

/*******************************************************************************
 * Platform binary types for linking
 ******************************************************************************/
#define PLATFORM_LINKER_FORMAT      "elf64-littleaarch64"
#define PLATFORM_LINKER_ARCH        aarch64

/*******************************************************************************
 * Generic platform constants
 ******************************************************************************/
#define PLATFORM_STACK_SIZE    0x800

#define PLAT_MAX_PWR_LVL        U(2)
#define PLAT_MAX_RET_STATE      U(1)
#define PLAT_MAX_OFF_STATE      U(2)

#define PLATFORM_SYSTEM_COUNT           U(1)
#define PLATFORM_CLUSTER_COUNT          U(1)
#define PLATFORM_CLUSTER0_CORE_COUNT    U(8)
#define PLATFORM_CORE_COUNT             (PLATFORM_CLUSTER0_CORE_COUNT)
#define PLATFORM_MAX_CPUS_PER_CLUSTER   U(8)

#define SOC_CHIP_ID			U(0x8192)

/*******************************************************************************
 * Platform memory map related constants
 ******************************************************************************/
#define TZRAM_BASE          0x54600000
#define TZRAM_SIZE          0x00030000

/*******************************************************************************
 * BL31 specific defines.
 ******************************************************************************/
/*
 * Put BL31 at the top of the Trusted SRAM (just below the shared memory, if
 * present). BL31_BASE is calculated using the current BL31 debug size plus a
 * little space for growth.
 */
#define BL31_BASE       (TZRAM_BASE + 0x1000)
#define BL31_LIMIT      (TZRAM_BASE + TZRAM_SIZE)

/*******************************************************************************
 * Platform specific page table and MMU setup constants
 ******************************************************************************/
#define PLAT_PHY_ADDR_SPACE_SIZE    (1ULL << 32)
#define PLAT_VIRT_ADDR_SPACE_SIZE   (1ULL << 32)
#define MAX_XLAT_TABLES             16
#define MAX_MMAP_REGIONS            16

/*******************************************************************************
 * Declarations and constants to access the mailboxes safely. Each mailbox is
 * aligned on the biggest cache line size in the platform. This is known only
 * to the platform as it might have a combination of integrated and external
 * caches. Such alignment ensures that two maiboxes do not sit on the same cache
 * line at any cache level. They could belong to different cpus/clusters &
 * get written while being protected by different locks causing corruption of
 * a valid mailbox address.
 ******************************************************************************/
#define CACHE_WRITEBACK_SHIFT    6
#define CACHE_WRITEBACK_GRANULE  (1 << CACHE_WRITEBACK_SHIFT)
#endif /* PLATFORM_DEF_H */
