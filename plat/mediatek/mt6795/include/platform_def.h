/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __PLATFORM_DEF_H__
#define __PLATFORM_DEF_H__

#define PLAT_PRIMARY_CPU  0x0

/* Special value used to verify platform parameters from BL2 to BL3-1 */
#define MT_BL31_PLAT_PARAM_VAL  0x0f1e2d3c4b5a6978ULL

#define IO_PHYS             (0x10000000)
#define INFRACFG_AO_BASE    (IO_PHYS + 0x1000)
#define MCUCFG_BASE         (IO_PHYS + 0x200000)
#define PERI_BASE           (IO_PHYS + 0x1000000)


#define GPIO_BASE           (IO_PHYS + 0x370000)
#define SPM_BASE            (IO_PHYS + 0x6000)
#define RGU_BASE            (MCUCFG_BASE + 0x11000)
#define PMIC_WRAP_BASE      (IO_PHYS + 0x10000)

#define TRNG_base           (MCUCFG_BASE + 0x230000)
#define MT_GIC_BASE         (0x10220000)
#define MCU_SYS_SIZE        (0x700000)
#define PLAT_MT_CCI_BASE    (IO_PHYS + 0x390000)

/* Aggregate of all devices in the first GB */
#define MTK_DEV_RNG0_BASE   IO_PHYS
#define MTK_DEV_RNG0_SIZE   0x400000
#define MTK_DEV_RNG1_BASE   (PERI_BASE)
#define MTK_DEV_RNG1_SIZE   0x4000000

/*******************************************************************************
 * UART related constants
 ******************************************************************************/
#define UART0_BASE (PERI_BASE + 0x2000)

#define UART_BAUDRATE   (921600)
#define UART_CLOCK (26000000)

/*******************************************************************************
 * System counter frequency related constants
 ******************************************************************************/
#define SYS_COUNTER_FREQ_IN_TICKS	13000000
#define SYS_COUNTER_FREQ_IN_MHZ		(SYS_COUNTER_FREQ_IN_TICKS/1000000)

/*******************************************************************************
 * GIC-400 & interrupt handling related constants
 ******************************************************************************/

/* Base MTK_platform compatible GIC memory map */
#define BASE_GICD_BASE      (MT_GIC_BASE+0x1000)
#define BASE_GICC_BASE      (MT_GIC_BASE + 0x2000)
#define BASE_GICR_BASE      (MT_GIC_BASE + 0x200000)
#define BASE_GICH_BASE      (MT_GIC_BASE + 0x4000)
#define BASE_GICV_BASE      (MT_GIC_BASE + 0x6000)

#define INT_POL_CTL0        0x10200620
#define GIC_PRIVATE_SIGNALS (32)

/*******************************************************************************
 * CCI-400 related constants
 ******************************************************************************/
#define PLAT_MT_CCI_CLUSTER0_SL_IFACE_IX  4
#define PLAT_MT_CCI_CLUSTER1_SL_IFACE_IX  3

/*******************************************************************************
 * WDT Registers
 ******************************************************************************/
#define MTK_WDT_BASE                        (RGU_BASE)
#define MTK_WDT_SIZE                        (0x1000)
#define MTK_WDT_MODE                        (MTK_WDT_BASE+0x0000)
#define MTK_WDT_LENGTH                      (MTK_WDT_BASE+0x0004)
#define MTK_WDT_RESTART                     (MTK_WDT_BASE+0x0008)
#define MTK_WDT_STATUS                      (MTK_WDT_BASE+0x000C)
#define MTK_WDT_INTERVAL                    (MTK_WDT_BASE+0x0010)
#define MTK_WDT_SWRST                       (MTK_WDT_BASE+0x0014)
#define MTK_WDT_SWSYSRST                    (MTK_WDT_BASE+0x0018)
#define MTK_WDT_NONRST_REG                  (MTK_WDT_BASE+0x0020)
#define MTK_WDT_NONRST_REG2                 (MTK_WDT_BASE+0x0024)
#define MTK_WDT_REQ_MODE                    (MTK_WDT_BASE+0x0030)
#define MTK_WDT_REQ_IRQ_EN                  (MTK_WDT_BASE+0x0034)
#define MTK_WDT_DEBUG_CTL                   (MTK_WDT_BASE+0x0040)

/*WDT_STATUS*/
#define MTK_WDT_STATUS_HWWDT_RST            (0x80000000)
#define MTK_WDT_STATUS_SWWDT_RST            (0x40000000)
#define MTK_WDT_STATUS_IRQWDT_RST           (0x20000000)
#define MTK_WDT_STATUS_DEBUGWDT_RST         (0x00080000)
#define MTK_WDT_STATUS_SPMWDT_RST           (0x0002)
#define MTK_WDT_STATUS_SPM_THERMAL_RST      (0x0001)
#define MTK_WDT_STATUS_THERMAL_DIRECT_RST   (1<<18)
#define MTK_WDT_STATUS_SECURITY_RST         (1<<28)

#define MTK_WDT_MODE_DUAL_MODE              0x0040
#define MTK_WDT_MODE_IRQ                    0x0008
#define MTK_WDT_MODE_KEY                    0x22000000
#define MTK_WDT_MODE_EXTEN                  0x0004
#define MTK_WDT_SWRST_KEY                   0x1209
#define MTK_WDT_RESTART_KEY                 (0x1971)

/* FIQ platform related define */
#define MT_IRQ_SEC_SGI_0  8
#define MT_IRQ_SEC_SGI_1  9
#define MT_IRQ_SEC_SGI_2  10
#define MT_IRQ_SEC_SGI_3  11
#define MT_IRQ_SEC_SGI_4  12
#define MT_IRQ_SEC_SGI_5  13
#define MT_IRQ_SEC_SGI_6  14
#define MT_IRQ_SEC_SGI_7  15

#define FIQ_SMP_CALL_SGI  MT_IRQ_SEC_SGI_5

#define PLAT_ARM_G0_IRQS	FIQ_SMP_CALL_SGI

#define DEBUG_XLAT_TABLE 0

/*******************************************************************************
 * Platform binary types for linking
 ******************************************************************************/
#define PLATFORM_LINKER_FORMAT    "elf64-littleaarch64"
#define PLATFORM_LINKER_ARCH      aarch64

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

#define FIRMWARE_WELCOME_STR    "Booting Trusted Firmware\n"
#if ENABLE_PLAT_COMPAT
#define PLATFORM_MAX_AFFLVL     MPIDR_AFFLVL2
#else
#define PLAT_MAX_PWR_LVL        2 /* MPIDR_AFFLVL2 */
#endif

#define PLATFORM_CACHE_LINE_SIZE      64
#define PLATFORM_SYSTEM_COUNT         1
#define PLATFORM_CLUSTER_COUNT        2
#define PLATFORM_CLUSTER0_CORE_COUNT  4
#define PLATFORM_CLUSTER1_CORE_COUNT  4
#define PLATFORM_CORE_COUNT   (PLATFORM_CLUSTER1_CORE_COUNT + \
					PLATFORM_CLUSTER0_CORE_COUNT)
#define PLATFORM_MAX_CPUS_PER_CLUSTER 4
#define PLATFORM_NUM_AFFS   (PLATFORM_SYSTEM_COUNT +  \
					PLATFORM_CLUSTER_COUNT + \
					PLATFORM_CORE_COUNT)

/*******************************************************************************
 * Platform memory map related constants
 ******************************************************************************/
/* ATF Argument */
#define ATF_ARG_SIZE      (0x800)

/* TF txet, ro, rw, internal SRAM, Size: release: 80KB, debug: 92KB */
#define TZRAM_BASE        (0x110000)
#if DEBUG
#define TZRAM_SIZE        (0x1C400)
#else
#define TZRAM_SIZE        (0x1C400)
#endif
#define TZRAM2_BASE	   0x00100000
#define TZRAM2_SIZE	   0xDC00
#define TZRAM2_LIMIT		(TZRAM2_BASE + TZRAM2_SIZE)

#define RAM_CONSOLE_BASE  0x0012D000
#define RAM_CONSOLE_SIZE  0x00001000
/*******************************************************************************
 * BL31 specific defines.
 ******************************************************************************/
/*
 * Put BL3-1 at the top of the Trusted SRAM (just below the shared memory, if
 * present). BL31_BASE is calculated using the current BL3-1 debug size plus a
 * little space for growth.
 */
#define BL31_BASE           (TZRAM_BASE + 0x1000)
#define BL31_LIMIT          (TZRAM_BASE + TZRAM_SIZE)
#define BSS1_STACK_LIMIT    (TZRAM_BASE + TZRAM_SIZE)
#define BL31_TZRAM_SIZE     (TZRAM_SIZE - ATF_ARG_SIZE)

/*******************************************************************************
 * Platform specific page table and MMU setup constants
 ******************************************************************************/
#define ADDR_SPACE_SIZE   (1ull << 32)
#define MAX_XLAT_TABLES   7
#define MAX_MMAP_REGIONS  16


/*******************************************************************************
 * CCI-400 related constants
 ******************************************************************************/
#define CCI400_BASE                     0x10390000
#define CCI400_SL_IFACE_CLUSTER0        4
#define CCI400_SL_IFACE_CLUSTER1        3
#define CCI400_SL_IFACE_INDEX(mpidr)  (mpidr & MPIDR_CLUSTER_MASK ? \
					CCI400_SL_IFACE_CLUSTER1 :   \
					CCI400_SL_IFACE_CLUSTER0)
#define CCI_SEC_ACCESS_OFFSET           (0x8)


/*******************************************************************************
 * Declarations and constants to access the mailboxes safely. Each mailbox is
 * aligned on the biggest cache line size in the platform. This is known only
 * to the platform as it might have a combination of integrated and external
 * caches. Such alignment ensures that two maiboxes do not sit on the same cache
 * line at any cache level. They could belong to different cpus/clusters &
 * get written while being protected by different locks causing corruption of
 * a valid mailbox address.
 ******************************************************************************/
#define CACHE_WRITEBACK_SHIFT     6
#define CACHE_WRITEBACK_GRANULE   (1 << CACHE_WRITEBACK_SHIFT)

#define BL32_BASE                 (0x0)

/*
 * Load address of BL3-3 for this platform port
 */
#define LK_SIZE_LIMIT				(0x100000)
#define PLAT_MTK_NS_IMAGE_OFFSET	(0x41E00000)
/* 16KB */
#define ATF_AEE_BUFFER_SIZE         (0x4000)
#define PAGE_SIZE_2MB_MASK          (PAGE_SIZE_2MB - 1)
#define IS_PAGE_2MB_ALIGNED(addr)   (((addr) & PAGE_SIZE_2MB_MASK) == 0)
#define PAGE_SIZE_2MB               (1 << PAGE_SIZE_2MB_SHIFT)
#define PAGE_SIZE_2MB_SHIFT         TWO_MB_SHIFT

#endif /* __PLATFORM_DEF_H__ */
