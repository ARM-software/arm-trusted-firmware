/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef STM32MP1_DEF_H
#define STM32MP1_DEF_H

#include <common/tbbr/tbbr_img_def.h>
#include <lib/utils_def.h>
#include <lib/xlat_tables/xlat_tables_defs.h>

/*******************************************************************************
 * STM32MP1 memory map related constants
 ******************************************************************************/

#define STM32MP1_SRAM_BASE		U(0x2FFC0000)
#define STM32MP1_SRAM_SIZE		U(0x00040000)

/* DDR configuration */
#define STM32MP1_DDR_BASE		U(0xC0000000)
#define STM32MP1_DDR_SIZE_DFLT		U(0x20000000)	/* 512 MB */
#define STM32MP1_DDR_MAX_SIZE		U(0x40000000)	/* Max 1GB */
#define STM32MP1_DDR_SPEED_DFLT		528

/* DDR power initializations */
#ifndef __ASSEMBLY__
enum ddr_type {
	STM32MP_DDR3,
	STM32MP_LPDDR2,
};
#endif

/* Section used inside TF binaries */
#define STM32MP1_PARAM_LOAD_SIZE	U(0x00002400)	/* 9 Ko for param */
/* 256 Octets reserved for header */
#define STM32MP1_HEADER_SIZE		U(0x00000100)

#define STM32MP1_BINARY_BASE		(STM32MP1_SRAM_BASE +		\
					 STM32MP1_PARAM_LOAD_SIZE +	\
					 STM32MP1_HEADER_SIZE)

#define STM32MP1_BINARY_SIZE		(STM32MP1_SRAM_SIZE -		\
					 (STM32MP1_PARAM_LOAD_SIZE +	\
					  STM32MP1_HEADER_SIZE))

#if STACK_PROTECTOR_ENABLED
#define STM32MP1_BL32_SIZE		U(0x00012000)	/* 72 Ko for BL32 */
#else
#define STM32MP1_BL32_SIZE		U(0x00011000)	/* 68 Ko for BL32 */
#endif

#define STM32MP1_BL32_BASE		(STM32MP1_SRAM_BASE + \
					 STM32MP1_SRAM_SIZE - \
					 STM32MP1_BL32_SIZE)

#if STACK_PROTECTOR_ENABLED
#define STM32MP1_BL2_SIZE		U(0x00015000)	/* 84 Ko for BL2 */
#else
#define STM32MP1_BL2_SIZE		U(0x00013000)	/* 76 Ko for BL2 */
#endif

#define STM32MP1_BL2_BASE		(STM32MP1_BL32_BASE - \
					 STM32MP1_BL2_SIZE)

/* BL2 and BL32/sp_min require 5 tables */
#define MAX_XLAT_TABLES			5

/*
 * MAX_MMAP_REGIONS is usually:
 * BL stm32mp1_mmap size + mmap regions in *_plat_arch_setup
 */
#if defined(IMAGE_BL2)
  #define MAX_MMAP_REGIONS		11
#endif
#if defined(IMAGE_BL32)
  #define MAX_MMAP_REGIONS		6
#endif

/* DTB initialization value */
#define STM32MP1_DTB_SIZE		U(0x00004000)	/* 16Ko for DTB */

#define STM32MP1_DTB_BASE		(STM32MP1_BL2_BASE - \
					 STM32MP1_DTB_SIZE)

#define STM32MP1_BL33_BASE		(STM32MP1_DDR_BASE + U(0x100000))

/*******************************************************************************
 * STM32MP1 device/io map related constants (used for MMU)
 ******************************************************************************/
#define STM32MP1_DEVICE1_BASE		U(0x40000000)
#define STM32MP1_DEVICE1_SIZE		U(0x40000000)

#define STM32MP1_DEVICE2_BASE		U(0x80000000)
#define STM32MP1_DEVICE2_SIZE		U(0x40000000)

/*******************************************************************************
 * STM32MP1 RCC
 ******************************************************************************/
#define RCC_BASE			U(0x50000000)

/*******************************************************************************
 * STM32MP1 PWR
 ******************************************************************************/
#define PWR_BASE			U(0x50001000)

/*******************************************************************************
 * STM32MP1 UART
 ******************************************************************************/
#define USART1_BASE			U(0x5C000000)
#define USART2_BASE			U(0x4000E000)
#define USART3_BASE			U(0x4000F000)
#define UART4_BASE			U(0x40010000)
#define UART5_BASE			U(0x40011000)
#define USART6_BASE			U(0x44003000)
#define UART7_BASE			U(0x40018000)
#define UART8_BASE			U(0x40019000)
#define STM32MP1_DEBUG_USART_BASE	UART4_BASE
#define STM32MP1_UART_BAUDRATE		115200

/*******************************************************************************
 * STM32MP1 GIC-400
 ******************************************************************************/
#define STM32MP1_GICD_BASE		U(0xA0021000)
#define STM32MP1_GICC_BASE		U(0xA0022000)
#define STM32MP1_GICH_BASE		U(0xA0024000)
#define STM32MP1_GICV_BASE		U(0xA0026000)

/*******************************************************************************
 * STM32MP1 TZC (TZ400)
 ******************************************************************************/
#define STM32MP1_TZC_BASE		U(0x5C006000)

#define STM32MP1_TZC_A7_ID		U(0)
#define STM32MP1_TZC_LCD_ID		U(3)
#define STM32MP1_TZC_GPU_ID		U(4)
#define STM32MP1_TZC_MDMA_ID		U(5)
#define STM32MP1_TZC_DMA_ID		U(6)
#define STM32MP1_TZC_USB_HOST_ID	U(7)
#define STM32MP1_TZC_USB_OTG_ID		U(8)
#define STM32MP1_TZC_SDMMC_ID		U(9)
#define STM32MP1_TZC_ETH_ID		U(10)
#define STM32MP1_TZC_DAP_ID		U(15)

#define STM32MP1_MEMORY_NS		0
#define STM32MP1_MEMORY_SECURE		1

#define STM32MP1_FILTER_BIT_ALL		3

/*******************************************************************************
 * STM32MP1 SDMMC
 ******************************************************************************/
#define STM32MP1_SDMMC1_BASE		U(0x58005000)
#define STM32MP1_SDMMC2_BASE		U(0x58007000)
#define STM32MP1_SDMMC3_BASE		U(0x48004000)

#define STM32MP1_MMC_INIT_FREQ			400000		/*400 KHz*/
#define STM32MP1_SD_NORMAL_SPEED_MAX_FREQ	25000000	/*25 MHz*/
#define STM32MP1_SD_HIGH_SPEED_MAX_FREQ		50000000	/*50 MHz*/
#define STM32MP1_EMMC_NORMAL_SPEED_MAX_FREQ	26000000	/*26 MHz*/
#define STM32MP1_EMMC_HIGH_SPEED_MAX_FREQ	52000000	/*52 MHz*/

/*******************************************************************************
 * STM32MP1 TAMP
 ******************************************************************************/
#define TAMP_BASE			U(0x5C00A000)
#define TAMP_BKP_REGISTER_BASE		(TAMP_BASE + U(0x100))

#if !(defined(__LINKER__) || defined(__ASSEMBLY__))
static inline uint32_t tamp_bkpr(uint32_t idx)
{
	return TAMP_BKP_REGISTER_BASE + (idx << 2);
}
#endif

/*******************************************************************************
 * STM32MP1 DDRCTRL
 ******************************************************************************/
#define DDRCTRL_BASE			U(0x5A003000)

/*******************************************************************************
 * STM32MP1 DDRPHYC
 ******************************************************************************/
#define DDRPHYC_BASE			U(0x5A004000)

/*******************************************************************************
 * STM32MP1 I2C4
 ******************************************************************************/
#define I2C4_BASE			U(0x5C002000)

#endif /* STM32MP1_DEF_H */
