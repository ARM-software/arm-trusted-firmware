/*
 * Copyright (c) 2015-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __BOARD_CSS_DEF_H__
#define __BOARD_CSS_DEF_H__

#include <common_def.h>
#include <soc_css_def.h>
#include <utils_def.h>
#include <v2m_def.h>

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

#ifndef __ASSEMBLY__

#include <mmio.h>

#define BOARD_CSS_GET_PLAT_TYPE(addr)					\
	((mmio_read_32(addr) & BOARD_CSS_PLAT_ID_REG_ID_MASK)		\
	>> BOARD_CSS_PLAT_ID_REG_ID_SHIFT)

#endif /* __ASSEMBLY__ */


/*
 * Required platform porting definitions common to all ARM CSS-based
 * development platforms
 */

#define PLAT_ARM_DRAM2_SIZE			ULL(0x180000000)

/* UART related constants */
#define PLAT_ARM_BOOT_UART_BASE			SOC_CSS_UART0_BASE
#define PLAT_ARM_BOOT_UART_CLK_IN_HZ		SOC_CSS_UART0_CLK_IN_HZ

#define PLAT_ARM_BL31_RUN_UART_BASE		SOC_CSS_UART1_BASE
#define PLAT_ARM_BL31_RUN_UART_CLK_IN_HZ	SOC_CSS_UART1_CLK_IN_HZ

#define PLAT_ARM_SP_MIN_RUN_UART_BASE		SOC_CSS_UART1_BASE
#define PLAT_ARM_SP_MIN_RUN_UART_CLK_IN_HZ	SOC_CSS_UART1_CLK_IN_HZ

#define PLAT_ARM_CRASH_UART_BASE		PLAT_ARM_BL31_RUN_UART_BASE
#define PLAT_ARM_CRASH_UART_CLK_IN_HZ		PLAT_ARM_BL31_RUN_UART_CLK_IN_HZ

#define PLAT_ARM_TSP_UART_BASE			V2M_IOFPGA_UART0_BASE
#define PLAT_ARM_TSP_UART_CLK_IN_HZ		V2M_IOFPGA_UART0_CLK_IN_HZ


#endif /* __BOARD_CSS_DEF_H__ */

