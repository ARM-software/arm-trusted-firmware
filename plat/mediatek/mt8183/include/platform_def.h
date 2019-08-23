/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#include <arch.h>
#include <drivers/arm/gic_common.h>

#define PLAT_PRIMARY_CPU   0x0

#define IO_PHYS            0x10000000
#define INFRACFG_AO_BASE   (IO_PHYS + 0x1000)
#define PERI_BASE          (IO_PHYS + 0x3000)
#define GPIO_BASE          (IO_PHYS + 0x5000)
#define SPM_BASE           (IO_PHYS + 0x6000)
#define SLEEP_REG_MD_BASE  (IO_PHYS + 0xf000)
#define RGU_BASE           (IO_PHYS + 0x7000)
#define I2C4_BASE_SE       (IO_PHYS + 0x1008000)
#define I2C2_BASE_SE       (IO_PHYS + 0x1009000)
#define PMIC_WRAP_BASE     (IO_PHYS + 0xd000)
#define MCUCFG_BASE        0x0c530000
#define CFG_SF_CTRL        0x0c510014
#define CFG_SF_INI         0x0c510010
#define EMI_BASE           (IO_PHYS + 0x219000)
#define EMI_MPU_BASE       (IO_PHYS + 0x226000)
#define TRNG_base          (IO_PHYS + 0x20f000)
#define MT_GIC_BASE        0x0c000000
#define PLAT_MT_CCI_BASE   0x0c500000
#define CCI_SIZE           0x00010000
#define EINT_BASE          0x1000b000
#define DVFSRC_BASE        (IO_PHYS + 0x12000)

#define SSPM_CFGREG_BASE   (IO_PHYS + 0x440000)
#define SSPM_MBOX_3_BASE   (IO_PHYS + 0x480000)

#define INFRACFG_AO_BASE   (IO_PHYS + 0x1000)

#define APMIXEDSYS         (IO_PHYS + 0xC000)
#define ARMPLL_LL_CON0     (APMIXEDSYS + 0x200)
#define ARMPLL_L_CON0      (APMIXEDSYS + 0x210)
#define ARMPLL_L_PWR_CON0  (APMIXEDSYS + 0x21c)
#define MAINPLL_CON0       (APMIXEDSYS + 0x220)
#define CCIPLL_CON0        (APMIXEDSYS + 0x290)

#define TOP_CKMUXSEL       (INFRACFG_AO_BASE + 0x0)

#define armpll_mux1_sel_big_mask    (0xf << 4)
#define armpll_mux1_sel_big_ARMSPLL (0x1 << 4)
#define armpll_mux1_sel_sml_mask    (0xf << 8)
#define armpll_mux1_sel_sml_ARMSPLL (0x1 << 8)


/* Aggregate of all devices in the first GB */
#define MTK_DEV_RNG0_BASE    IO_PHYS
#define MTK_DEV_RNG0_SIZE    0x490000
#define MTK_DEV_RNG1_BASE    (IO_PHYS + 0x1000000)
#define MTK_DEV_RNG1_SIZE    0x4000000
#define MTK_DEV_RNG2_BASE    0x0c000000
#define MTK_DEV_RNG2_SIZE    0x600000
#define MT_MCUSYS_SIZE       0x90000
#define RAM_CONSOLE_BASE     0x11d000
#define RAM_CONSOLE_SIZE     0x1000

/*******************************************************************************
 * MSDC
 ******************************************************************************/
#define MSDC0_BASE          (IO_PHYS + 0x01230000)

/*******************************************************************************
 * MCUSYS related constants
 ******************************************************************************/
#define MT_L2_WRITE_ACCESS_RATE  (MCUCFG_BASE + 0x604)
#define MP0_CA7L_CACHE_CONFIG    (MCUCFG_BASE + 0x7f0)
#define MP1_CA7L_CACHE_CONFIG    (MCUCFG_BASE + 0x7f4)
#define EMI_WFIFO                (MCUCFG_BASE + 0x0b5c)

/*******************************************************************************
 * GIC related constants
 ******************************************************************************/
#define MT_POLARITY_LOW     0
#define MT_POLARITY_HIGH    1
#define MT_EDGE_SENSITIVE   1
#define MT_LEVEL_SENSITIVE  0

/*******************************************************************************
 * UART related constants
 ******************************************************************************/
#define UART0_BASE    (IO_PHYS + 0x01002000)
#define UART1_BASE    (IO_PHYS + 0x01003000)

#define UART_BAUDRATE 115200
#define UART_CLOCK    26000000

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
#define BASE_GICC_BASE        (MT_GIC_BASE + 0x400000)
#define MT_GIC_RDIST_BASE     (MT_GIC_BASE + 0x100000)
#define BASE_GICR_BASE        (MT_GIC_BASE + 0x100000)
#define BASE_GICH_BASE        (MT_GIC_BASE + 0x4000)
#define BASE_GICV_BASE        (MT_GIC_BASE + 0x6000)
#define INT_POL_CTL0          (MCUCFG_BASE + 0xa80)
#define SEC_POL_CTL_EN0       (MCUCFG_BASE + 0xa00)
#define GIC_SYNC_DCM          (MCUCFG_BASE + 0x758)
#define GIC_SYNC_DCM_MASK     0x3
#define GIC_SYNC_DCM_ON       0x3
#define GIC_SYNC_DCM_OFF      0x0
#define GIC_PRIVATE_SIGNALS   32

#define PLAT_ARM_GICD_BASE BASE_GICD_BASE
#define PLAT_ARM_GICC_BASE BASE_GICC_BASE

#define PLAT_ARM_G1S_IRQ_PROPS(grp) ( \
INTR_PROP_DESC(MT_IRQ_SEC_SGI_0, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_EDGE), \
INTR_PROP_DESC(MT_IRQ_SEC_SGI_1, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_EDGE), \
INTR_PROP_DESC(MT_IRQ_SEC_SGI_2, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_EDGE), \
INTR_PROP_DESC(MT_IRQ_SEC_SGI_3, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_EDGE), \
INTR_PROP_DESC(MT_IRQ_SEC_SGI_4, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_EDGE), \
INTR_PROP_DESC(MT_IRQ_SEC_SGI_5, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_EDGE), \
INTR_PROP_DESC(MT_IRQ_SEC_SGI_6, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_EDGE), \
INTR_PROP_DESC(MT_IRQ_SEC_SGI_7, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_EDGE)) \

#define PLAT_ARM_G0_IRQ_PROPS(grp)

/*******************************************************************************
 * CCI-400 related constants
 ******************************************************************************/
#define PLAT_MT_CCI_CLUSTER0_SL_IFACE_IX    4
#define PLAT_MT_CCI_CLUSTER1_SL_IFACE_IX    3

/*******************************************************************************
 * WDT Registers
 ******************************************************************************/
#define MTK_WDT_BASE            (IO_PHYS + 0x00007000)
#define MTK_WDT_SIZE            0x1000
#define MTK_WDT_MODE            (MTK_WDT_BASE + 0x0000)
#define MTK_WDT_LENGTH          (MTK_WDT_BASE + 0x0004)
#define MTK_WDT_RESTART         (MTK_WDT_BASE + 0x0008)
#define MTK_WDT_STATUS          (MTK_WDT_BASE + 0x000C)
#define MTK_WDT_INTERVAL        (MTK_WDT_BASE + 0x0010)
#define MTK_WDT_SWRST           (MTK_WDT_BASE + 0x0014)
#define MTK_WDT_SWSYSRST        (MTK_WDT_BASE + 0x0018)
#define MTK_WDT_NONRST_REG      (MTK_WDT_BASE + 0x0020)
#define MTK_WDT_NONRST_REG2     (MTK_WDT_BASE + 0x0024)
#define MTK_WDT_REQ_MODE        (MTK_WDT_BASE + 0x0030)
#define MTK_WDT_REQ_IRQ_EN      (MTK_WDT_BASE + 0x0034)
#define MTK_WDT_EXT_REQ_CON     (MTK_WDT_BASE + 0x0038)
#define MTK_WDT_DEBUG_CTL       (MTK_WDT_BASE + 0x0040)
#define MTK_WDT_LATCH_CTL       (MTK_WDT_BASE + 0x0044)
#define MTK_WDT_DEBUG_CTL2      (MTK_WDT_BASE + 0x00A0)
#define MTK_WDT_COUNTER         (MTK_WDT_BASE + 0x0514)

/* WDT_STATUS */
#define MTK_WDT_STATUS_SPM_THERMAL_RST      (1 << 0)
#define MTK_WDT_STATUS_SPM_RST              (1 << 1)
#define MTK_WDT_STATUS_EINT_RST             (1 << 2)
#define MTK_WDT_STATUS_SYSRST_RST           (1 << 3) /* from PMIC */
#define MTK_WDT_STATUS_DVFSP_RST            (1 << 4)
#define MTK_WDT_STATUS_PMCU_RST             (1 << 16)
#define MTK_WDT_STATUS_MDDBG_RST            (1 << 17)
#define MTK_WDT_STATUS_THERMAL_DIRECT_RST   (1 << 18)
#define MTK_WDT_STATUS_DEBUG_RST            (1 << 19)
#define MTK_WDT_STATUS_SECURITY_RST         (1 << 28)
#define MTK_WDT_STATUS_IRQ_ASSERT           (1 << 29)
#define MTK_WDT_STATUS_SW_WDT_RST           (1 << 30)
#define MTK_WDT_STATUS_HW_WDT_RST           (1U << 31)

/* RGU other related */
#define MTK_WDT_MODE_DUAL_MODE    0x0040
#define MTK_WDT_MODE_IRQ          0x0008
#define MTK_WDT_MODE_KEY          0x22000000
#define MTK_WDT_MODE_EXTEN        0x0004
#define MTK_WDT_SWRST_KEY         0x1209
#define MTK_WDT_RESTART_KEY       0x1971

/*******************************************************************************
 * TRNG Registers
 ******************************************************************************/
#define TRNG_BASE_ADDR       TRNG_base
#define TRNG_BASE_SIZE       0x1000
#define TRNG_CTRL            (TRNG_base + 0x0000)
#define TRNG_TIME            (TRNG_base + 0x0004)
#define TRNG_DATA            (TRNG_base + 0x0008)
#define TRNG_PDN_base        0x10001000
#define TRNG_PDN_BASE_ADDR   TRNG_PDN_BASE_ADDR
#define TRNG_PDN_BASE_SIZE   0x1000
#define TRNG_PDN_SET         (TRNG_PDN_base + 0x0088)
#define TRNG_PDN_CLR         (TRNG_PDN_base + 0x008c)
#define TRNG_PDN_STATUS      (TRNG_PDN_base + 0x0094)
#define TRNG_CTRL_RDY        0x80000000
#define TRNG_CTRL_START      0x00000001
#define TRNG_PDN_VALUE       0x200

/* FIQ platform related define */
#define MT_IRQ_SEC_SGI_0    8
#define MT_IRQ_SEC_SGI_1    9
#define MT_IRQ_SEC_SGI_2    10
#define MT_IRQ_SEC_SGI_3    11
#define MT_IRQ_SEC_SGI_4    12
#define MT_IRQ_SEC_SGI_5    13
#define MT_IRQ_SEC_SGI_6    14
#define MT_IRQ_SEC_SGI_7    15

#define FIQ_SMP_CALL_SGI    13
#define WDT_IRQ_BIT_ID      174
#define ATF_LOG_IRQ_ID      277

#define ATF_AMMS_IRQ_ID     338
#define PCCIF1_IRQ0_BIT_ID  185
#define PCCIF1_IRQ1_BIT_ID  186

#define DEBUG_XLAT_TABLE    0

/*******************************************************************************
 * Platform binary types for linking
 ******************************************************************************/
#define PLATFORM_LINKER_FORMAT      "elf64-littleaarch64"
#define PLATFORM_LINKER_ARCH        aarch64

/*******************************************************************************
 * Generic platform constants
 ******************************************************************************/

/* Size of cacheable stacks */
#if DEBUG_XLAT_TABLE
#define PLATFORM_STACK_SIZE    0x800
#elif IMAGE_BL1
#define PLATFORM_STACK_SIZE    0x440
#elif IMAGE_BL2
#define PLATFORM_STACK_SIZE    0x400
#elif IMAGE_BL31
#define PLATFORM_STACK_SIZE    0x800
#elif IMAGE_BL32
#define PLATFORM_STACK_SIZE    0x440
#endif

#define FIRMWARE_WELCOME_STR    "Booting Trusted Firmware\n"
#define PLAT_MAX_PWR_LVL        U(2)
#define PLAT_MAX_RET_STATE		U(1)
#define PLAT_MAX_OFF_STATE		U(2)

#define PLATFORM_CACHE_LINE_SIZE        64
#define PLATFORM_SYSTEM_COUNT           1
#define PLATFORM_CLUSTER_COUNT          2
#define PLATFORM_CLUSTER0_CORE_COUNT    4
#define PLATFORM_CLUSTER1_CORE_COUNT    4
#define PLATFORM_CORE_COUNT             (PLATFORM_CLUSTER1_CORE_COUNT + \
					 PLATFORM_CLUSTER0_CORE_COUNT)
#define PLATFORM_MAX_CPUS_PER_CLUSTER   4
#define PLATFORM_NUM_AFFS               (PLATFORM_SYSTEM_COUNT + \
					 PLATFORM_CLUSTER_COUNT + \
					 PLATFORM_CORE_COUNT)

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
