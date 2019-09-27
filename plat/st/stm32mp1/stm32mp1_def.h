/*
 * Copyright (c) 2015-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef STM32MP1_DEF_H
#define STM32MP1_DEF_H

#include <common/tbbr/tbbr_img_def.h>
#include <drivers/st/stm32mp1_rcc.h>
#include <dt-bindings/clock/stm32mp1-clks.h>
#include <dt-bindings/reset/stm32mp1-resets.h>
#include <lib/utils_def.h>
#include <lib/xlat_tables/xlat_tables_defs.h>

#ifndef __ASSEMBLER__
#include <drivers/st/bsec.h>
#include <drivers/st/stm32mp1_clk.h>

#include <boot_api.h>
#include <stm32mp_auth.h>
#include <stm32mp_common.h>
#include <stm32mp_dt.h>
#include <stm32mp_shres_helpers.h>
#include <stm32mp1_dbgmcu.h>
#include <stm32mp1_private.h>
#endif

/*******************************************************************************
 * CHIP ID
 ******************************************************************************/
#define STM32MP157C_PART_NB	U(0x05000000)
#define STM32MP157A_PART_NB	U(0x05000001)
#define STM32MP153C_PART_NB	U(0x05000024)
#define STM32MP153A_PART_NB	U(0x05000025)
#define STM32MP151C_PART_NB	U(0x0500002E)
#define STM32MP151A_PART_NB	U(0x0500002F)

#define STM32MP1_REV_B		U(0x2000)

/*******************************************************************************
 * PACKAGE ID
 ******************************************************************************/
#define PKG_AA_LFBGA448		U(4)
#define PKG_AB_LFBGA354		U(3)
#define PKG_AC_TFBGA361		U(2)
#define PKG_AD_TFBGA257		U(1)

/*******************************************************************************
 * STM32MP1 memory map related constants
 ******************************************************************************/
#define STM32MP_ROM_BASE		U(0x00000000)
#define STM32MP_ROM_SIZE		U(0x00020000)

#define STM32MP_SYSRAM_BASE		U(0x2FFC0000)
#define STM32MP_SYSRAM_SIZE		U(0x00040000)

/* DDR configuration */
#define STM32MP_DDR_BASE		U(0xC0000000)
#define STM32MP_DDR_MAX_SIZE		U(0x40000000)	/* Max 1GB */
#ifdef AARCH32_SP_OPTEE
#define STM32MP_DDR_S_SIZE		U(0x01E00000)	/* 30 MB */
#define STM32MP_DDR_SHMEM_SIZE		U(0x00200000)	/* 2 MB */
#endif

/* DDR power initializations */
#ifndef __ASSEMBLER__
enum ddr_type {
	STM32MP_DDR3,
	STM32MP_LPDDR2,
	STM32MP_LPDDR3
};
#endif

/* Section used inside TF binaries */
#define STM32MP_PARAM_LOAD_SIZE		U(0x00002400)	/* 9 KB for param */
/* 256 Octets reserved for header */
#define STM32MP_HEADER_SIZE		U(0x00000100)

#define STM32MP_BINARY_BASE		(STM32MP_SYSRAM_BASE +		\
					 STM32MP_PARAM_LOAD_SIZE +	\
					 STM32MP_HEADER_SIZE)

#define STM32MP_BINARY_SIZE		(STM32MP_SYSRAM_SIZE -		\
					 (STM32MP_PARAM_LOAD_SIZE +	\
					  STM32MP_HEADER_SIZE))

#ifdef AARCH32_SP_OPTEE
#define STM32MP_BL32_SIZE		U(0)

#define STM32MP_OPTEE_BASE		STM32MP_SYSRAM_BASE

#define STM32MP_OPTEE_SIZE		(STM32MP_DTB_BASE -  \
					 STM32MP_OPTEE_BASE)
#else
#if STACK_PROTECTOR_ENABLED
#define STM32MP_BL32_SIZE		U(0x00012000)	/* 72 KB for BL32 */
#else
#define STM32MP_BL32_SIZE		U(0x00011000)	/* 68 KB for BL32 */
#endif
#endif

#define STM32MP_BL32_BASE		(STM32MP_SYSRAM_BASE + \
					 STM32MP_SYSRAM_SIZE - \
					 STM32MP_BL32_SIZE)

#ifdef AARCH32_SP_OPTEE
#if STACK_PROTECTOR_ENABLED
#define STM32MP_BL2_SIZE		U(0x00019000)	/* 100 KB for BL2 */
#else
#define STM32MP_BL2_SIZE		U(0x00017000)	/* 92 KB for BL2 */
#endif
#else
#if STACK_PROTECTOR_ENABLED
#define STM32MP_BL2_SIZE		U(0x00018000)	/* 96 KB for BL2 */
#else
#define STM32MP_BL2_SIZE		U(0x00016000)	/* 88 KB for BL2 */
#endif
#endif

#define STM32MP_BL2_BASE		(STM32MP_BL32_BASE - \
					 STM32MP_BL2_SIZE)

/* BL2 and BL32/sp_min require 4 tables */
#define MAX_XLAT_TABLES			U(4)		/* 16 KB for mapping */

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
#define STM32MP_DTB_SIZE		U(0x00005000)	/* 20 KB for DTB */

#define STM32MP_DTB_BASE		(STM32MP_BL2_BASE - \
					 STM32MP_DTB_SIZE)

#define STM32MP_BL33_BASE		(STM32MP_DDR_BASE + U(0x100000))

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
 * STM32MP1 GPIO
 ******************************************************************************/
#define GPIOA_BASE			U(0x50002000)
#define GPIOB_BASE			U(0x50003000)
#define GPIOC_BASE			U(0x50004000)
#define GPIOD_BASE			U(0x50005000)
#define GPIOE_BASE			U(0x50006000)
#define GPIOF_BASE			U(0x50007000)
#define GPIOG_BASE			U(0x50008000)
#define GPIOH_BASE			U(0x50009000)
#define GPIOI_BASE			U(0x5000A000)
#define GPIOJ_BASE			U(0x5000B000)
#define GPIOK_BASE			U(0x5000C000)
#define GPIOZ_BASE			U(0x54004000)
#define GPIO_BANK_OFFSET		U(0x1000)

/* Bank IDs used in GPIO driver API */
#define GPIO_BANK_A			U(0)
#define GPIO_BANK_B			U(1)
#define GPIO_BANK_C			U(2)
#define GPIO_BANK_D			U(3)
#define GPIO_BANK_E			U(4)
#define GPIO_BANK_F			U(5)
#define GPIO_BANK_G			U(6)
#define GPIO_BANK_H			U(7)
#define GPIO_BANK_I			U(8)
#define GPIO_BANK_J			U(9)
#define GPIO_BANK_K			U(10)
#define GPIO_BANK_Z			U(25)

#define STM32MP_GPIOZ_PIN_MAX_COUNT	8

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
#define STM32MP_UART_BAUDRATE		U(115200)

/* For UART crash console */
#define STM32MP_DEBUG_USART_BASE	UART4_BASE
/* UART4 on HSI@64MHz, TX on GPIOG11 Alternate 6 */
#define STM32MP_DEBUG_USART_CLK_FRQ	64000000
#define DEBUG_UART_TX_GPIO_BANK_ADDRESS	GPIOG_BASE
#define DEBUG_UART_TX_GPIO_BANK_CLK_REG	RCC_MP_AHB4ENSETR
#define DEBUG_UART_TX_GPIO_BANK_CLK_EN	RCC_MP_AHB4ENSETR_GPIOGEN
#define DEBUG_UART_TX_GPIO_PORT		11
#define DEBUG_UART_TX_GPIO_ALTERNATE	6
#define DEBUG_UART_TX_CLKSRC_REG	RCC_UART24CKSELR
#define DEBUG_UART_TX_CLKSRC		RCC_UART24CKSELR_HSI
#define DEBUG_UART_TX_EN_REG		RCC_MP_APB1ENSETR
#define DEBUG_UART_TX_EN		RCC_MP_APB1ENSETR_UART4EN

/*******************************************************************************
 * STM32MP1 TZC (TZ400)
 ******************************************************************************/
#define STM32MP1_TZC_BASE		U(0x5C006000)

#define STM32MP1_TZC_A7_ID		U(0)
#define STM32MP1_TZC_M4_ID		U(1)
#define STM32MP1_TZC_LCD_ID		U(3)
#define STM32MP1_TZC_GPU_ID		U(4)
#define STM32MP1_TZC_MDMA_ID		U(5)
#define STM32MP1_TZC_DMA_ID		U(6)
#define STM32MP1_TZC_USB_HOST_ID	U(7)
#define STM32MP1_TZC_USB_OTG_ID		U(8)
#define STM32MP1_TZC_SDMMC_ID		U(9)
#define STM32MP1_TZC_ETH_ID		U(10)
#define STM32MP1_TZC_DAP_ID		U(15)

#define STM32MP1_FILTER_BIT_ALL		U(3)

/*******************************************************************************
 * STM32MP1 SDMMC
 ******************************************************************************/
#define STM32MP_SDMMC1_BASE		U(0x58005000)
#define STM32MP_SDMMC2_BASE		U(0x58007000)
#define STM32MP_SDMMC3_BASE		U(0x48004000)

#define STM32MP_MMC_INIT_FREQ			U(400000)	/*400 KHz*/
#define STM32MP_SD_NORMAL_SPEED_MAX_FREQ	U(25000000)	/*25 MHz*/
#define STM32MP_SD_HIGH_SPEED_MAX_FREQ		U(50000000)	/*50 MHz*/
#define STM32MP_EMMC_NORMAL_SPEED_MAX_FREQ	U(26000000)	/*26 MHz*/
#define STM32MP_EMMC_HIGH_SPEED_MAX_FREQ	U(52000000)	/*52 MHz*/

/*******************************************************************************
 * STM32MP1 BSEC / OTP
 ******************************************************************************/
#define STM32MP1_OTP_MAX_ID		0x5FU
#define STM32MP1_UPPER_OTP_START	0x20U

#define OTP_MAX_SIZE			(STM32MP1_OTP_MAX_ID + 1U)

/* OTP offsets */
#define DATA0_OTP			U(0)
#define PART_NUMBER_OTP			U(1)
#define PACKAGE_OTP			U(16)
#define HW2_OTP				U(18)

/* OTP mask */
/* DATA0 */
#define DATA0_OTP_SECURED		BIT(6)

/* PART NUMBER */
#define PART_NUMBER_OTP_PART_MASK	GENMASK_32(7, 0)
#define PART_NUMBER_OTP_PART_SHIFT	0

/* PACKAGE */
#define PACKAGE_OTP_PKG_MASK		GENMASK_32(29, 27)
#define PACKAGE_OTP_PKG_SHIFT		27

/* IWDG OTP */
#define HW2_OTP_IWDG_HW_POS		U(3)
#define HW2_OTP_IWDG_FZ_STOP_POS	U(5)
#define HW2_OTP_IWDG_FZ_STANDBY_POS	U(7)

/* HW2 OTP */
#define HW2_OTP_PRODUCT_BELOW_2V5	BIT(13)

/*******************************************************************************
 * STM32MP1 TAMP
 ******************************************************************************/
#define TAMP_BASE			U(0x5C00A000)
#define TAMP_BKP_REGISTER_BASE		(TAMP_BASE + U(0x100))

#if !(defined(__LINKER__) || defined(__ASSEMBLER__))
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
 * STM32MP1 IWDG
 ******************************************************************************/
#define IWDG_MAX_INSTANCE		U(2)
#define IWDG1_INST			U(0)
#define IWDG2_INST			U(1)

#define IWDG1_BASE			U(0x5C003000)
#define IWDG2_BASE			U(0x5A002000)

/*******************************************************************************
 * STM32MP1 I2C4
 ******************************************************************************/
#define I2C4_BASE			U(0x5C002000)

/*******************************************************************************
 * STM32MP1 DBGMCU
 ******************************************************************************/
#define DBGMCU_BASE			U(0x50081000)

/*******************************************************************************
 * Device Tree defines
 ******************************************************************************/
#define DT_BSEC_COMPAT			"st,stm32mp15-bsec"
#define DT_IWDG_COMPAT			"st,stm32mp1-iwdg"
#define DT_PWR_COMPAT			"st,stm32mp1-pwr"
#define DT_RCC_CLK_COMPAT		"st,stm32mp1-rcc"
#define DT_SYSCFG_COMPAT		"st,stm32mp157-syscfg"

#endif /* STM32MP1_DEF_H */
