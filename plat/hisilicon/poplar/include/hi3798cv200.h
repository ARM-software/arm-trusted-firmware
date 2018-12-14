/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef HI3798CV200_H
#define HI3798CV200_H

#include <lib/utils_def.h>

/* PL011 */
#define PL011_UART0_BASE		(0xF8B00000)
#define PL011_BAUDRATE			(115200)
#define PL011_UART0_CLK_IN_HZ		(75000000)

/* Sys Counter */
#define SYS_COUNTER_FREQ_IN_TICKS	(24000000)
#define SYS_COUNTER_FREQ_IN_MHZ		(24)

/* Timer */
#define SEC_TIMER0_BASE			(0xF8008000)
#define TIMER00_LOAD			(SEC_TIMER0_BASE + 0x000)
#define TIMER00_VALUE			(SEC_TIMER0_BASE + 0x004)
#define TIMER00_CONTROL			(SEC_TIMER0_BASE + 0x008)
#define TIMER00_BGLOAD			(SEC_TIMER0_BASE + 0x018)

#define SEC_TIMER2_BASE			(0xF8009000)
#define TIMER20_LOAD			(SEC_TIMER2_BASE + 0x000)
#define TIMER20_VALUE			(SEC_TIMER2_BASE + 0x004)
#define TIMER20_CONTROL			(SEC_TIMER2_BASE + 0x008)
#define TIMER20_BGLOAD			(SEC_TIMER2_BASE + 0x018)

/* GPIO */
#define	GPIO_MAX			(13)
#define	GPIO_BASE(x)			(x != 5 ?			\
					0xf820000 + x * 0x1000 : 0xf8004000)

/* SCTL */
#define REG_BASE_SCTL			(0xF8000000)
#define REG_SC_GEN12			(0x00B0)

/* CRG */
#define REG_BASE_CRG			(0xF8A22000)
#define REG_CPU_LP			(0x48)
#define REG_CPU_RST			(0x50)
#define REG_PERI_CRG39			(0x9C)
#define REG_PERI_CRG40			(0xA0)

/* MCI */
#define REG_BASE_MCI			(0xF9830000)
#define MCI_CDETECT			(0x50)
#define MCI_VERID			(0x6C)
#define MCI_VERID_VALUE			(0x5342250A)
#define MCI_VERID_VALUE2		(0x5342270A)

/* EMMC */
#define REG_EMMC_PERI_CRG		REG_PERI_CRG40
#define REG_SDCARD_PERI_CRG		REG_PERI_CRG39
#define EMMC_CLK_MASK			(0x7 << 8)
#define EMMC_SRST_REQ			(0x1 << 4)
#define EMMC_CKEN			(0x1 << 1)
#define EMMC_BUS_CKEN			(0x1 << 0)
#define EMMC_CLK_100M			(0 << 8)
#define EMMC_CLK_50M			(1 << 8)
#define EMMC_CLK_25M			(2 << 8)

#define EMMC_DESC_SIZE			U(0x00100000) /* 1MB */
#define EMMC_INIT_PARAMS(base)				\
	{	.bus_width = MMC_BUS_WIDTH_8,		\
		.clk_rate = 25 * 1000 * 1000,		\
		.desc_base = (base),	\
		.desc_size = EMMC_DESC_SIZE,		\
		.flags =  MMC_FLAG_CMD23,		\
		.reg_base = REG_BASE_MCI,		\
	}

/* GIC-400 */
#define GICD_BASE			(0xF1001000)
#define GICC_BASE			(0xF1002000)
#define GICR_BASE			(0xF1000000)

/* FIQ platform related define */
#define HISI_IRQ_SEC_SGI_0		8
#define HISI_IRQ_SEC_SGI_1		9
#define HISI_IRQ_SEC_SGI_2		10
#define HISI_IRQ_SEC_SGI_3		11
#define HISI_IRQ_SEC_SGI_4		12
#define HISI_IRQ_SEC_SGI_5		13
#define HISI_IRQ_SEC_SGI_6		14
#define HISI_IRQ_SEC_SGI_7		15
#define HISI_IRQ_SEC_PPI_0		29
#define HISI_IRQ_SEC_TIMER0		60
#define HISI_IRQ_SEC_TIMER1		50
#define HISI_IRQ_SEC_TIMER2		52
#define HISI_IRQ_SEC_TIMER3		88
#define HISI_IRQ_SEC_AXI		110

/* Watchdog */
#define HISI_WDG0_BASE			(0xF8A2C000)

#define HISI_TZPC_BASE			(0xF8A80000)
#define HISI_TZPC_SEC_ATTR_CTRL		(HISI_TZPC_BASE + 0x10)

#endif /* HI3798CV200_H */
