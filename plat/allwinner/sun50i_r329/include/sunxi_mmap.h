/*
 * Copyright (c) 2017-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SUNXI_MMAP_H
#define SUNXI_MMAP_H

/* Memory regions */
#define SUNXI_ROM_BASE			0x00000000
#define SUNXI_ROM_SIZE			0x00010000
/*
 * In fact all SRAM from 0x100000 is SRAM A2. However as it's too big for
 * firmware, and the user manual gives a tip on a 2*64K/27*64K partition,
 * only use the first 2*64K for firmwares now, with the SPL using the first
 * 64K and BL3-1 using the second one.
 *
 * Only the used 2*64K SRAM is defined here, to prevent a gaint translation
 * table to be generated.
 */
#define SUNXI_SRAM_BASE			0x00100000
#define SUNXI_SRAM_SIZE			0x00020000
#define SUNXI_SRAM_A1_BASE		0x00100000
#define SUNXI_SRAM_A1_SIZE		0x00010000
#define SUNXI_SRAM_A2_BASE		0x00110000
#define SUNXI_SRAM_A2_BL31_OFFSET	0x00000000
#define SUNXI_SRAM_A2_SIZE		0x00010000
#define SUNXI_DEV_BASE			0x01000000
#define SUNXI_DEV_SIZE			0x09000000
#define SUNXI_DRAM_BASE			0x40000000
#define SUNXI_DRAM_VIRT_BASE		0x0a000000

/* Memory-mapped devices */
#define SUNXI_WDOG_BASE			0x020000a0
#define SUNXI_R_WDOG_BASE		SUNXI_WDOG_BASE
#define SUNXI_PIO_BASE			0x02000400
#define SUNXI_SPC_BASE			0x02000800
#define SUNXI_CCU_BASE			0x02001000
#define SUNXI_UART0_BASE		0x02500000
#define SUNXI_SYSCON_BASE		0x03000000
#define SUNXI_DMA_BASE			0x03002000
#define SUNXI_SID_BASE			0x03006000
#define SUNXI_GICD_BASE			0x03021000
#define SUNXI_GICC_BASE			0x03022000
#define SUNXI_SPI0_BASE			0x04025000
#define SUNXI_R_CPUCFG_BASE		0x07000400
#define SUNXI_R_PRCM_BASE		0x07010000
#define SUNXI_R_PIO_BASE		0x07022000
#define SUNXI_R_UART_BASE		0x07080000
#define SUNXI_R_I2C_BASE		0x07081400
#define SUNXI_CPUCFG_BASE		0x08100000
#define SUNXI_C0_CPUXCFG_BASE		0x09010000

#endif /* SUNXI_MMAP_H */
