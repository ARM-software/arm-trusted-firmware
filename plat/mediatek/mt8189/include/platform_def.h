/*
 * Copyright (c) 2025, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#include <arch_def.h>

#define PLAT_PRIMARY_CPU	(0x0)

#define MT_GIC_BASE		(0x0C000000)
#define MCUCFG_BASE		(0x0C530000)
#define MCUCFG_REG_SIZE		(0x10000)
#define IO_PHYS			(0x10000000)

/* Aggregate of all devices for MMU mapping */
#define MTK_DEV_RNG0_BASE	(MT_GIC_BASE)
#define MTK_DEV_RNG0_SIZE	(0x600000)
#define MTK_DEV_RNG1_BASE	(IO_PHYS)
#define MTK_DEV_RNG1_SIZE	(0x10000000)

#define TOPCKGEN_BASE		(IO_PHYS)

/*******************************************************************************
 * GPIO related constants
 ******************************************************************************/
#define GPIO_BASE		(IO_PHYS + 0x00005000)
#define IOCFG_LM_BASE		(IO_PHYS + 0x01B50000)
#define IOCFG_RB0_BASE		(IO_PHYS + 0x01C50000)
#define IOCFG_RB1_BASE		(IO_PHYS + 0x01C60000)
#define IOCFG_BM0_BASE		(IO_PHYS + 0x01D20000)
#define IOCFG_BM1_BASE		(IO_PHYS + 0x01D30000)
#define IOCFG_BM2_BASE		(IO_PHYS + 0x01D40000)
#define IOCFG_LT0_BASE		(IO_PHYS + 0x01E20000)
#define IOCFG_LT1_BASE		(IO_PHYS + 0x01E30000)
#define IOCFG_RT_BASE		(IO_PHYS + 0x01F20000)

/*******************************************************************************
 * UART related constants
 ******************************************************************************/
#define UART0_BASE	(IO_PHYS + 0x01002000)
#define UART_BAUDRATE	(115200)

/*******************************************************************************
 * CIRQ related constants
 ******************************************************************************/
#define SYS_CIRQ_BASE		(IO_PHYS + 204000)
#define MD_WDT_IRQ_BIT_ID	(519)
#define CIRQ_REG_NUM		(19)
#define CIRQ_SPI_START		(128)
#define CIRQ_IRQ_NUM		(598)

/*******************************************************************************
 * System counter frequency related constants
 ******************************************************************************/
#define SYS_COUNTER_FREQ_IN_HZ	(13000000)
#define SYS_COUNTER_FREQ_IN_MHZ	(13)

/*******************************************************************************
 * Platform binary types for linking
 ******************************************************************************/
#define PLATFORM_LINKER_FORMAT		"elf64-littleaarch64"
#define PLATFORM_LINKER_ARCH		aarch64

/*******************************************************************************
 * Generic platform constants
 ******************************************************************************/
#define PLATFORM_STACK_SIZE		(0x800)
#define FIRMWARE_WELCOME_STR		"Booting Trusted Firmware\n"
#define SOC_CHIP_ID			U(0x8189)

/*******************************************************************************
 * Platform memory map related constants
 ******************************************************************************/
#define TZRAM_BASE			(0x54600000)
#define TZRAM_SIZE			(0x00200000)

/*******************************************************************************
 * BL31 specific defines.
 ******************************************************************************/
/*
 * Put BL3-1 at the top of the Trusted SRAM (just below the shared memory, if
 * present). BL31_BASE is calculated using the current BL3-1 debug size plus a
 * little space for growth.
 */
#define BL31_BASE			(TZRAM_BASE + 0x1000)
#define BL31_LIMIT			(TZRAM_BASE + TZRAM_SIZE)

/*******************************************************************************
 * GIC-600 & interrupt handling related constants
 ******************************************************************************/
/* Base MTK_platform compatible GIC memory map */
#define BASE_GICD_BASE		(MT_GIC_BASE)
#define MT_GIC_RDIST_BASE	(MT_GIC_BASE + 0x40000)
#define DEV_IRQ_ID		300

#define PLAT_MTK_G1S_IRQ_PROPS(grp) \
	INTR_PROP_DESC(DEV_IRQ_ID, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_LEVEL)

/*******************************************************************************
 * Platform specific page table and MMU setup constants
 ******************************************************************************/
#define PLAT_PHY_ADDR_SPACE_SIZE	(1ULL << 32)
#define PLAT_VIRT_ADDR_SPACE_SIZE	(1ULL << 32)
#define MAX_XLAT_TABLES			(16)
#define MAX_MMAP_REGIONS		(16)

/*******************************************************************************
 * SYSTIMER related definitions
 ******************************************************************************/
#define SYSTIMER_BASE		(IO_PHYS + 0x0CC10000)

#endif /* PLATFORM_DEF_H */
