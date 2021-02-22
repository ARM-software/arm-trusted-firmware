/*
 * Copyright (c) 2015-2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BOARD_CSS_DEF_H
#define BOARD_CSS_DEF_H

#include <lib/utils_def.h>
#include <plat/arm/board/common/v2m_def.h>
#include <plat/arm/soc/common/soc_css_def.h>
#include <plat/common/common_def.h>

/*
 * Definitions common to all ARM CSS-based development platforms
 */

/* Platform ID address */
#define BOARD_CSS_PLAT_ID_REG_ADDR		0x7ffe00e0

/* Platform ID related accessors */
#define BOARD_CSS_PLAT_ID_REG_ID_MASK		0x0f
#define BOARD_CSS_PLAT_ID_REG_ID_SHIFT		0x0
#define BOARD_CSS_PLAT_ID_REG_VERSION_MASK	0xf00
#define BOARD_CSS_PLAT_ID_REG_VERSION_SHIFT	0x8
#define BOARD_CSS_PLAT_TYPE_RTL			0x00
#define BOARD_CSS_PLAT_TYPE_FPGA		0x01
#define BOARD_CSS_PLAT_TYPE_EMULATOR		0x02
#define BOARD_CSS_PLAT_TYPE_FVP			0x03

#ifndef __ASSEMBLER__

#include <lib/mmio.h>

#define BOARD_CSS_GET_PLAT_TYPE(addr)					\
	((mmio_read_32(addr) & BOARD_CSS_PLAT_ID_REG_ID_MASK)		\
	>> BOARD_CSS_PLAT_ID_REG_ID_SHIFT)

#endif /* __ASSEMBLER__ */


#define MAX_IO_DEVICES			3
#define MAX_IO_HANDLES			4

/* Reserve the last block of flash for PSCI MEM PROTECT flag */
#define PLAT_ARM_FLASH_IMAGE_BASE	V2M_FLASH0_BASE
#define PLAT_ARM_FLASH_IMAGE_MAX_SIZE	(V2M_FLASH0_SIZE - V2M_FLASH_BLOCK_SIZE)

#if ARM_GPT_SUPPORT
/*
 * Offset of the FIP in the GPT image. BL1 component uses this option
 * as it does not load the partition table to get the FIP base
 * address. At sector 34 by default (i.e. after reserved sectors 0-33)
 * Offset = 34 * 512(sector size) = 17408 i.e. 0x4400
 */
#define PLAT_ARM_FIP_OFFSET_IN_GPT		0x4400
#endif /* ARM_GPT_SUPPORT */

#define PLAT_ARM_NVM_BASE		V2M_FLASH0_BASE
#define PLAT_ARM_NVM_SIZE		(V2M_FLASH0_SIZE - V2M_FLASH_BLOCK_SIZE)

/* UART related constants */
#define PLAT_ARM_BOOT_UART_BASE			SOC_CSS_UART0_BASE
#define PLAT_ARM_BOOT_UART_CLK_IN_HZ		SOC_CSS_UART0_CLK_IN_HZ

#define PLAT_ARM_RUN_UART_BASE		SOC_CSS_UART1_BASE
#define PLAT_ARM_RUN_UART_CLK_IN_HZ	SOC_CSS_UART1_CLK_IN_HZ

#define PLAT_ARM_SP_MIN_RUN_UART_BASE		SOC_CSS_UART1_BASE
#define PLAT_ARM_SP_MIN_RUN_UART_CLK_IN_HZ	SOC_CSS_UART1_CLK_IN_HZ

#define PLAT_ARM_CRASH_UART_BASE		PLAT_ARM_RUN_UART_BASE
#define PLAT_ARM_CRASH_UART_CLK_IN_HZ		PLAT_ARM_RUN_UART_CLK_IN_HZ

#define PLAT_ARM_TSP_UART_BASE			V2M_IOFPGA_UART0_BASE
#define PLAT_ARM_TSP_UART_CLK_IN_HZ		V2M_IOFPGA_UART0_CLK_IN_HZ

#endif /* BOARD_CSS_DEF_H */
