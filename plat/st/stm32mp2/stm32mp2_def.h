/*
 * Copyright (c) 2023-2025, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef STM32MP2_DEF_H
#define STM32MP2_DEF_H

#include <common/tbbr/tbbr_img_def.h>
#ifndef __ASSEMBLER__
#include <drivers/st/bsec.h>
#include <drivers/st/stm32mp2_clk.h>
#include <drivers/st/stm32mp2_risaf.h>
#include <drivers/st/stm32mp_rifsc_regs.h>
#endif
#if STM32MP21
#include <drivers/st/stm32mp21_pwr.h>
#include <drivers/st/stm32mp21_rcc.h>
#else /* STM32MP21 */
#include <drivers/st/stm32mp2_pwr.h>
#include <drivers/st/stm32mp25_rcc.h>
#endif /* STM32MP21 */
#if STM32MP21
#include <dt-bindings/clock/st,stm32mp21-rcc.h>
#include <dt-bindings/clock/stm32mp21-clksrc.h>
#include <dt-bindings/reset/st,stm32mp21-rcc.h>
#endif /* STM32MP21 */
#if STM32MP23
#include <dt-bindings/clock/stm32mp25-clks.h>
#include <dt-bindings/clock/stm32mp25-clksrc.h>
#include <dt-bindings/reset/stm32mp25-resets.h>
#endif /* STM32MP23 */
#if STM32MP25
#include <dt-bindings/clock/stm32mp25-clks.h>
#include <dt-bindings/clock/stm32mp25-clksrc.h>
#include <dt-bindings/reset/stm32mp25-resets.h>
#endif /* STM32MP25 */
#include <dt-bindings/gpio/stm32-gpio.h>
#include <dt-bindings/soc/rif.h>
#include <dt-bindings/soc/stm32mp25-rif.h>

#ifndef __ASSEMBLER__
#include <boot_api.h>
#include <stm32mp2_private.h>
#include <stm32mp_common.h>
#include <stm32mp_dt.h>
#include <stm32mp_shared_resources.h>
#endif

/*******************************************************************************
 * CHIP ID
 ******************************************************************************/
#define STM32MP2_CHIP_ID			U(0x505)

#define STM32MP251A_PART_NB			U(0x400B3E6D)
#define STM32MP251C_PART_NB			U(0x000B306D)
#define STM32MP251D_PART_NB			U(0xC00B3E6D)
#define STM32MP251F_PART_NB			U(0x800B306D)
#define STM32MP253A_PART_NB			U(0x400B3E0C)
#define STM32MP253C_PART_NB			U(0x000B300C)
#define STM32MP253D_PART_NB			U(0xC00B3E0C)
#define STM32MP253F_PART_NB			U(0x800B300C)
#define STM32MP255A_PART_NB			U(0x40082E00)
#define STM32MP255C_PART_NB			U(0x00082000)
#define STM32MP255D_PART_NB			U(0xC0082E00)
#define STM32MP255F_PART_NB			U(0x80082000)
#define STM32MP257A_PART_NB			U(0x40002E00)
#define STM32MP257C_PART_NB			U(0x00002000)
#define STM32MP257D_PART_NB			U(0xC0002E00)
#define STM32MP257F_PART_NB			U(0x80002000)

#define STM32MP2_REV_A				U(0x08)
#define STM32MP2_REV_B				U(0x10)
#define STM32MP2_REV_X				U(0x12)
#define STM32MP2_REV_Y				U(0x11)
#define STM32MP2_REV_Z				U(0x09)

/*******************************************************************************
 * PACKAGE ID
 ******************************************************************************/
#define STM32MP25_PKG_CUSTOM			U(0)
#define STM32MP25_PKG_AL_VFBGA361		U(1)
#define STM32MP25_PKG_AK_VFBGA424		U(3)
#define STM32MP25_PKG_AI_TFBGA436		U(5)
#define STM32MP25_PKG_UNKNOWN			U(7)

/*******************************************************************************
 * STM32MP2 memory map related constants
 ******************************************************************************/
#define STM32MP_SYSRAM_BASE			U(0x0E000000)
#define STM32MP_SYSRAM_SIZE			U(0x00040000)
#define SRAM1_BASE				U(0x0E040000)
#define SRAM1_SIZE_FOR_TFA			U(0x00010000)
#define RETRAM_BASE				U(0x0E080000)
#define RETRAM_SIZE				U(0x00020000)

#if defined(IMAGE_BL2) && STM32MP_USB_PROGRAMMER
#define STM32MP_USB_DWC3_SIZE			PAGE_SIZE
#define STM32MP_USB_DWC3_BASE			(STM32MP_SYSRAM_BASE + \
						 STM32MP_SYSRAM_SIZE - \
						 STM32MP_SYSRAM_DEVICE_SIZE)

#define STM32MP_SYSRAM_DEVICE_SIZE		STM32MP_USB_DWC3_SIZE
#define STM32MP_SYSRAM_DEVICE_BASE		STM32MP_USB_DWC3_BASE

#define STM32MP_SYSRAM_MEM_SIZE			(STM32MP_SYSRAM_SIZE - \
						 STM32MP_SYSRAM_DEVICE_SIZE)
#define STM32MP_SYSRAM_MEM_BASE			STM32MP_SYSRAM_BASE
#endif /* IMAGE_BL2 && STM32MP_USB_PROGRAMMER */

/* DDR configuration */
#define STM32MP_DDR_BASE			U(0x80000000)
#define STM32MP_DDR_MAX_SIZE			UL(0x100000000)	/* Max 4GB */

/* DDR power initializations */
#ifndef __ASSEMBLER__
enum ddr_type {
	STM32MP_DDR3,
	STM32MP_DDR4,
	STM32MP_LPDDR4
};
#endif

/* Section used inside TF binaries */
#define STM32MP_PARAM_LOAD_SIZE			U(0x00002400) /* 9 KB for param */
/* 512 Bytes reserved for header */
#define STM32MP_HEADER_SIZE			U(0x00000200)
#define STM32MP_HEADER_BASE			(STM32MP_SYSRAM_BASE +	\
						 STM32MP_PARAM_LOAD_SIZE)

/* round_up(STM32MP_PARAM_LOAD_SIZE + STM32MP_HEADER_SIZE, PAGE_SIZE) */
#define STM32MP_HEADER_RESERVED_SIZE		U(0x3000)

#define STM32MP_BINARY_BASE			(STM32MP_SYSRAM_BASE +	\
						 STM32MP_PARAM_LOAD_SIZE +	\
						 STM32MP_HEADER_SIZE)

#define STM32MP_BINARY_SIZE			(STM32MP_SYSRAM_SIZE -	\
						 (STM32MP_PARAM_LOAD_SIZE +	\
						  STM32MP_HEADER_SIZE))

#define STM32MP_BL2_RO_SIZE			U(0x00020000) /* 128 KB */
#define STM32MP_BL2_SIZE			U(0x00029000) /* 164 KB for BL2 */

/* Allocate remaining sysram to BL31 Binary only */
#define STM32MP_BL31_SIZE			(STM32MP_SYSRAM_SIZE - \
						 STM32MP_BL2_SIZE)

#define STM32MP_BL2_BASE			(STM32MP_SYSRAM_BASE + \
						 STM32MP_SYSRAM_SIZE - \
						 STM32MP_BL2_SIZE)

#define STM32MP_BL2_RO_BASE			STM32MP_BL2_BASE

#define STM32MP_BL2_RW_BASE			(STM32MP_BL2_RO_BASE + \
						 STM32MP_BL2_RO_SIZE)

#define STM32MP_BL2_RW_SIZE			(STM32MP_SYSRAM_BASE + \
						 STM32MP_SYSRAM_SIZE - \
						 STM32MP_BL2_RW_BASE)

/* BL2 and BL32/sp_min require 4 tables */
#define MAX_XLAT_TABLES				U(4)	/* 16 KB for mapping */

/*
 * MAX_MMAP_REGIONS is usually:
 * BL stm32mp2_mmap size + mmap regions in *_plat_arch_setup
 */
#if STM32MP_USB_PROGRAMMER || defined(IMAGE_BL31)
#define MAX_MMAP_REGIONS			7
#else
#define MAX_MMAP_REGIONS			6
#endif

/* DTB initialization value */
#define STM32MP_BL2_DTB_SIZE			U(0x00006000)	/* 24 KB for DTB */

#define STM32MP_BL2_DTB_BASE			(STM32MP_BL2_BASE - \
						 STM32MP_BL2_DTB_SIZE)

#if defined(IMAGE_BL2)
#define STM32MP_DTB_SIZE			STM32MP_BL2_DTB_SIZE
#define STM32MP_DTB_BASE			STM32MP_BL2_DTB_BASE
#endif

#if STM32MP_DDR_FIP_IO_STORAGE
#define STM32MP_DDR_FW_BASE			SRAM1_BASE
#define STM32MP_DDR_FW_DMEM_OFFSET		U(0x400)
#define STM32MP_DDR_FW_IMEM_OFFSET		U(0x800)
#define STM32MP_DDR_FW_MAX_SIZE			U(0x8800)
#endif

#define STM32MP_FW_CONFIG_MAX_SIZE		PAGE_SIZE
#define STM32MP_FW_CONFIG_BASE			STM32MP_SYSRAM_BASE

#define STM32MP_BL33_BASE			(STM32MP_DDR_BASE + U(0x04000000))
#define STM32MP_BL33_MAX_SIZE			U(0x400000)
#define STM32MP_HW_CONFIG_BASE			(STM32MP_BL33_BASE + \
						STM32MP_BL33_MAX_SIZE)
#define STM32MP_HW_CONFIG_MAX_SIZE		U(0x40000)
#define STM32MP_SOC_FW_CONFIG_MAX_SIZE		U(0x10000) /* 64kB for BL31 DT */

/*******************************************************************************
 * STM32MP2 device/io map related constants (used for MMU)
 ******************************************************************************/
#define STM32MP_DEVICE_BASE			U(0x40000000)
#define STM32MP_DEVICE_SIZE			U(0x40000000)

/*******************************************************************************
 * STM32MP2 RCC
 ******************************************************************************/
#define RCC_BASE				U(0x44200000)

/*******************************************************************************
 * STM32MP2 PWR
 ******************************************************************************/
#define PWR_BASE				U(0x44210000)

/*******************************************************************************
 * STM32MP2 GPIO
 ******************************************************************************/
#define GPIOA_BASE				U(0x44240000)
#define GPIOB_BASE				U(0x44250000)
#define GPIOC_BASE				U(0x44260000)
#define GPIOD_BASE				U(0x44270000)
#define GPIOE_BASE				U(0x44280000)
#define GPIOF_BASE				U(0x44290000)
#define GPIOG_BASE				U(0x442A0000)
#define GPIOH_BASE				U(0x442B0000)
#define GPIOI_BASE				U(0x442C0000)
#define GPIOJ_BASE				U(0x442D0000)
#define GPIOK_BASE				U(0x442E0000)
#define GPIOZ_BASE				U(0x46200000)
#define GPIO_BANK_OFFSET			U(0x10000)

#define STM32MP_GPIOS_PIN_MAX_COUNT		16
#define STM32MP_GPIOZ_PIN_MAX_COUNT		8

/*******************************************************************************
 * STM32MP2 UART
 ******************************************************************************/
#define USART1_BASE				U(0x40330000)
#define USART2_BASE				U(0x400E0000)
#define USART3_BASE				U(0x400F0000)
#define UART4_BASE				U(0x40100000)
#define UART5_BASE				U(0x40110000)
#define USART6_BASE				U(0x40220000)
#define UART7_BASE				U(0x40370000)
#define UART8_BASE				U(0x40380000)
#define UART9_BASE				U(0x402C0000)
#define STM32MP_NB_OF_UART			U(9)

/* For UART crash console */
#define STM32MP_DEBUG_USART_CLK_FRQ		64000000
/* USART2 on HSI@64MHz, TX on GPIOA4 Alternate 6 */
#ifdef ULTRA_FLY
#define STM32MP_DEBUG_USART_BASE		USART1_BASE
#define DEBUG_UART_TX_GPIO_BANK_ADDRESS		GPIOA_BASE
#define DEBUG_UART_TX_GPIO_BANK_CLK_REG		RCC_GPIOACFGR
#define DEBUG_UART_TX_GPIO_BANK_CLK_EN		RCC_GPIOxCFGR_GPIOxEN
#define DEBUG_UART_TX_GPIO_PORT			3
#define DEBUG_UART_TX_GPIO_ALTERNATE		6
#define DEBUG_UART_TX_CLKSRC_REG		RCC_XBAR8CFGR
#define DEBUG_UART_TX_CLKSRC			XBAR_SRC_HSI
#define DEBUG_UART_TX_EN_REG			RCC_USART1CFGR
#define DEBUG_UART_TX_EN			RCC_UARTxCFGR_UARTxEN
#define DEBUG_UART_RST_REG			RCC_USART1CFGR
#define DEBUG_UART_RST_BIT			RCC_UARTxCFGR_UARTxRST
#define DEBUG_UART_PREDIV_CFGR			RCC_PREDIV19CFGR
#define DEBUG_UART_FINDIV_CFGR			RCC_FINDIV19CFGR
#else
#define STM32MP_DEBUG_USART_BASE		USART2_BASE
#define DEBUG_UART_TX_GPIO_BANK_ADDRESS		GPIOA_BASE
#define DEBUG_UART_TX_GPIO_BANK_CLK_REG		RCC_GPIOACFGR
#define DEBUG_UART_TX_GPIO_BANK_CLK_EN		RCC_GPIOxCFGR_GPIOxEN
#define DEBUG_UART_TX_GPIO_PORT			4
#define DEBUG_UART_TX_GPIO_ALTERNATE		6
#define DEBUG_UART_TX_CLKSRC_REG		RCC_XBAR8CFGR
#define DEBUG_UART_TX_CLKSRC			XBAR_SRC_HSI
#define DEBUG_UART_TX_EN_REG			RCC_USART2CFGR
#define DEBUG_UART_TX_EN			RCC_UARTxCFGR_UARTxEN
#define DEBUG_UART_RST_REG			RCC_USART2CFGR
#define DEBUG_UART_RST_BIT			RCC_UARTxCFGR_UARTxRST
#define DEBUG_UART_PREDIV_CFGR			RCC_PREDIV8CFGR
#define DEBUG_UART_FINDIV_CFGR			RCC_FINDIV8CFGR
#endif

/*******************************************************************************
 * STM32MP2 SDMMC
 ******************************************************************************/
#define STM32MP_SDMMC1_BASE			U(0x48220000)
#define STM32MP_SDMMC2_BASE			U(0x48230000)
#define STM32MP_SDMMC3_BASE			U(0x48240000)

/*******************************************************************************
 * STM32MP2 OSPI
 ******************************************************************************/
/* OSPI 1 & 2 memory map area */
#define STM32MP_OSPI_MM_BASE			U(0x60000000)
#define STM32MP_OSPI_MM_SIZE			U(0x10000000)

/*******************************************************************************
 * STM32MP2 BSEC / OTP
 ******************************************************************************/
/*
 * 367 available OTPs, the other are masked
 * - ECIES key: 368 to 375 (only readable by bootrom)
 * - HWKEY: 376 to 383 (never reloadable or readable)
 */
#define STM32MP2_OTP_MAX_ID			U(0x16F)
#define STM32MP2_MID_OTP_START			U(0x80)
#define STM32MP2_UPPER_OTP_START		U(0x100)

/* OTP labels */
#define PART_NUMBER_OTP				"part-number-otp"
#define REVISION_OTP				"rev_otp"
#define PACKAGE_OTP				"package-otp"
#define HCONF1_OTP				"otp124"
#define NAND_OTP				"otp16"
#define NAND2_OTP				"otp20"
#define BOARD_ID_OTP				"board-id"
#define UID_OTP					"uid-otp"
#define LIFECYCLE2_OTP				"otp18"
#define PKH_OTP					"otp144"
#define ENCKEY_OTP				"otp260"

/* OTP mask */
/* PACKAGE */
#define PACKAGE_OTP_PKG_MASK			GENMASK_32(2, 0)
#define PACKAGE_OTP_PKG_SHIFT			U(0)

/* IWDG OTP */
#define HCONF1_OTP_IWDG_HW_POS			U(0)
#define HCONF1_OTP_IWDG_FZ_STOP_POS		U(1)
#define HCONF1_OTP_IWDG_FZ_STANDBY_POS		U(2)

/* NAND OTP */
/* NAND parameter storage flag */
#define NAND_PARAM_STORED_IN_OTP		BIT_32(31)

/* NAND page size in bytes */
#define NAND_PAGE_SIZE_MASK			GENMASK_32(30, 29)
#define NAND_PAGE_SIZE_SHIFT			U(29)
#define NAND_PAGE_SIZE_2K			U(0)
#define NAND_PAGE_SIZE_4K			U(1)
#define NAND_PAGE_SIZE_8K			U(2)

/* NAND block size in pages */
#define NAND_BLOCK_SIZE_MASK			GENMASK_32(28, 27)
#define NAND_BLOCK_SIZE_SHIFT			U(27)
#define NAND_BLOCK_SIZE_64_PAGES		U(0)
#define NAND_BLOCK_SIZE_128_PAGES		U(1)
#define NAND_BLOCK_SIZE_256_PAGES		U(2)

/* NAND number of block (in unit of 256 blocks) */
#define NAND_BLOCK_NB_MASK			GENMASK_32(26, 19)
#define NAND_BLOCK_NB_SHIFT			U(19)
#define NAND_BLOCK_NB_UNIT			U(256)

/* NAND bus width in bits */
#define NAND_WIDTH_MASK				BIT_32(18)
#define NAND_WIDTH_SHIFT			U(18)

/* NAND number of ECC bits per 512 bytes */
#define NAND_ECC_BIT_NB_MASK			GENMASK_32(17, 15)
#define NAND_ECC_BIT_NB_SHIFT			U(15)
#define NAND_ECC_BIT_NB_UNSET			U(0)
#define NAND_ECC_BIT_NB_1_BITS			U(1)
#define NAND_ECC_BIT_NB_4_BITS			U(2)
#define NAND_ECC_BIT_NB_8_BITS			U(3)
#define NAND_ECC_ON_DIE				U(4)

/* NAND number of planes */
#define NAND_PLANE_BIT_NB_MASK			BIT_32(14)

/* NAND2 OTP */
#define NAND2_PAGE_SIZE_SHIFT			U(16)

/* NAND2 config distribution */
#define NAND2_CONFIG_DISTRIB			BIT_32(0)
#define NAND2_PNAND_NAND2_SNAND_NAND1		U(0)
#define NAND2_PNAND_NAND1_SNAND_NAND2		U(1)

/* MONOTONIC OTP */
#define MAX_MONOTONIC_VALUE			U(32)

/* UID OTP */
#define UID_WORD_NB				U(3)

/* Lifecycle OTP */
#define SECURE_BOOT_CLOSED_SECURE		GENMASK_32(3, 0)

/*******************************************************************************
 * STM32MP2 TAMP
 ******************************************************************************/
#define PLAT_MAX_TAMP_INT			U(5)
#define PLAT_MAX_TAMP_EXT			U(3)
#define TAMP_BASE				U(0x46010000)
#define TAMP_SMCR				(TAMP_BASE + U(0x20))
#define TAMP_BKP_REGISTER_BASE			(TAMP_BASE + U(0x100))
#define TAMP_BKP_REG_CLK			CK_BUS_RTC
#define TAMP_BKP_SEC_NUMBER			U(10)
#define TAMP_COUNTR				U(0x40)

#if !(defined(__LINKER__) || defined(__ASSEMBLER__))
static inline uintptr_t tamp_bkpr(uint32_t idx)
{
	return TAMP_BKP_REGISTER_BASE + (idx << 2);
}
#endif

/*******************************************************************************
 * STM32MP2 USB
 ******************************************************************************/
#define USB_DWC3_BASE				U(0x48300000)

/*******************************************************************************
 * STM32MP2 DDRCTRL
 ******************************************************************************/
#define DDRCTRL_BASE				U(0x48040000)

/*******************************************************************************
 * STM32MP2 DDRDBG
 ******************************************************************************/
#define DDRDBG_BASE				U(0x48050000)

/*******************************************************************************
 * STM32MP2 DDRPHYC
 ******************************************************************************/
#define DDRPHYC_BASE				U(0x48C00000)

/*******************************************************************************
 * Miscellaneous STM32MP2 peripherals base address
 ******************************************************************************/
#define BSEC_BASE				U(0x44000000)
#define DBGMCU_BASE				U(0x4A010000)
#define HASH_BASE				U(0x42010000)
#define RTC_BASE				U(0x46000000)
#define STGEN_BASE				U(0x48080000)
#define SYSCFG_BASE				U(0x44230000)

/*******************************************************************************
 * STM32MP RIF
 ******************************************************************************/
#define RIFSC_BASE				U(0x42080000)
#define RISAB1_BASE				U(0x420F0000)
#define RISAB2_BASE				U(0x42100000)
#define RISAB3_BASE				U(0x42110000)
#define RISAB5_BASE				U(0x42130000)

#define RISAF1_INST				0
#define RISAF2_INST				1
#define RISAF4_INST				3
#define RISAF5_INST				4
#define RISAF_MAX_INSTANCE			5

#define RISAF1_BASE				U(0x420A0000)
#define RISAF2_BASE				U(0x420B0000)
#define RISAF4_BASE				U(0x420D0000)
#define RISAF5_BASE				U(0x420E0000)

#define USE_RISAF2
#define USE_RISAF4

#ifdef USE_RISAF1
#define RISAF1_MAX_REGION			4
#else
#define RISAF1_MAX_REGION			0
#endif
#ifdef USE_RISAF2
#define RISAF2_MAX_REGION			4
#else
#define RISAF2_MAX_REGION			0
#endif
#ifdef USE_RISAF4
/* Consider only encrypted region maximum number, to save memory consumption */
#define RISAF4_MAX_REGION			4
#else
#define RISAF4_MAX_REGION			0
#endif
#ifdef USE_RISAF5
#define RISAF5_MAX_REGION			2
#else
#define RISAF5_MAX_REGION			0
#endif
#define RISAF_MAX_REGION			(RISAF1_MAX_REGION + RISAF2_MAX_REGION + \
						 RISAF4_MAX_REGION + RISAF5_MAX_REGION)

#define RISAF_KEY_SIZE_IN_BYTES			RISAF_ENCRYPTION_KEY_SIZE_IN_BYTES
#define RISAF_SEED_SIZE_IN_BYTES		U(4)

/*******************************************************************************
 * RIFSC
 ******************************************************************************/
#define STM32MP2_RIMU_USB3DR			U(4)

/*
 * USB3DR Secure/Priv Master (DMA) access
 */
#define RIFSC_USB_BOOT_USB3DR_RIMC_CONF		(RIFSC_RIMC_ATTRx_MPRIV | RIFSC_RIMC_ATTRx_MSEC | \
						 RIF_CID1 << RIFSC_RIMC_ATTRx_MCID_SHIFT | \
						 RIFSC_RIMC_ATTRx_CIDSEL)

/*******************************************************************************
 * STM32MP CA35SSC
 ******************************************************************************/
#define A35SSC_BASE				U(0x48800000)

/*******************************************************************************
 * REGULATORS
 ******************************************************************************/
/* 3 PWR + 1 VREFBUF + 14 PMIC regulators + 1 FIXED */
#define PLAT_NB_RDEVS				U(19)
/* 2 FIXED */
#define PLAT_NB_FIXED_REGUS			U(2)
/* No GPIO regu */
#define PLAT_NB_GPIO_REGUS			U(0)

/*******************************************************************************
 * Device Tree defines
 ******************************************************************************/
#define DT_BSEC_COMPAT				"st,stm32mp25-bsec"
#define DT_DDR_COMPAT				"st,stm32mp2-ddr"
#define DT_PWR_COMPAT				"st,stm32mp25-pwr"
#if STM32MP21
#define DT_RCC_CLK_COMPAT			"st,stm32mp21-rcc"
#else
#define DT_RCC_CLK_COMPAT			"st,stm32mp25-rcc"
#endif
#define DT_SDMMC2_COMPAT			"st,stm32mp25-sdmmc2"
#define DT_UART_COMPAT				"st,stm32h7-uart"

#endif /* STM32MP2_DEF_H */
