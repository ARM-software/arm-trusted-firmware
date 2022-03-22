/*
 * Copyright (C) 2022, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>

#include <arch.h>
#include <arch_helpers.h>
#include "clk-stm32-core.h"
#include <common/debug.h>
#include <common/fdt_wrappers.h>
#include <drivers/clk.h>
#include <drivers/delay_timer.h>
#include <drivers/st/stm32mp13_rcc.h>
#include <drivers/st/stm32mp1_clk.h>
#include <drivers/st/stm32mp_clkfunc.h>
#include <dt-bindings/clock/stm32mp13-clksrc.h>
#include <lib/mmio.h>
#include <lib/spinlock.h>
#include <lib/utils_def.h>
#include <libfdt.h>
#include <plat/common/platform.h>

#include <platform_def.h>

struct stm32_osci_dt_cfg {
	unsigned long freq;
	bool bypass;
	bool digbyp;
	bool css;
	uint32_t drive;
};

enum pll_mn {
	PLL_CFG_M,
	PLL_CFG_N,
	PLL_DIV_MN_NB
};

enum pll_pqr {
	PLL_CFG_P,
	PLL_CFG_Q,
	PLL_CFG_R,
	PLL_DIV_PQR_NB
};

enum pll_csg {
	PLL_CSG_MOD_PER,
	PLL_CSG_INC_STEP,
	PLL_CSG_SSCG_MODE,
	PLL_CSG_NB
};

struct stm32_pll_vco {
	uint32_t status;
	uint32_t src;
	uint32_t div_mn[PLL_DIV_MN_NB];
	uint32_t frac;
	bool csg_enabled;
	uint32_t csg[PLL_CSG_NB];
};

struct stm32_pll_output {
	uint32_t output[PLL_DIV_PQR_NB];
};

struct stm32_pll_dt_cfg {
	struct stm32_pll_vco vco;
	struct stm32_pll_output output;
};

struct stm32_clk_platdata {
	uint32_t nosci;
	struct stm32_osci_dt_cfg *osci;
	uint32_t npll;
	struct stm32_pll_dt_cfg *pll;
	uint32_t nclksrc;
	uint32_t *clksrc;
	uint32_t nclkdiv;
	uint32_t *clkdiv;
};

enum stm32_clock {
	/* ROOT CLOCKS */
	_CK_OFF,
	_CK_HSI,
	_CK_HSE,
	_CK_CSI,
	_CK_LSI,
	_CK_LSE,
	_I2SCKIN,
	_CSI_DIV122,
	_HSE_DIV,
	_HSE_DIV2,
	_CK_PLL1,
	_CK_PLL2,
	_CK_PLL3,
	_CK_PLL4,
	_PLL1P,
	_PLL1P_DIV,
	_PLL2P,
	_PLL2Q,
	_PLL2R,
	_PLL3P,
	_PLL3Q,
	_PLL3R,
	_PLL4P,
	_PLL4Q,
	_PLL4R,
	_PCLK1,
	_PCLK2,
	_PCLK3,
	_PCLK4,
	_PCLK5,
	_PCLK6,
	_CKMPU,
	_CKAXI,
	_CKMLAHB,
	_CKPER,
	_CKTIMG1,
	_CKTIMG2,
	_CKTIMG3,
	_USB_PHY_48,
	_MCO1_K,
	_MCO2_K,
	_TRACECK,
	/* BUS and KERNEL CLOCKS */
	_DDRC1,
	_DDRC1LP,
	_DDRPHYC,
	_DDRPHYCLP,
	_DDRCAPB,
	_DDRCAPBLP,
	_AXIDCG,
	_DDRPHYCAPB,
	_DDRPHYCAPBLP,
	_SYSCFG,
	_DDRPERFM,
	_IWDG2APB,
	_USBPHY_K,
	_USBO_K,
	_RTCAPB,
	_TZC,
	_ETZPC,
	_IWDG1APB,
	_BSEC,
	_STGENC,
	_USART1_K,
	_USART2_K,
	_I2C3_K,
	_I2C4_K,
	_I2C5_K,
	_TIM12,
	_TIM15,
	_RTCCK,
	_GPIOA,
	_GPIOB,
	_GPIOC,
	_GPIOD,
	_GPIOE,
	_GPIOF,
	_GPIOG,
	_GPIOH,
	_GPIOI,
	_PKA,
	_SAES_K,
	_CRYP1,
	_HASH1,
	_RNG1_K,
	_BKPSRAM,
	_SDMMC1_K,
	_SDMMC2_K,
	_DBGCK,
	_USART3_K,
	_UART4_K,
	_UART5_K,
	_UART7_K,
	_UART8_K,
	_USART6_K,
	_MCE,
	_FMC_K,
	_QSPI_K,
#if defined(IMAGE_BL32)
	_LTDC,
	_DMA1,
	_DMA2,
	_MDMA,
	_ETH1MAC,
	_USBH,
	_TIM2,
	_TIM3,
	_TIM4,
	_TIM5,
	_TIM6,
	_TIM7,
	_LPTIM1_K,
	_SPI2_K,
	_SPI3_K,
	_SPDIF_K,
	_TIM1,
	_TIM8,
	_SPI1_K,
	_SAI1_K,
	_SAI2_K,
	_DFSDM,
	_FDCAN_K,
	_TIM13,
	_TIM14,
	_TIM16,
	_TIM17,
	_SPI4_K,
	_SPI5_K,
	_I2C1_K,
	_I2C2_K,
	_ADFSDM,
	_LPTIM2_K,
	_LPTIM3_K,
	_LPTIM4_K,
	_LPTIM5_K,
	_VREF,
	_DTS,
	_PMBCTRL,
	_HDP,
	_STGENRO,
	_DCMIPP_K,
	_DMAMUX1,
	_DMAMUX2,
	_DMA3,
	_ADC1_K,
	_ADC2_K,
	_TSC,
	_AXIMC,
	_ETH1CK,
	_ETH1TX,
	_ETH1RX,
	_CRC1,
	_ETH2CK,
	_ETH2TX,
	_ETH2RX,
	_ETH2MAC,
#endif
	CK_LAST
};

/* PARENT CONFIG */
static const uint16_t RTC_src[] = {
	 _CK_OFF, _CK_LSE, _CK_LSI, _CK_HSE
};

static const uint16_t MCO1_src[] = {
	 _CK_HSI, _CK_HSE, _CK_CSI, _CK_LSI, _CK_LSE
};

static const uint16_t MCO2_src[] = {
	 _CKMPU, _CKAXI, _CKMLAHB, _PLL4P, _CK_HSE, _CK_HSI
};

static const uint16_t PLL12_src[] = {
	 _CK_HSI, _CK_HSE
};

static const uint16_t PLL3_src[] = {
	 _CK_HSI, _CK_HSE, _CK_CSI
};

static const uint16_t PLL4_src[] = {
	 _CK_HSI, _CK_HSE, _CK_CSI, _I2SCKIN
};

static const uint16_t MPU_src[] = {
	 _CK_HSI, _CK_HSE, _PLL1P, _PLL1P_DIV
};

static const uint16_t AXI_src[] = {
	 _CK_HSI, _CK_HSE, _PLL2P
};

static const uint16_t MLAHBS_src[] = {
	 _CK_HSI, _CK_HSE, _CK_CSI, _PLL3P
};

static const uint16_t CKPER_src[] = {
	 _CK_HSI, _CK_CSI, _CK_HSE, _CK_OFF
};

static const uint16_t I2C12_src[] = {
	 _PCLK1, _PLL4R, _CK_HSI, _CK_CSI
};

static const uint16_t I2C3_src[] = {
	 _PCLK6, _PLL4R, _CK_HSI, _CK_CSI
};

static const uint16_t I2C4_src[] = {
	 _PCLK6, _PLL4R, _CK_HSI, _CK_CSI
};

static const uint16_t I2C5_src[] = {
	 _PCLK6, _PLL4R, _CK_HSI, _CK_CSI
};

static const uint16_t SPI1_src[] = {
	 _PLL4P, _PLL3Q, _I2SCKIN, _CKPER, _PLL3R
};

static const uint16_t SPI23_src[] = {
	 _PLL4P, _PLL3Q, _I2SCKIN, _CKPER, _PLL3R
};

static const uint16_t SPI4_src[] = {
	 _PCLK6, _PLL4Q, _CK_HSI, _CK_CSI, _CK_HSE, _I2SCKIN
};

static const uint16_t SPI5_src[] = {
	 _PCLK6, _PLL4Q, _CK_HSI, _CK_CSI, _CK_HSE
};

static const uint16_t UART1_src[] = {
	 _PCLK6, _PLL3Q, _CK_HSI, _CK_CSI, _PLL4Q, _CK_HSE
};

static const uint16_t UART2_src[] = {
	 _PCLK6, _PLL3Q, _CK_HSI, _CK_CSI, _PLL4Q, _CK_HSE
};

static const uint16_t UART35_src[] = {
	 _PCLK1, _PLL4Q, _CK_HSI, _CK_CSI, _CK_HSE
};

static const uint16_t UART4_src[] = {
	 _PCLK1, _PLL4Q, _CK_HSI, _CK_CSI, _CK_HSE
};

static const uint16_t UART6_src[] = {
	 _PCLK2, _PLL4Q, _CK_HSI, _CK_CSI, _CK_HSE
};

static const uint16_t UART78_src[] = {
	 _PCLK1, _PLL4Q, _CK_HSI, _CK_CSI, _CK_HSE
};

static const uint16_t LPTIM1_src[] = {
	 _PCLK1, _PLL4P, _PLL3Q, _CK_LSE, _CK_LSI, _CKPER
};

static const uint16_t LPTIM2_src[] = {
	 _PCLK3, _PLL4Q, _CKPER, _CK_LSE, _CK_LSI
};

static const uint16_t LPTIM3_src[] = {
	 _PCLK3, _PLL4Q, _CKPER, _CK_LSE, _CK_LSI
};

static const uint16_t LPTIM45_src[] = {
	 _PCLK3, _PLL4P, _PLL3Q, _CK_LSE, _CK_LSI, _CKPER
};

static const uint16_t SAI1_src[] = {
	 _PLL4Q, _PLL3Q, _I2SCKIN, _CKPER, _PLL3R
};

static const uint16_t SAI2_src[] = {
	 _PLL4Q, _PLL3Q, _I2SCKIN, _CKPER, _NO_ID, _PLL3R
};

static const uint16_t FDCAN_src[] = {
	 _CK_HSE, _PLL3Q, _PLL4Q, _PLL4R
};

static const uint16_t SPDIF_src[] = {
	 _PLL4P, _PLL3Q, _CK_HSI
};

static const uint16_t ADC1_src[] = {
	 _PLL4R, _CKPER, _PLL3Q
};

static const uint16_t ADC2_src[] = {
	 _PLL4R, _CKPER, _PLL3Q
};

static const uint16_t SDMMC1_src[] = {
	 _CKAXI, _PLL3R, _PLL4P, _CK_HSI
};

static const uint16_t SDMMC2_src[] = {
	 _CKAXI, _PLL3R, _PLL4P, _CK_HSI
};

static const uint16_t ETH1_src[] = {
	 _PLL4P, _PLL3Q
};

static const uint16_t ETH2_src[] = {
	 _PLL4P, _PLL3Q
};

static const uint16_t USBPHY_src[] = {
	 _CK_HSE, _PLL4R, _HSE_DIV2
};

static const uint16_t USBO_src[] = {
	 _PLL4R, _USB_PHY_48
};

static const uint16_t QSPI_src[] = {
	 _CKAXI, _PLL3R, _PLL4P, _CKPER
};

static const uint16_t FMC_src[] = {
	 _CKAXI, _PLL3R, _PLL4P, _CKPER
};

/* Position 2 of RNG1 mux is reserved */
static const uint16_t RNG1_src[] = {
	 _CK_CSI, _PLL4R, _CK_OFF, _CK_LSI
};

static const uint16_t STGEN_src[] = {
	 _CK_HSI, _CK_HSE
};

static const uint16_t DCMIPP_src[] = {
	 _CKAXI, _PLL2Q, _PLL4P, _CKPER
};

static const uint16_t SAES_src[] = {
	 _CKAXI, _CKPER, _PLL4R, _CK_LSI
};

#define MUX_CFG(id, src, _offset, _shift, _witdh)[id] = {\
	.id_parents	= src,\
	.num_parents	= ARRAY_SIZE(src),\
	.mux		= &(struct mux_cfg) {\
		.offset	= (_offset),\
		.shift	= (_shift),\
		.width	= (_witdh),\
		.bitrdy = MUX_NO_BIT_RDY,\
	},\
}

#define MUX_RDY_CFG(id, src, _offset, _shift, _witdh)[id] = {\
	.id_parents	= src,\
	.num_parents	= ARRAY_SIZE(src),\
	.mux		= &(struct mux_cfg) {\
		.offset	= (_offset),\
		.shift	= (_shift),\
		.width	= (_witdh),\
		.bitrdy = 31,\
	},\
}

static const struct parent_cfg parent_mp13[] = {
	MUX_CFG(MUX_ADC1,	ADC1_src,	RCC_ADC12CKSELR, 0, 2),
	MUX_CFG(MUX_ADC2,	ADC2_src,	RCC_ADC12CKSELR, 2, 2),
	MUX_RDY_CFG(MUX_AXI,	AXI_src,	RCC_ASSCKSELR, 0, 3),
	MUX_CFG(MUX_CKPER,	CKPER_src,	RCC_CPERCKSELR, 0, 2),
	MUX_CFG(MUX_DCMIPP,	DCMIPP_src,	RCC_DCMIPPCKSELR, 0, 2),
	MUX_CFG(MUX_ETH1,	ETH1_src,	RCC_ETH12CKSELR, 0, 2),
	MUX_CFG(MUX_ETH2,	ETH2_src,	RCC_ETH12CKSELR, 8, 2),
	MUX_CFG(MUX_FDCAN,	FDCAN_src,	RCC_FDCANCKSELR, 0, 2),
	MUX_CFG(MUX_FMC,	FMC_src,	RCC_FMCCKSELR, 0, 2),
	MUX_CFG(MUX_I2C12,	I2C12_src,	RCC_I2C12CKSELR, 0, 3),
	MUX_CFG(MUX_I2C3,	I2C3_src,	RCC_I2C345CKSELR, 0, 3),
	MUX_CFG(MUX_I2C4,	I2C4_src,	RCC_I2C345CKSELR, 3, 3),
	MUX_CFG(MUX_I2C5,	I2C5_src,	RCC_I2C345CKSELR, 6, 3),
	MUX_CFG(MUX_LPTIM1,	LPTIM1_src,	RCC_LPTIM1CKSELR, 0, 3),
	MUX_CFG(MUX_LPTIM2,	LPTIM2_src,	RCC_LPTIM23CKSELR, 0, 3),
	MUX_CFG(MUX_LPTIM3,	LPTIM3_src,	RCC_LPTIM23CKSELR, 3, 3),
	MUX_CFG(MUX_LPTIM45,	LPTIM45_src,	RCC_LPTIM45CKSELR, 0, 3),
	MUX_CFG(MUX_MCO1,	MCO1_src,	RCC_MCO1CFGR, 0, 3),
	MUX_CFG(MUX_MCO2,	MCO2_src,	RCC_MCO2CFGR, 0, 3),
	MUX_RDY_CFG(MUX_MLAHB,	MLAHBS_src,	RCC_MSSCKSELR, 0, 2),
	MUX_RDY_CFG(MUX_MPU,	MPU_src,	RCC_MPCKSELR, 0, 2),
	MUX_RDY_CFG(MUX_PLL12,	PLL12_src,	RCC_RCK12SELR, 0, 2),
	MUX_RDY_CFG(MUX_PLL3,	PLL3_src,	RCC_RCK3SELR, 0, 2),
	MUX_RDY_CFG(MUX_PLL4,	PLL4_src,	RCC_RCK4SELR, 0, 2),
	MUX_CFG(MUX_QSPI,	QSPI_src,	RCC_QSPICKSELR, 0, 2),
	MUX_CFG(MUX_RNG1,	RNG1_src,	RCC_RNG1CKSELR, 0, 2),
	MUX_CFG(MUX_RTC,	RTC_src,	RCC_BDCR, 16, 2),
	MUX_CFG(MUX_SAES,	SAES_src,	RCC_SAESCKSELR, 0, 2),
	MUX_CFG(MUX_SAI1,	SAI1_src,	RCC_SAI1CKSELR, 0, 3),
	MUX_CFG(MUX_SAI2,	SAI2_src,	RCC_SAI2CKSELR, 0, 3),
	MUX_CFG(MUX_SDMMC1,	SDMMC1_src,	RCC_SDMMC12CKSELR, 0, 3),
	MUX_CFG(MUX_SDMMC2,	SDMMC2_src,	RCC_SDMMC12CKSELR, 3, 3),
	MUX_CFG(MUX_SPDIF,	SPDIF_src,	RCC_SPDIFCKSELR, 0, 2),
	MUX_CFG(MUX_SPI1,	SPI1_src,	RCC_SPI2S1CKSELR, 0, 3),
	MUX_CFG(MUX_SPI23,	SPI23_src,	RCC_SPI2S23CKSELR, 0, 3),
	MUX_CFG(MUX_SPI4,	SPI4_src,	RCC_SPI45CKSELR, 0, 3),
	MUX_CFG(MUX_SPI5,	SPI5_src,	RCC_SPI45CKSELR, 3, 3),
	MUX_CFG(MUX_STGEN,	STGEN_src,	RCC_STGENCKSELR, 0, 2),
	MUX_CFG(MUX_UART1,	UART1_src,	RCC_UART12CKSELR, 0, 3),
	MUX_CFG(MUX_UART2,	UART2_src,	RCC_UART12CKSELR, 3, 3),
	MUX_CFG(MUX_UART35,	UART35_src,	RCC_UART35CKSELR, 0, 3),
	MUX_CFG(MUX_UART4,	UART4_src,	RCC_UART4CKSELR, 0, 3),
	MUX_CFG(MUX_UART6,	UART6_src,	RCC_UART6CKSELR, 0, 3),
	MUX_CFG(MUX_UART78,	UART78_src,	RCC_UART78CKSELR, 0, 3),
	MUX_CFG(MUX_USBO,	USBO_src,	RCC_USBCKSELR, 4, 1),
	MUX_CFG(MUX_USBPHY,	USBPHY_src,	RCC_USBCKSELR, 0, 2),
};

/*
 * GATE CONFIG
 */

enum enum_gate_cfg {
	GATE_ZERO, /* reserved for no gate */
	GATE_LSE,
	GATE_RTCCK,
	GATE_LSI,
	GATE_HSI,
	GATE_CSI,
	GATE_HSE,
	GATE_LSI_RDY,
	GATE_CSI_RDY,
	GATE_LSE_RDY,
	GATE_HSE_RDY,
	GATE_HSI_RDY,
	GATE_MCO1,
	GATE_MCO2,
	GATE_DBGCK,
	GATE_TRACECK,
	GATE_PLL1,
	GATE_PLL1_DIVP,
	GATE_PLL1_DIVQ,
	GATE_PLL1_DIVR,
	GATE_PLL2,
	GATE_PLL2_DIVP,
	GATE_PLL2_DIVQ,
	GATE_PLL2_DIVR,
	GATE_PLL3,
	GATE_PLL3_DIVP,
	GATE_PLL3_DIVQ,
	GATE_PLL3_DIVR,
	GATE_PLL4,
	GATE_PLL4_DIVP,
	GATE_PLL4_DIVQ,
	GATE_PLL4_DIVR,
	GATE_DDRC1,
	GATE_DDRC1LP,
	GATE_DDRPHYC,
	GATE_DDRPHYCLP,
	GATE_DDRCAPB,
	GATE_DDRCAPBLP,
	GATE_AXIDCG,
	GATE_DDRPHYCAPB,
	GATE_DDRPHYCAPBLP,
	GATE_TIM2,
	GATE_TIM3,
	GATE_TIM4,
	GATE_TIM5,
	GATE_TIM6,
	GATE_TIM7,
	GATE_LPTIM1,
	GATE_SPI2,
	GATE_SPI3,
	GATE_USART3,
	GATE_UART4,
	GATE_UART5,
	GATE_UART7,
	GATE_UART8,
	GATE_I2C1,
	GATE_I2C2,
	GATE_SPDIF,
	GATE_TIM1,
	GATE_TIM8,
	GATE_SPI1,
	GATE_USART6,
	GATE_SAI1,
	GATE_SAI2,
	GATE_DFSDM,
	GATE_ADFSDM,
	GATE_FDCAN,
	GATE_LPTIM2,
	GATE_LPTIM3,
	GATE_LPTIM4,
	GATE_LPTIM5,
	GATE_VREF,
	GATE_DTS,
	GATE_PMBCTRL,
	GATE_HDP,
	GATE_SYSCFG,
	GATE_DCMIPP,
	GATE_DDRPERFM,
	GATE_IWDG2APB,
	GATE_USBPHY,
	GATE_STGENRO,
	GATE_LTDC,
	GATE_RTCAPB,
	GATE_TZC,
	GATE_ETZPC,
	GATE_IWDG1APB,
	GATE_BSEC,
	GATE_STGENC,
	GATE_USART1,
	GATE_USART2,
	GATE_SPI4,
	GATE_SPI5,
	GATE_I2C3,
	GATE_I2C4,
	GATE_I2C5,
	GATE_TIM12,
	GATE_TIM13,
	GATE_TIM14,
	GATE_TIM15,
	GATE_TIM16,
	GATE_TIM17,
	GATE_DMA1,
	GATE_DMA2,
	GATE_DMAMUX1,
	GATE_DMA3,
	GATE_DMAMUX2,
	GATE_ADC1,
	GATE_ADC2,
	GATE_USBO,
	GATE_TSC,
	GATE_GPIOA,
	GATE_GPIOB,
	GATE_GPIOC,
	GATE_GPIOD,
	GATE_GPIOE,
	GATE_GPIOF,
	GATE_GPIOG,
	GATE_GPIOH,
	GATE_GPIOI,
	GATE_PKA,
	GATE_SAES,
	GATE_CRYP1,
	GATE_HASH1,
	GATE_RNG1,
	GATE_BKPSRAM,
	GATE_AXIMC,
	GATE_MCE,
	GATE_ETH1CK,
	GATE_ETH1TX,
	GATE_ETH1RX,
	GATE_ETH1MAC,
	GATE_FMC,
	GATE_QSPI,
	GATE_SDMMC1,
	GATE_SDMMC2,
	GATE_CRC1,
	GATE_USBH,
	GATE_ETH2CK,
	GATE_ETH2TX,
	GATE_ETH2RX,
	GATE_ETH2MAC,
	GATE_MDMA,

	LAST_GATE
};

#define GATE_CFG(id, _offset, _bit_idx, _offset_clr)[id] = {\
	.offset		= (_offset),\
	.bit_idx	= (_bit_idx),\
	.set_clr	= (_offset_clr),\
}

static const struct gate_cfg gates_mp13[LAST_GATE] = {
	GATE_CFG(GATE_LSE,		RCC_BDCR,	0,	0),
	GATE_CFG(GATE_RTCCK,		RCC_BDCR,	20,	0),
	GATE_CFG(GATE_LSI,		RCC_RDLSICR,	0,	0),
	GATE_CFG(GATE_HSI,		RCC_OCENSETR,	0,	1),
	GATE_CFG(GATE_CSI,		RCC_OCENSETR,	4,	1),
	GATE_CFG(GATE_HSE,		RCC_OCENSETR,	8,	1),
	GATE_CFG(GATE_LSI_RDY,		RCC_RDLSICR,	1,	0),
	GATE_CFG(GATE_CSI_RDY,		RCC_OCRDYR,	4,	0),
	GATE_CFG(GATE_LSE_RDY,		RCC_BDCR,	2,	0),
	GATE_CFG(GATE_HSE_RDY,		RCC_OCRDYR,	8,	0),
	GATE_CFG(GATE_HSI_RDY,		RCC_OCRDYR,	0,	0),
	GATE_CFG(GATE_MCO1,		RCC_MCO1CFGR,	12,	0),
	GATE_CFG(GATE_MCO2,		RCC_MCO2CFGR,	12,	0),
	GATE_CFG(GATE_DBGCK,		RCC_DBGCFGR,	8,	0),
	GATE_CFG(GATE_TRACECK,		RCC_DBGCFGR,	9,	0),
	GATE_CFG(GATE_PLL1,		RCC_PLL1CR,	0,	0),
	GATE_CFG(GATE_PLL1_DIVP,	RCC_PLL1CR,	4,	0),
	GATE_CFG(GATE_PLL1_DIVQ,	RCC_PLL1CR,	5,	0),
	GATE_CFG(GATE_PLL1_DIVR,	RCC_PLL1CR,	6,	0),
	GATE_CFG(GATE_PLL2,		RCC_PLL2CR,	0,	0),
	GATE_CFG(GATE_PLL2_DIVP,	RCC_PLL2CR,	4,	0),
	GATE_CFG(GATE_PLL2_DIVQ,	RCC_PLL2CR,	5,	0),
	GATE_CFG(GATE_PLL2_DIVR,	RCC_PLL2CR,	6,	0),
	GATE_CFG(GATE_PLL3,		RCC_PLL3CR,	0,	0),
	GATE_CFG(GATE_PLL3_DIVP,	RCC_PLL3CR,	4,	0),
	GATE_CFG(GATE_PLL3_DIVQ,	RCC_PLL3CR,	5,	0),
	GATE_CFG(GATE_PLL3_DIVR,	RCC_PLL3CR,	6,	0),
	GATE_CFG(GATE_PLL4,		RCC_PLL4CR,	0,	0),
	GATE_CFG(GATE_PLL4_DIVP,	RCC_PLL4CR,	4,	0),
	GATE_CFG(GATE_PLL4_DIVQ,	RCC_PLL4CR,	5,	0),
	GATE_CFG(GATE_PLL4_DIVR,	RCC_PLL4CR,	6,	0),
	GATE_CFG(GATE_DDRC1,		RCC_DDRITFCR,	0,	0),
	GATE_CFG(GATE_DDRC1LP,		RCC_DDRITFCR,	1,	0),
	GATE_CFG(GATE_DDRPHYC,		RCC_DDRITFCR,	4,	0),
	GATE_CFG(GATE_DDRPHYCLP,	RCC_DDRITFCR,	5,	0),
	GATE_CFG(GATE_DDRCAPB,		RCC_DDRITFCR,	6,	0),
	GATE_CFG(GATE_DDRCAPBLP,	RCC_DDRITFCR,	7,	0),
	GATE_CFG(GATE_AXIDCG,		RCC_DDRITFCR,	8,	0),
	GATE_CFG(GATE_DDRPHYCAPB,	RCC_DDRITFCR,	9,	0),
	GATE_CFG(GATE_DDRPHYCAPBLP,	RCC_DDRITFCR,	10,	0),
	GATE_CFG(GATE_TIM2,		RCC_MP_APB1ENSETR,	0,	1),
	GATE_CFG(GATE_TIM3,		RCC_MP_APB1ENSETR,	1,	1),
	GATE_CFG(GATE_TIM4,		RCC_MP_APB1ENSETR,	2,	1),
	GATE_CFG(GATE_TIM5,		RCC_MP_APB1ENSETR,	3,	1),
	GATE_CFG(GATE_TIM6,		RCC_MP_APB1ENSETR,	4,	1),
	GATE_CFG(GATE_TIM7,		RCC_MP_APB1ENSETR,	5,	1),
	GATE_CFG(GATE_LPTIM1,		RCC_MP_APB1ENSETR,	9,	1),
	GATE_CFG(GATE_SPI2,		RCC_MP_APB1ENSETR,	11,	1),
	GATE_CFG(GATE_SPI3,		RCC_MP_APB1ENSETR,	12,	1),
	GATE_CFG(GATE_USART3,		RCC_MP_APB1ENSETR,	15,	1),
	GATE_CFG(GATE_UART4,		RCC_MP_APB1ENSETR,	16,	1),
	GATE_CFG(GATE_UART5,		RCC_MP_APB1ENSETR,	17,	1),
	GATE_CFG(GATE_UART7,		RCC_MP_APB1ENSETR,	18,	1),
	GATE_CFG(GATE_UART8,		RCC_MP_APB1ENSETR,	19,	1),
	GATE_CFG(GATE_I2C1,		RCC_MP_APB1ENSETR,	21,	1),
	GATE_CFG(GATE_I2C2,		RCC_MP_APB1ENSETR,	22,	1),
	GATE_CFG(GATE_SPDIF,		RCC_MP_APB1ENSETR,	26,	1),
	GATE_CFG(GATE_TIM1,		RCC_MP_APB2ENSETR,	0,	1),
	GATE_CFG(GATE_TIM8,		RCC_MP_APB2ENSETR,	1,	1),
	GATE_CFG(GATE_SPI1,		RCC_MP_APB2ENSETR,	8,	1),
	GATE_CFG(GATE_USART6,		RCC_MP_APB2ENSETR,	13,	1),
	GATE_CFG(GATE_SAI1,		RCC_MP_APB2ENSETR,	16,	1),
	GATE_CFG(GATE_SAI2,		RCC_MP_APB2ENSETR,	17,	1),
	GATE_CFG(GATE_DFSDM,		RCC_MP_APB2ENSETR,	20,	1),
	GATE_CFG(GATE_ADFSDM,		RCC_MP_APB2ENSETR,	21,	1),
	GATE_CFG(GATE_FDCAN,		RCC_MP_APB2ENSETR,	24,	1),
	GATE_CFG(GATE_LPTIM2,		RCC_MP_APB3ENSETR,	0,	1),
	GATE_CFG(GATE_LPTIM3,		RCC_MP_APB3ENSETR,	1,	1),
	GATE_CFG(GATE_LPTIM4,		RCC_MP_APB3ENSETR,	2,	1),
	GATE_CFG(GATE_LPTIM5,		RCC_MP_APB3ENSETR,	3,	1),
	GATE_CFG(GATE_VREF,		RCC_MP_APB3ENSETR,	13,	1),
	GATE_CFG(GATE_DTS,		RCC_MP_APB3ENSETR,	16,	1),
	GATE_CFG(GATE_PMBCTRL,		RCC_MP_APB3ENSETR,	17,	1),
	GATE_CFG(GATE_HDP,		RCC_MP_APB3ENSETR,	20,	1),
	GATE_CFG(GATE_SYSCFG,		RCC_MP_S_APB3ENSETR,	0,	1),
	GATE_CFG(GATE_DCMIPP,		RCC_MP_APB4ENSETR,	1,	1),
	GATE_CFG(GATE_DDRPERFM,		RCC_MP_APB4ENSETR,	8,	1),
	GATE_CFG(GATE_IWDG2APB,		RCC_MP_APB4ENSETR,	15,	1),
	GATE_CFG(GATE_USBPHY,		RCC_MP_APB4ENSETR,	16,	1),
	GATE_CFG(GATE_STGENRO,		RCC_MP_APB4ENSETR,	20,	1),
	GATE_CFG(GATE_LTDC,		RCC_MP_S_APB4ENSETR,	0,	1),
	GATE_CFG(GATE_RTCAPB,		RCC_MP_APB5ENSETR,	8,	1),
	GATE_CFG(GATE_TZC,		RCC_MP_APB5ENSETR,	11,	1),
	GATE_CFG(GATE_ETZPC,		RCC_MP_APB5ENSETR,	13,	1),
	GATE_CFG(GATE_IWDG1APB,		RCC_MP_APB5ENSETR,	15,	1),
	GATE_CFG(GATE_BSEC,		RCC_MP_APB5ENSETR,	16,	1),
	GATE_CFG(GATE_STGENC,		RCC_MP_APB5ENSETR,	20,	1),
	GATE_CFG(GATE_USART1,		RCC_MP_APB6ENSETR,	0,	1),
	GATE_CFG(GATE_USART2,		RCC_MP_APB6ENSETR,	1,	1),
	GATE_CFG(GATE_SPI4,		RCC_MP_APB6ENSETR,	2,	1),
	GATE_CFG(GATE_SPI5,		RCC_MP_APB6ENSETR,	3,	1),
	GATE_CFG(GATE_I2C3,		RCC_MP_APB6ENSETR,	4,	1),
	GATE_CFG(GATE_I2C4,		RCC_MP_APB6ENSETR,	5,	1),
	GATE_CFG(GATE_I2C5,		RCC_MP_APB6ENSETR,	6,	1),
	GATE_CFG(GATE_TIM12,		RCC_MP_APB6ENSETR,	7,	1),
	GATE_CFG(GATE_TIM13,		RCC_MP_APB6ENSETR,	8,	1),
	GATE_CFG(GATE_TIM14,		RCC_MP_APB6ENSETR,	9,	1),
	GATE_CFG(GATE_TIM15,		RCC_MP_APB6ENSETR,	10,	1),
	GATE_CFG(GATE_TIM16,		RCC_MP_APB6ENSETR,	11,	1),
	GATE_CFG(GATE_TIM17,		RCC_MP_APB6ENSETR,	12,	1),
	GATE_CFG(GATE_DMA1,		RCC_MP_AHB2ENSETR,	0,	1),
	GATE_CFG(GATE_DMA2,		RCC_MP_AHB2ENSETR,	1,	1),
	GATE_CFG(GATE_DMAMUX1,		RCC_MP_AHB2ENSETR,	2,	1),
	GATE_CFG(GATE_DMA3,		RCC_MP_AHB2ENSETR,	3,	1),
	GATE_CFG(GATE_DMAMUX2,		RCC_MP_AHB2ENSETR,	4,	1),
	GATE_CFG(GATE_ADC1,		RCC_MP_AHB2ENSETR,	5,	1),
	GATE_CFG(GATE_ADC2,		RCC_MP_AHB2ENSETR,	6,	1),
	GATE_CFG(GATE_USBO,		RCC_MP_AHB2ENSETR,	8,	1),
	GATE_CFG(GATE_TSC,		RCC_MP_AHB4ENSETR,	15,	1),

	GATE_CFG(GATE_GPIOA,		RCC_MP_S_AHB4ENSETR,	0,	1),
	GATE_CFG(GATE_GPIOB,		RCC_MP_S_AHB4ENSETR,	1,	1),
	GATE_CFG(GATE_GPIOC,		RCC_MP_S_AHB4ENSETR,	2,	1),
	GATE_CFG(GATE_GPIOD,		RCC_MP_S_AHB4ENSETR,	3,	1),
	GATE_CFG(GATE_GPIOE,		RCC_MP_S_AHB4ENSETR,	4,	1),
	GATE_CFG(GATE_GPIOF,		RCC_MP_S_AHB4ENSETR,	5,	1),
	GATE_CFG(GATE_GPIOG,		RCC_MP_S_AHB4ENSETR,	6,	1),
	GATE_CFG(GATE_GPIOH,		RCC_MP_S_AHB4ENSETR,	7,	1),
	GATE_CFG(GATE_GPIOI,		RCC_MP_S_AHB4ENSETR,	8,	1),

	GATE_CFG(GATE_PKA,		RCC_MP_AHB5ENSETR,	2,	1),
	GATE_CFG(GATE_SAES,		RCC_MP_AHB5ENSETR,	3,	1),
	GATE_CFG(GATE_CRYP1,		RCC_MP_AHB5ENSETR,	4,	1),
	GATE_CFG(GATE_HASH1,		RCC_MP_AHB5ENSETR,	5,	1),
	GATE_CFG(GATE_RNG1,		RCC_MP_AHB5ENSETR,	6,	1),
	GATE_CFG(GATE_BKPSRAM,		RCC_MP_AHB5ENSETR,	8,	1),
	GATE_CFG(GATE_AXIMC,		RCC_MP_AHB5ENSETR,	16,	1),
	GATE_CFG(GATE_MCE,		RCC_MP_AHB6ENSETR,	1,	1),
	GATE_CFG(GATE_ETH1CK,		RCC_MP_AHB6ENSETR,	7,	1),
	GATE_CFG(GATE_ETH1TX,		RCC_MP_AHB6ENSETR,	8,	1),
	GATE_CFG(GATE_ETH1RX,		RCC_MP_AHB6ENSETR,	9,	1),
	GATE_CFG(GATE_ETH1MAC,		RCC_MP_AHB6ENSETR,	10,	1),
	GATE_CFG(GATE_FMC,		RCC_MP_AHB6ENSETR,	12,	1),
	GATE_CFG(GATE_QSPI,		RCC_MP_AHB6ENSETR,	14,	1),
	GATE_CFG(GATE_SDMMC1,		RCC_MP_AHB6ENSETR,	16,	1),
	GATE_CFG(GATE_SDMMC2,		RCC_MP_AHB6ENSETR,	17,	1),
	GATE_CFG(GATE_CRC1,		RCC_MP_AHB6ENSETR,	20,	1),
	GATE_CFG(GATE_USBH,		RCC_MP_AHB6ENSETR,	24,	1),
	GATE_CFG(GATE_ETH2CK,		RCC_MP_AHB6ENSETR,	27,	1),
	GATE_CFG(GATE_ETH2TX,		RCC_MP_AHB6ENSETR,	28,	1),
	GATE_CFG(GATE_ETH2RX,		RCC_MP_AHB6ENSETR,	29,	1),
	GATE_CFG(GATE_ETH2MAC,		RCC_MP_AHB6ENSETR,	30,	1),
	GATE_CFG(GATE_MDMA,		RCC_MP_S_AHB6ENSETR,	0,	1),
};

/*
 * DIV CONFIG
 */

static const struct clk_div_table axi_div_table[] = {
	{ 0, 1 }, { 1, 2 }, { 2, 3 }, { 3, 4 },
	{ 4, 4 }, { 5, 4 }, { 6, 4 }, { 7, 4 },
	{ 0 },
};

static const struct clk_div_table mlahb_div_table[] = {
	{ 0, 1 }, { 1, 2 }, { 2, 4 }, { 3, 8 },
	{ 4, 16 }, { 5, 32 }, { 6, 64 }, { 7, 128 },
	{ 8, 256 }, { 9, 512 }, { 10, 512}, { 11, 512 },
	{ 12, 512 }, { 13, 512 }, { 14, 512}, { 15, 512 },
	{ 0 },
};

static const struct clk_div_table apb_div_table[] = {
	{ 0, 1 }, { 1, 2 }, { 2, 4 }, { 3, 8 },
	{ 4, 16 }, { 5, 16 }, { 6, 16 }, { 7, 16 },
	{ 0 },
};

#define DIV_CFG(id, _offset, _shift, _width, _flags, _table, _bitrdy)[id] = {\
		.offset	= _offset,\
		.shift	= _shift,\
		.width	= _width,\
		.flags	= _flags,\
		.table	= _table,\
		.bitrdy	= _bitrdy,\
}

static const struct div_cfg dividers_mp13[] = {
	DIV_CFG(DIV_PLL1DIVP, RCC_PLL1CFGR2, 0, 7, 0, NULL, DIV_NO_BIT_RDY),
	DIV_CFG(DIV_PLL2DIVP, RCC_PLL2CFGR2, 0, 7, 0, NULL, DIV_NO_BIT_RDY),
	DIV_CFG(DIV_PLL2DIVQ, RCC_PLL2CFGR2, 8, 7, 0, NULL, DIV_NO_BIT_RDY),
	DIV_CFG(DIV_PLL2DIVR, RCC_PLL2CFGR2, 16, 7, 0, NULL, DIV_NO_BIT_RDY),
	DIV_CFG(DIV_PLL3DIVP, RCC_PLL3CFGR2, 0, 7, 0, NULL, DIV_NO_BIT_RDY),
	DIV_CFG(DIV_PLL3DIVQ, RCC_PLL3CFGR2, 8, 7, 0, NULL, DIV_NO_BIT_RDY),
	DIV_CFG(DIV_PLL3DIVR, RCC_PLL3CFGR2, 16, 7, 0, NULL, DIV_NO_BIT_RDY),
	DIV_CFG(DIV_PLL4DIVP, RCC_PLL4CFGR2, 0, 7, 0, NULL, DIV_NO_BIT_RDY),
	DIV_CFG(DIV_PLL4DIVQ, RCC_PLL4CFGR2, 8, 7, 0, NULL, DIV_NO_BIT_RDY),
	DIV_CFG(DIV_PLL4DIVR, RCC_PLL4CFGR2, 16, 7, 0, NULL, DIV_NO_BIT_RDY),
	DIV_CFG(DIV_MPU, RCC_MPCKDIVR, 0, 4, 0, NULL, DIV_NO_BIT_RDY),
	DIV_CFG(DIV_AXI, RCC_AXIDIVR, 0, 3, 0, axi_div_table, 31),
	DIV_CFG(DIV_MLAHB, RCC_MLAHBDIVR, 0, 4, 0, mlahb_div_table, 31),
	DIV_CFG(DIV_APB1, RCC_APB1DIVR, 0, 3, 0, apb_div_table, 31),
	DIV_CFG(DIV_APB2, RCC_APB2DIVR, 0, 3, 0, apb_div_table, 31),
	DIV_CFG(DIV_APB3, RCC_APB3DIVR, 0, 3, 0, apb_div_table, 31),
	DIV_CFG(DIV_APB4, RCC_APB4DIVR, 0, 3, 0, apb_div_table, 31),
	DIV_CFG(DIV_APB5, RCC_APB5DIVR, 0, 3, 0, apb_div_table, 31),
	DIV_CFG(DIV_APB6, RCC_APB6DIVR, 0, 3, 0, apb_div_table, 31),
	DIV_CFG(DIV_RTC, RCC_RTCDIVR, 0, 6, 0, NULL, DIV_NO_BIT_RDY),
	DIV_CFG(DIV_MCO1, RCC_MCO1CFGR, 4, 4, 0, NULL, DIV_NO_BIT_RDY),
	DIV_CFG(DIV_MCO2, RCC_MCO2CFGR, 4, 4, 0, NULL, DIV_NO_BIT_RDY),

	DIV_CFG(DIV_HSI, RCC_HSICFGR, 0, 2, CLK_DIVIDER_POWER_OF_TWO, NULL, DIV_NO_BIT_RDY),
	DIV_CFG(DIV_TRACE, RCC_DBGCFGR, 0, 3, CLK_DIVIDER_POWER_OF_TWO, NULL, DIV_NO_BIT_RDY),

	DIV_CFG(DIV_ETH1PTP, RCC_ETH12CKSELR, 4, 4, 0, NULL, DIV_NO_BIT_RDY),
	DIV_CFG(DIV_ETH2PTP, RCC_ETH12CKSELR, 12, 4, 0, NULL, DIV_NO_BIT_RDY),
};

#define MAX_HSI_HZ		64000000
#define USB_PHY_48_MHZ		48000000

#define TIMEOUT_US_200MS	U(200000)
#define TIMEOUT_US_1S		U(1000000)

#define PLLRDY_TIMEOUT		TIMEOUT_US_200MS
#define CLKSRC_TIMEOUT		TIMEOUT_US_200MS
#define CLKDIV_TIMEOUT		TIMEOUT_US_200MS
#define HSIDIV_TIMEOUT		TIMEOUT_US_200MS
#define OSCRDY_TIMEOUT		TIMEOUT_US_1S

enum stm32_osc {
	OSC_HSI,
	OSC_HSE,
	OSC_CSI,
	OSC_LSI,
	OSC_LSE,
	OSC_I2SCKIN,
	NB_OSCILLATOR
};

enum stm32mp1_pll_id {
	_PLL1,
	_PLL2,
	_PLL3,
	_PLL4,
	_PLL_NB
};

enum stm32mp1_plltype {
	PLL_800,
	PLL_1600,
	PLL_2000,
	PLL_TYPE_NB
};

#define RCC_OFFSET_PLLXCR		0
#define RCC_OFFSET_PLLXCFGR1		4
#define RCC_OFFSET_PLLXCFGR2		8
#define RCC_OFFSET_PLLXFRACR		12
#define RCC_OFFSET_PLLXCSGR		16

struct stm32_clk_pll {
	enum stm32mp1_plltype plltype;
	uint16_t clk_id;
	uint16_t reg_pllxcr;
};

struct stm32mp1_pll {
	uint8_t refclk_min;
	uint8_t refclk_max;
};

/* Define characteristic of PLL according type */
static const struct stm32mp1_pll stm32mp1_pll[PLL_TYPE_NB] = {
	[PLL_800] = {
		.refclk_min = 4,
		.refclk_max = 16,
	},
	[PLL_1600] = {
		.refclk_min = 8,
		.refclk_max = 16,
	},
	[PLL_2000] = {
		.refclk_min = 8,
		.refclk_max = 16,
	},
};

#if STM32MP_USB_PROGRAMMER
static bool pll4_bootrom;
#endif

/* RCC clock device driver private */
static unsigned int refcounts_mp13[CK_LAST];

static const struct stm32_clk_pll *clk_st32_pll_data(unsigned int idx);

#if STM32MP_UART_PROGRAMMER || STM32MP_USB_PROGRAMMER
static void clk_oscillator_check_bypass(struct stm32_clk_priv *priv, int idx,
					bool digbyp, bool bypass)
{
	struct clk_oscillator_data *osc_data = clk_oscillator_get_data(priv, idx);
	struct stm32_clk_bypass *bypass_data = osc_data->bypass;
	uintptr_t address;

	if (bypass_data == NULL) {
		return;
	}

	address = priv->base + bypass_data->offset;
	if ((mmio_read_32(address) & RCC_OCENR_HSEBYP) &&
	    (!(digbyp || bypass))) {
		panic();
	}
}
#endif

static void stm32_enable_oscillator_hse(struct stm32_clk_priv *priv)
{
	struct stm32_clk_platdata *pdata = priv->pdata;
	struct stm32_osci_dt_cfg *osci = &pdata->osci[OSC_HSE];
	bool digbyp =  osci->digbyp;
	bool bypass = osci->bypass;
	bool css = osci->css;

	if (_clk_stm32_get_rate(priv, _CK_HSE) == 0U) {
		return;
	}

	clk_oscillator_set_bypass(priv, _CK_HSE, digbyp, bypass);

	_clk_stm32_enable(priv, _CK_HSE);

#if STM32MP_UART_PROGRAMMER || STM32MP_USB_PROGRAMMER
	clk_oscillator_check_bypass(priv, _CK_HSE, digbyp, bypass);
#endif

	clk_oscillator_set_css(priv, _CK_HSE, css);
}

static void stm32_enable_oscillator_lse(struct stm32_clk_priv *priv)
{
	struct clk_oscillator_data *osc_data = clk_oscillator_get_data(priv, _CK_LSE);
	struct stm32_clk_platdata *pdata = priv->pdata;
	struct stm32_osci_dt_cfg *osci = &pdata->osci[OSC_LSE];
	bool digbyp =  osci->digbyp;
	bool bypass = osci->bypass;
	uint8_t drive = osci->drive;

	if (_clk_stm32_get_rate(priv, _CK_LSE) == 0U) {
		return;
	}

	clk_oscillator_set_bypass(priv, _CK_LSE, digbyp, bypass);

	clk_oscillator_set_drive(priv, _CK_LSE, drive);

	_clk_stm32_gate_enable(priv, osc_data->gate_id);
}

static int stm32mp1_set_hsidiv(uint8_t hsidiv)
{
	uint64_t timeout;
	uintptr_t rcc_base = stm32mp_rcc_base();
	uintptr_t address = rcc_base + RCC_OCRDYR;

	mmio_clrsetbits_32(rcc_base + RCC_HSICFGR,
			   RCC_HSICFGR_HSIDIV_MASK,
			   RCC_HSICFGR_HSIDIV_MASK & (uint32_t)hsidiv);

	timeout = timeout_init_us(HSIDIV_TIMEOUT);
	while ((mmio_read_32(address) & RCC_OCRDYR_HSIDIVRDY) == 0U) {
		if (timeout_elapsed(timeout)) {
			ERROR("HSIDIV failed @ 0x%lx: 0x%x\n",
			      address, mmio_read_32(address));
			return -ETIMEDOUT;
		}
	}

	return 0;
}

static int stm32mp1_hsidiv(unsigned long hsifreq)
{
	uint8_t hsidiv;
	uint32_t hsidivfreq = MAX_HSI_HZ;

	for (hsidiv = 0; hsidiv < 4U; hsidiv++) {
		if (hsidivfreq == hsifreq) {
			break;
		}

		hsidivfreq /= 2U;
	}

	if (hsidiv == 4U) {
		ERROR("Invalid clk-hsi frequency\n");
		return -EINVAL;
	}

	if (hsidiv != 0U) {
		return stm32mp1_set_hsidiv(hsidiv);
	}

	return 0;
}

static int stm32_clk_oscillators_lse_set_css(struct stm32_clk_priv *priv)
{
	struct stm32_clk_platdata *pdata = priv->pdata;
	struct stm32_osci_dt_cfg *osci = &pdata->osci[OSC_LSE];

	clk_oscillator_set_css(priv, _CK_LSE, osci->css);

	return 0;
}

static int stm32mp1_come_back_to_hsi(void)
{
	int ret;
	struct stm32_clk_priv *priv = clk_stm32_get_priv();

	/* Come back to HSI */
	ret = _clk_stm32_set_parent(priv, _CKMPU, _CK_HSI);
	if (ret != 0) {
		return ret;
	}

	ret = _clk_stm32_set_parent(priv, _CKAXI, _CK_HSI);
	if (ret != 0) {
		return ret;
	}

	ret = _clk_stm32_set_parent(priv, _CKMLAHB, _CK_HSI);
	if (ret != 0) {
		return ret;
	}

	return 0;
}

static int stm32_clk_configure_clk_get_binding_id(struct stm32_clk_priv *priv, uint32_t data)
{
	unsigned long binding_id = ((unsigned long)data & CLK_ID_MASK) >> CLK_ID_SHIFT;

	return clk_get_index(priv, binding_id);
}

static int stm32_clk_configure_clk(struct stm32_clk_priv *priv, uint32_t data)
{
	int sel = (data & CLK_SEL_MASK) >> CLK_SEL_SHIFT;
	int enable = (data & CLK_ON_MASK) >> CLK_ON_SHIFT;
	int clk_id;
	int ret;

	clk_id = stm32_clk_configure_clk_get_binding_id(priv, data);
	if (clk_id < 0) {
		return clk_id;
	}

	ret = _clk_stm32_set_parent_by_index(priv, clk_id, sel);
	if (ret != 0) {
		return ret;
	}

	if (enable) {
		clk_stm32_enable_call_ops(priv, clk_id);
	} else {
		clk_stm32_disable_call_ops(priv, clk_id);
	}

	return 0;
}

static int stm32_clk_configure_mux(struct stm32_clk_priv *priv, uint32_t data)
{
	int mux = (data & MUX_ID_MASK) >> MUX_ID_SHIFT;
	int sel = (data & MUX_SEL_MASK) >> MUX_SEL_SHIFT;

	return clk_mux_set_parent(priv, mux, sel);
}

static int stm32_clk_dividers_configure(struct stm32_clk_priv *priv)
{
	struct stm32_clk_platdata *pdata = priv->pdata;
	uint32_t i;

	for (i = 0; i < pdata->nclkdiv; i++) {
		int div_id, div_n;
		int val;
		int ret;

		val = pdata->clkdiv[i] & CMD_DATA_MASK;
		div_id = (val & DIV_ID_MASK) >> DIV_ID_SHIFT;
		div_n = (val & DIV_DIVN_MASK) >> DIV_DIVN_SHIFT;

		ret = clk_stm32_set_div(priv, div_id, div_n);
		if (ret != 0) {
			return ret;
		}
	}

	return 0;
}

static int stm32_clk_source_configure(struct stm32_clk_priv *priv)
{
	struct stm32_clk_platdata *pdata = priv->pdata;
	bool ckper_disabled = false;
	int clk_id;
	int ret;
	uint32_t i;

	for (i = 0; i < pdata->nclksrc; i++) {
		uint32_t val = pdata->clksrc[i];
		uint32_t cmd, cmd_data;

		if (val == (uint32_t)CLK_CKPER_DISABLED) {
			ckper_disabled = true;
			continue;
		}

		if (val == (uint32_t)CLK_RTC_DISABLED) {
			continue;
		}

		cmd = (val & CMD_MASK) >> CMD_SHIFT;
		cmd_data = val & ~CMD_MASK;

		switch (cmd) {
		case CMD_MUX:
			ret = stm32_clk_configure_mux(priv, cmd_data);
			break;

		case CMD_CLK:
			clk_id = stm32_clk_configure_clk_get_binding_id(priv, cmd_data);

			if (clk_id == _RTCCK) {
				if ((_clk_stm32_is_enabled(priv, _RTCCK) == true)) {
					continue;
				}
			}

			ret = stm32_clk_configure_clk(priv, cmd_data);
			break;
		default:
			ret = -EINVAL;
			break;
		}

		if (ret != 0) {
			return ret;
		}
	}

	/*
	 * CKPER is source for some peripheral clocks
	 * (FMC-NAND / QPSI-NOR) and switching source is allowed
	 * only if previous clock is still ON
	 * => deactivate CKPER only after switching clock
	 */
	if (ckper_disabled) {
		ret = stm32_clk_configure_mux(priv, CLK_CKPER_DISABLED & CMD_MASK);
		if (ret != 0) {
			return ret;
		}
	}

	return 0;
}

static int stm32_clk_stgen_configure(struct stm32_clk_priv *priv, int id)
{
	unsigned long stgen_freq;

	stgen_freq = _clk_stm32_get_rate(priv, id);

	stm32mp_stgen_config(stgen_freq);

	return 0;
}

#define CLK_PLL_CFG(_idx, _clk_id, _type, _reg)\
	[(_idx)] = {\
		.clk_id = (_clk_id),\
		.plltype = (_type),\
		.reg_pllxcr = (_reg),\
	}

static int clk_stm32_pll_compute_cfgr1(struct stm32_clk_priv *priv,
				       const struct stm32_clk_pll *pll,
				       struct stm32_pll_vco *vco,
				       uint32_t *value)
{
	uint32_t divm = vco->div_mn[PLL_CFG_M];
	uint32_t divn = vco->div_mn[PLL_CFG_N];
	unsigned long prate = 0UL;
	unsigned long refclk = 0UL;

	prate = _clk_stm32_get_parent_rate(priv, pll->clk_id);
	refclk = prate / (divm + 1U);

	if ((refclk < (stm32mp1_pll[pll->plltype].refclk_min * 1000000U)) ||
	    (refclk > (stm32mp1_pll[pll->plltype].refclk_max * 1000000U))) {
		return -EINVAL;
	}

	*value = 0;

	if ((pll->plltype == PLL_800) && (refclk >= 8000000U)) {
		*value = 1U << RCC_PLLNCFGR1_IFRGE_SHIFT;
	}

	*value |= (divn << RCC_PLLNCFGR1_DIVN_SHIFT) & RCC_PLLNCFGR1_DIVN_MASK;
	*value |= (divm << RCC_PLLNCFGR1_DIVM_SHIFT) & RCC_PLLNCFGR1_DIVM_MASK;

	return 0;
}

static uint32_t  clk_stm32_pll_compute_cfgr2(struct stm32_pll_output *out)
{
	uint32_t value = 0;

	value |= (out->output[PLL_CFG_P] << RCC_PLLNCFGR2_DIVP_SHIFT) & RCC_PLLNCFGR2_DIVP_MASK;
	value |= (out->output[PLL_CFG_Q] << RCC_PLLNCFGR2_DIVQ_SHIFT) & RCC_PLLNCFGR2_DIVQ_MASK;
	value |= (out->output[PLL_CFG_R] << RCC_PLLNCFGR2_DIVR_SHIFT) & RCC_PLLNCFGR2_DIVR_MASK;

	return value;
}

static void clk_stm32_pll_config_vco(struct stm32_clk_priv *priv,
				     const struct stm32_clk_pll *pll,
				     struct stm32_pll_vco *vco)
{
	uintptr_t pll_base = priv->base + pll->reg_pllxcr;
	uint32_t value = 0;

	if (clk_stm32_pll_compute_cfgr1(priv, pll, vco, &value) != 0) {
		ERROR("Invalid Vref clock !\n");
		panic();
	}

	/* Write N / M / IFREGE fields */
	mmio_write_32(pll_base + RCC_OFFSET_PLLXCFGR1, value);

	/* Fractional configuration */
	mmio_write_32(pll_base + RCC_OFFSET_PLLXFRACR, 0);

	/* Frac must be enabled only once its configuration is loaded */
	mmio_write_32(pll_base + RCC_OFFSET_PLLXFRACR, vco->frac << RCC_PLLNFRACR_FRACV_SHIFT);
	mmio_setbits_32(pll_base + RCC_OFFSET_PLLXFRACR, RCC_PLLNFRACR_FRACLE);
}

static void clk_stm32_pll_config_csg(struct stm32_clk_priv *priv,
				     const struct stm32_clk_pll *pll,
				     struct stm32_pll_vco *vco)
{
	uintptr_t pll_base = priv->base + pll->reg_pllxcr;
	uint32_t mod_per = 0;
	uint32_t inc_step = 0;
	uint32_t sscg_mode = 0;
	uint32_t value = 0;

	if (!vco->csg_enabled) {
		return;
	}

	mod_per = vco->csg[PLL_CSG_MOD_PER];
	inc_step = vco->csg[PLL_CSG_INC_STEP];
	sscg_mode = vco->csg[PLL_CSG_SSCG_MODE];

	value |= (mod_per << RCC_PLLNCSGR_MOD_PER_SHIFT) & RCC_PLLNCSGR_MOD_PER_MASK;
	value |= (inc_step << RCC_PLLNCSGR_INC_STEP_SHIFT) & RCC_PLLNCSGR_INC_STEP_MASK;
	value |= (sscg_mode << RCC_PLLNCSGR_SSCG_MODE_SHIFT) & RCC_PLLNCSGR_SSCG_MODE_MASK;

	mmio_write_32(pll_base + RCC_OFFSET_PLLXCSGR, value);
	mmio_setbits_32(pll_base + RCC_OFFSET_PLLXCR, RCC_PLLNCR_SSCG_CTRL);
}

static void clk_stm32_pll_config_out(struct stm32_clk_priv *priv, const struct stm32_clk_pll *pll,
				     struct stm32_pll_output *out)
{
	uintptr_t pll_base = priv->base + pll->reg_pllxcr;
	uint32_t value = 0;

	value = clk_stm32_pll_compute_cfgr2(out);

	mmio_write_32(pll_base + RCC_OFFSET_PLLXCFGR2, value);
}

static inline struct stm32_pll_dt_cfg *clk_stm32_pll_get_pdata(int pll_idx)
{
	struct stm32_clk_priv *priv = clk_stm32_get_priv();
	struct stm32_clk_platdata *pdata = priv->pdata;

	return &pdata->pll[pll_idx];
}

static bool _clk_stm32_pll_is_enabled(struct stm32_clk_priv *priv, const struct stm32_clk_pll *pll)
{
	uintptr_t pll_base = priv->base + pll->reg_pllxcr;

	return ((mmio_read_32(pll_base) & RCC_PLLNCR_PLLON) != 0U);
}

static void _clk_stm32_pll_set_on(struct stm32_clk_priv *priv, const struct stm32_clk_pll *pll)
{
	uintptr_t pll_base = priv->base + pll->reg_pllxcr;

	/* Preserve RCC_PLLNCR_SSCG_CTRL value */
	mmio_clrsetbits_32(pll_base, RCC_PLLNCR_DIVPEN | RCC_PLLNCR_DIVQEN | RCC_PLLNCR_DIVREN,
			   RCC_PLLNCR_PLLON);
}

static void _clk_stm32_pll_set_off(struct stm32_clk_priv *priv, const struct stm32_clk_pll *pll)
{
	uintptr_t pll_base = priv->base + pll->reg_pllxcr;

	/* Stop all output */
	mmio_clrbits_32(pll_base, RCC_PLLNCR_DIVPEN | RCC_PLLNCR_DIVQEN | RCC_PLLNCR_DIVREN);

	/* Stop PLL */
	mmio_clrbits_32(pll_base, RCC_PLLNCR_PLLON);
}

static int _clk_stm32_pll_wait_ready_on(struct stm32_clk_priv *priv,
					const struct stm32_clk_pll *pll)
{
	uintptr_t pll_base = priv->base + pll->reg_pllxcr;
	uint64_t timeout = timeout_init_us(PLLRDY_TIMEOUT);

	/* Wait PLL lock */
	while ((mmio_read_32(pll_base) & RCC_PLLNCR_PLLRDY) == 0U) {
		if (timeout_elapsed(timeout)) {
			ERROR("%d clock start failed @ 0x%x: 0x%x\n",
			      pll->clk_id, pll->reg_pllxcr, mmio_read_32(pll_base));
			return -EINVAL;
		}
	}

	return 0;
}

static int _clk_stm32_pll_wait_ready_off(struct stm32_clk_priv *priv,
					 const struct stm32_clk_pll *pll)
{
	uintptr_t pll_base = priv->base + pll->reg_pllxcr;
	uint64_t timeout = timeout_init_us(PLLRDY_TIMEOUT);

	/* Wait PLL lock */
	while ((mmio_read_32(pll_base) & RCC_PLLNCR_PLLRDY) != 0U) {
		if (timeout_elapsed(timeout)) {
			ERROR("%d clock stop failed @ 0x%x: 0x%x\n",
			      pll->clk_id, pll->reg_pllxcr, mmio_read_32(pll_base));
			return -EINVAL;
		}
	}

	return 0;
}

static int _clk_stm32_pll_enable(struct stm32_clk_priv *priv, const struct stm32_clk_pll *pll)
{
	if (_clk_stm32_pll_is_enabled(priv, pll)) {
		return 0;
	}

	/* Preserve RCC_PLLNCR_SSCG_CTRL value */
	_clk_stm32_pll_set_on(priv, pll);

	/* Wait PLL lock */
	return _clk_stm32_pll_wait_ready_on(priv, pll);
}

static void _clk_stm32_pll_disable(struct stm32_clk_priv *priv, const struct stm32_clk_pll *pll)
{
	if (!_clk_stm32_pll_is_enabled(priv, pll)) {
		return;
	}

	/* Stop all outputs and the PLL */
	_clk_stm32_pll_set_off(priv, pll);

	/* Wait PLL stopped */
	_clk_stm32_pll_wait_ready_off(priv, pll);
}

static int _clk_stm32_pll_init(struct stm32_clk_priv *priv, int pll_idx,
			       struct stm32_pll_dt_cfg *pll_conf)
{
	const struct stm32_clk_pll *pll = clk_st32_pll_data(pll_idx);
	uintptr_t pll_base = priv->base + pll->reg_pllxcr;
	int ret = 0;

	/* Configure PLLs source */
	ret = stm32_clk_configure_mux(priv, pll_conf->vco.src);
	if (ret) {
		return ret;
	}

#if STM32MP_USB_PROGRAMMER
	if ((pll_idx == _PLL4) && pll4_bootrom) {
		clk_stm32_pll_config_out(priv, pll, &pll_conf->output);

		mmio_setbits_32(pll_base,
				RCC_PLLNCR_DIVPEN | RCC_PLLNCR_DIVQEN | RCC_PLLNCR_DIVREN);

		return 0;
	}
#endif
	/* Stop the PLL before */
	_clk_stm32_pll_disable(priv, pll);

	clk_stm32_pll_config_vco(priv, pll, &pll_conf->vco);
	clk_stm32_pll_config_out(priv, pll, &pll_conf->output);
	clk_stm32_pll_config_csg(priv, pll, &pll_conf->vco);

	ret = _clk_stm32_pll_enable(priv, pll);
	if (ret != 0) {
		return ret;
	}

	mmio_setbits_32(pll_base, RCC_PLLNCR_DIVPEN | RCC_PLLNCR_DIVQEN | RCC_PLLNCR_DIVREN);

	return 0;
}

static int clk_stm32_pll_init(struct stm32_clk_priv *priv, int pll_idx)
{
	struct stm32_pll_dt_cfg *pll_conf = clk_stm32_pll_get_pdata(pll_idx);

	if (pll_conf->vco.status) {
		return _clk_stm32_pll_init(priv, pll_idx, pll_conf);
	}

	return 0;
}

static int stm32_clk_pll_configure(struct stm32_clk_priv *priv)
{
	int err = 0;

	err = clk_stm32_pll_init(priv, _PLL1);
	if (err) {
		return err;
	}

	err = clk_stm32_pll_init(priv, _PLL2);
	if (err) {
		return err;
	}

	err = clk_stm32_pll_init(priv, _PLL3);
	if (err) {
		return err;
	}

	err = clk_stm32_pll_init(priv, _PLL4);
	if (err) {
		return err;
	}

	return 0;
}

static int stm32_clk_oscillators_wait_lse_ready(struct stm32_clk_priv *priv)
{
	int ret = 0;

	if (_clk_stm32_get_rate(priv, _CK_LSE) != 0U) {
		ret = clk_oscillator_wait_ready_on(priv, _CK_LSE);
	}

	return ret;
}

static void stm32_clk_oscillators_enable(struct stm32_clk_priv *priv)
{
	stm32_enable_oscillator_hse(priv);
	stm32_enable_oscillator_lse(priv);
	_clk_stm32_enable(priv, _CK_LSI);
	_clk_stm32_enable(priv, _CK_CSI);
}

static int stm32_clk_hsidiv_configure(struct stm32_clk_priv *priv)
{
	return stm32mp1_hsidiv(_clk_stm32_get_rate(priv, _CK_HSI));
}

#if STM32MP_USB_PROGRAMMER
static bool stm32mp1_clk_is_pll4_used_by_bootrom(struct stm32_clk_priv *priv, int usbphy_p)
{
	/* Don't initialize PLL4, when used by BOOTROM */
	if ((stm32mp_get_boot_itf_selected() ==
	     BOOT_API_CTX_BOOT_INTERFACE_SEL_SERIAL_USB) &&
	    (usbphy_p == _PLL4R)) {
		return true;
	}

	return false;
}

static int stm32mp1_clk_check_usb_conflict(struct stm32_clk_priv *priv, int usbphy_p, int usbo_p)
{
	int _usbo_p;
	int _usbphy_p;

	if (!pll4_bootrom) {
		return 0;
	}

	_usbo_p = _clk_stm32_get_parent(priv, _USBO_K);
	_usbphy_p = _clk_stm32_get_parent(priv, _USBPHY_K);

	if ((_usbo_p != usbo_p) || (_usbphy_p != usbphy_p)) {
		return -FDT_ERR_BADVALUE;
	}

	return 0;
}
#endif

static struct clk_oscillator_data stm32mp13_osc_data[NB_OSCILLATOR] = {
	OSCILLATOR(OSC_HSI, _CK_HSI, "clk-hsi", GATE_HSI, GATE_HSI_RDY,
		   NULL, NULL, NULL),

	OSCILLATOR(OSC_LSI, _CK_LSI, "clk-lsi", GATE_LSI, GATE_LSI_RDY,
		   NULL, NULL, NULL),

	OSCILLATOR(OSC_CSI, _CK_CSI, "clk-csi", GATE_CSI, GATE_CSI_RDY,
		   NULL, NULL, NULL),

	OSCILLATOR(OSC_LSE, _CK_LSE, "clk-lse", GATE_LSE, GATE_LSE_RDY,
		   BYPASS(RCC_BDCR, 1, 3),
		   CSS(RCC_BDCR, 8),
		   DRIVE(RCC_BDCR, 4, 2, 2)),

	OSCILLATOR(OSC_HSE, _CK_HSE, "clk-hse", GATE_HSE, GATE_HSE_RDY,
		   BYPASS(RCC_OCENSETR, 10, 7),
		   CSS(RCC_OCENSETR, 11),
		   NULL),

	OSCILLATOR(OSC_I2SCKIN, _I2SCKIN, "i2s_ckin", NO_GATE, NO_GATE,
		   NULL, NULL, NULL),
};

static const char *clk_stm32_get_oscillator_name(enum stm32_osc id)
{
	if (id < NB_OSCILLATOR) {
		return stm32mp13_osc_data[id].name;
	}

	return NULL;
}

#define CLK_PLL_CFG(_idx, _clk_id, _type, _reg)\
	[(_idx)] = {\
		.clk_id = (_clk_id),\
		.plltype = (_type),\
		.reg_pllxcr = (_reg),\
	}

static const struct stm32_clk_pll stm32_mp13_clk_pll[_PLL_NB] = {
	CLK_PLL_CFG(_PLL1, _CK_PLL1, PLL_2000, RCC_PLL1CR),
	CLK_PLL_CFG(_PLL2, _CK_PLL2, PLL_1600, RCC_PLL2CR),
	CLK_PLL_CFG(_PLL3, _CK_PLL3, PLL_800, RCC_PLL3CR),
	CLK_PLL_CFG(_PLL4, _CK_PLL4, PLL_800, RCC_PLL4CR),
};

static const struct stm32_clk_pll *clk_st32_pll_data(unsigned int idx)
{
	return &stm32_mp13_clk_pll[idx];
}

struct stm32_pll_cfg {
	int pll_id;
};

static unsigned long clk_stm32_pll_recalc_rate(struct stm32_clk_priv *priv,  int id,
					       unsigned long prate)
{
	const struct clk_stm32 *clk = _clk_get(priv, id);
	struct stm32_pll_cfg *pll_cfg = clk->clock_cfg;
	const struct stm32_clk_pll *pll = clk_st32_pll_data(pll_cfg->pll_id);
	uintptr_t pll_base = priv->base + pll->reg_pllxcr;
	uint32_t cfgr1, fracr, divm, divn;
	unsigned long fvco;

	cfgr1 = mmio_read_32(pll_base + RCC_OFFSET_PLLXCFGR1);
	fracr = mmio_read_32(pll_base + RCC_OFFSET_PLLXFRACR);

	divm = (cfgr1 & (RCC_PLLNCFGR1_DIVM_MASK)) >> RCC_PLLNCFGR1_DIVM_SHIFT;
	divn = cfgr1 & RCC_PLLNCFGR1_DIVN_MASK;

	/*
	 * With FRACV :
	 *   Fvco = Fck_ref * ((DIVN + 1) + FRACV / 2^13) / (DIVM + 1)
	 * Without FRACV
	 *   Fvco = Fck_ref * ((DIVN + 1) / (DIVM + 1)
	 */
	if ((fracr & RCC_PLLNFRACR_FRACLE) != 0U) {
		uint32_t fracv = (fracr & RCC_PLLNFRACR_FRACV_MASK) >>
				 RCC_PLLNFRACR_FRACV_SHIFT;
		unsigned long long numerator, denominator;

		numerator = (((unsigned long long)divn + 1U) << 13) + fracv;
		numerator = prate * numerator;
		denominator = ((unsigned long long)divm + 1U) << 13;
		fvco = (unsigned long)(numerator / denominator);
	} else {
		fvco = (unsigned long)(prate * (divn + 1U) / (divm + 1U));
	}

	return fvco;
};

static bool clk_stm32_pll_is_enabled(struct stm32_clk_priv *priv, int id)
{
	const struct clk_stm32 *clk = _clk_get(priv, id);
	struct stm32_pll_cfg *pll_cfg = clk->clock_cfg;
	const struct stm32_clk_pll *pll = clk_st32_pll_data(pll_cfg->pll_id);

	return _clk_stm32_pll_is_enabled(priv, pll);
}

static int clk_stm32_pll_enable(struct stm32_clk_priv *priv, int id)
{
	const struct clk_stm32 *clk = _clk_get(priv, id);
	struct stm32_pll_cfg *pll_cfg = clk->clock_cfg;
	const struct stm32_clk_pll *pll = clk_st32_pll_data(pll_cfg->pll_id);

	return _clk_stm32_pll_enable(priv, pll);
}

static void clk_stm32_pll_disable(struct stm32_clk_priv *priv, int id)
{
	const struct clk_stm32 *clk = _clk_get(priv, id);
	struct stm32_pll_cfg *pll_cfg = clk->clock_cfg;
	const struct stm32_clk_pll *pll = clk_st32_pll_data(pll_cfg->pll_id);

	_clk_stm32_pll_disable(priv, pll);
}

static const struct stm32_clk_ops clk_stm32_pll_ops = {
	.recalc_rate	= clk_stm32_pll_recalc_rate,
	.enable		= clk_stm32_pll_enable,
	.disable	= clk_stm32_pll_disable,
	.is_enabled	= clk_stm32_pll_is_enabled,
};

#define CLK_PLL(idx, _idx, _parent, _gate, _pll_id, _flags)[idx] = {\
	.name = #idx,\
	.binding = _idx,\
	.parent = _parent,\
	.flags = (_flags),\
	.clock_cfg	= &(struct stm32_pll_cfg) {\
		.pll_id = _pll_id,\
	},\
	.ops = &clk_stm32_pll_ops,\
}

struct clk_stm32_composite_cfg {
	int gate_id;
	int div_id;
};

static unsigned long clk_stm32_composite_recalc_rate(struct stm32_clk_priv *priv,
						     int idx, unsigned long prate)
{
	const struct clk_stm32 *clk = _clk_get(priv, idx);
	struct clk_stm32_composite_cfg *composite_cfg = clk->clock_cfg;

	return _clk_stm32_divider_recalc(priv, composite_cfg->div_id, prate);
};

static bool clk_stm32_composite_gate_is_enabled(struct stm32_clk_priv *priv, int idx)
{
	const struct clk_stm32 *clk = _clk_get(priv, idx);
	struct clk_stm32_composite_cfg *composite_cfg = clk->clock_cfg;

	return _clk_stm32_gate_is_enabled(priv, composite_cfg->gate_id);
}

static int clk_stm32_composite_gate_enable(struct stm32_clk_priv *priv, int idx)
{
	const struct clk_stm32 *clk = _clk_get(priv, idx);
	struct clk_stm32_composite_cfg *composite_cfg = clk->clock_cfg;

	return _clk_stm32_gate_enable(priv, composite_cfg->gate_id);
}

static void clk_stm32_composite_gate_disable(struct stm32_clk_priv *priv, int idx)
{
	const struct clk_stm32 *clk = _clk_get(priv, idx);
	struct clk_stm32_composite_cfg *composite_cfg = clk->clock_cfg;

	_clk_stm32_gate_disable(priv, composite_cfg->gate_id);
}

static const struct stm32_clk_ops clk_stm32_composite_ops = {
	.recalc_rate = clk_stm32_composite_recalc_rate,
	.is_enabled = clk_stm32_composite_gate_is_enabled,
	.enable = clk_stm32_composite_gate_enable,
	.disable = clk_stm32_composite_gate_disable,
};

#define STM32_COMPOSITE(idx, _binding, _parent, _flags, _gate_id,\
			_div_id)[idx] = {\
	.name = #idx,\
	.binding = (_binding),\
	.parent =  (_parent),\
	.flags = (_flags),\
	.clock_cfg	= &(struct clk_stm32_composite_cfg) {\
		.gate_id	= (_gate_id),\
		.div_id	= (_div_id),\
	},\
	.ops = &clk_stm32_composite_ops,\
}

static const struct clk_stm32 stm32mp13_clk[CK_LAST] = {
	/* ROOT CLOCKS */
	CLK_FIXED_RATE(_CK_OFF, _NO_ID, 0),
	CLK_OSC(_CK_HSE, CK_HSE, CLK_IS_ROOT, OSC_HSE),
	CLK_OSC(_CK_HSI, CK_HSI, CLK_IS_ROOT, OSC_HSI),
	CLK_OSC(_CK_CSI, CK_CSI, CLK_IS_ROOT, OSC_CSI),
	CLK_OSC(_CK_LSI, CK_LSI, CLK_IS_ROOT, OSC_LSI),
	CLK_OSC(_CK_LSE, CK_LSE, CLK_IS_ROOT, OSC_LSE),

	CLK_OSC_FIXED(_I2SCKIN, _NO_ID, CLK_IS_ROOT, OSC_I2SCKIN),

	CLK_FIXED_RATE(_USB_PHY_48, _NO_ID, USB_PHY_48_MHZ),

	STM32_DIV(_HSE_DIV, _NO_ID, _CK_HSE, 0, DIV_RTC),

	FIXED_FACTOR(_HSE_DIV2, CK_HSE_DIV2, _CK_HSE, 1, 2),
	FIXED_FACTOR(_CSI_DIV122, _NO_ID, _CK_CSI, 1, 122),

	CLK_PLL(_CK_PLL1, PLL1, MUX(MUX_PLL12), GATE_PLL1, _PLL1, 0),
	CLK_PLL(_CK_PLL2, PLL2, MUX(MUX_PLL12), GATE_PLL2, _PLL2, 0),
	CLK_PLL(_CK_PLL3, PLL3, MUX(MUX_PLL3), GATE_PLL3, _PLL3, 0),
	CLK_PLL(_CK_PLL4, PLL4, MUX(MUX_PLL4), GATE_PLL4, _PLL4, 0),

	STM32_COMPOSITE(_PLL1P, PLL1_P, _CK_PLL1, CLK_IS_CRITICAL, GATE_PLL1_DIVP, DIV_PLL1DIVP),
	STM32_DIV(_PLL1P_DIV, _NO_ID, _CK_PLL1, 0, DIV_MPU),

	STM32_COMPOSITE(_PLL2P, PLL2_P, _CK_PLL2, CLK_IS_CRITICAL, GATE_PLL2_DIVP, DIV_PLL2DIVP),
	STM32_COMPOSITE(_PLL2Q, PLL2_Q, _CK_PLL2, 0, GATE_PLL2_DIVQ, DIV_PLL2DIVQ),
	STM32_COMPOSITE(_PLL2R, PLL2_R, _CK_PLL2, CLK_IS_CRITICAL, GATE_PLL2_DIVR, DIV_PLL2DIVR),

	STM32_COMPOSITE(_PLL3P, PLL3_P, _CK_PLL3, 0, GATE_PLL3_DIVP, DIV_PLL3DIVP),
	STM32_COMPOSITE(_PLL3Q, PLL3_Q, _CK_PLL3, 0, GATE_PLL3_DIVQ, DIV_PLL3DIVQ),
	STM32_COMPOSITE(_PLL3R, PLL3_R, _CK_PLL3, 0, GATE_PLL3_DIVR, DIV_PLL3DIVR),

	STM32_COMPOSITE(_PLL4P, PLL4_P, _CK_PLL4, 0, GATE_PLL4_DIVP, DIV_PLL4DIVP),
	STM32_COMPOSITE(_PLL4Q, PLL4_Q, _CK_PLL4, 0, GATE_PLL4_DIVQ, DIV_PLL4DIVQ),
	STM32_COMPOSITE(_PLL4R, PLL4_R, _CK_PLL4, 0, GATE_PLL4_DIVR, DIV_PLL4DIVR),

	STM32_MUX(_CKMPU, CK_MPU, MUX_MPU, 0),
	STM32_DIV(_CKAXI, CK_AXI, MUX(MUX_AXI), 0, DIV_AXI),
	STM32_DIV(_CKMLAHB, CK_MLAHB, MUX(MUX_MLAHB), CLK_IS_CRITICAL, DIV_MLAHB),
	STM32_MUX(_CKPER, CK_PER, MUX(MUX_CKPER), 0),

	STM32_DIV(_PCLK1, PCLK1, _CKMLAHB, 0, DIV_APB1),
	STM32_DIV(_PCLK2, PCLK2, _CKMLAHB, 0, DIV_APB2),
	STM32_DIV(_PCLK3, PCLK3, _CKMLAHB, 0, DIV_APB3),
	STM32_DIV(_PCLK4, PCLK4, _CKAXI, 0, DIV_APB4),
	STM32_DIV(_PCLK5, PCLK5, _CKAXI, 0, DIV_APB5),
	STM32_DIV(_PCLK6, PCLK6, _CKMLAHB, 0, DIV_APB6),

	CK_TIMER(_CKTIMG1, CK_TIMG1, _PCLK1, 0, RCC_APB1DIVR, RCC_TIMG1PRER),
	CK_TIMER(_CKTIMG2, CK_TIMG2, _PCLK2, 0, RCC_APB2DIVR, RCC_TIMG2PRER),
	CK_TIMER(_CKTIMG3, CK_TIMG3, _PCLK6, 0, RCC_APB6DIVR, RCC_TIMG3PRER),

	/* END ROOT CLOCKS */

	STM32_GATE(_DDRC1, DDRC1, _CKAXI, CLK_IS_CRITICAL, GATE_DDRC1),
	STM32_GATE(_DDRC1LP, DDRC1LP, _CKAXI, CLK_IS_CRITICAL, GATE_DDRC1LP),
	STM32_GATE(_DDRPHYC, DDRPHYC, _PLL2R, CLK_IS_CRITICAL, GATE_DDRPHYC),
	STM32_GATE(_DDRPHYCLP, DDRPHYCLP, _PLL2R, CLK_IS_CRITICAL, GATE_DDRPHYCLP),
	STM32_GATE(_DDRCAPB, DDRCAPB, _PCLK4, CLK_IS_CRITICAL, GATE_DDRCAPB),
	STM32_GATE(_DDRCAPBLP, DDRCAPBLP, _PCLK4, CLK_IS_CRITICAL, GATE_DDRCAPBLP),
	STM32_GATE(_AXIDCG, AXIDCG, _CKAXI, CLK_IS_CRITICAL, GATE_AXIDCG),
	STM32_GATE(_DDRPHYCAPB, DDRPHYCAPB, _PCLK4, CLK_IS_CRITICAL, GATE_DDRPHYCAPB),
	STM32_GATE(_DDRPHYCAPBLP, DDRPHYCAPBLP, _PCLK4, CLK_IS_CRITICAL,  GATE_DDRPHYCAPBLP),

	STM32_GATE(_SYSCFG, SYSCFG, _PCLK3, 0, GATE_SYSCFG),
	STM32_GATE(_DDRPERFM, DDRPERFM, _PCLK4, 0, GATE_DDRPERFM),
	STM32_GATE(_IWDG2APB, IWDG2, _PCLK4, 0, GATE_IWDG2APB),
	STM32_GATE(_USBPHY_K, USBPHY_K, MUX(MUX_USBPHY), 0, GATE_USBPHY),
	STM32_GATE(_USBO_K, USBO_K, MUX(MUX_USBO), 0, GATE_USBO),

	STM32_GATE(_RTCAPB, RTCAPB, _PCLK5, CLK_IS_CRITICAL, GATE_RTCAPB),
	STM32_GATE(_TZC, TZC, _PCLK5, CLK_IS_CRITICAL, GATE_TZC),
	STM32_GATE(_ETZPC, TZPC, _PCLK5, CLK_IS_CRITICAL, GATE_ETZPC),
	STM32_GATE(_IWDG1APB, IWDG1, _PCLK5, 0, GATE_IWDG1APB),
	STM32_GATE(_BSEC, BSEC, _PCLK5, CLK_IS_CRITICAL, GATE_BSEC),
	STM32_GATE(_STGENC, STGEN_K, MUX(MUX_STGEN), CLK_IS_CRITICAL, GATE_STGENC),

	STM32_GATE(_USART1_K, USART1_K, MUX(MUX_UART1), 0, GATE_USART1),
	STM32_GATE(_USART2_K, USART2_K, MUX(MUX_UART2), 0, GATE_USART2),
	STM32_GATE(_I2C3_K, I2C3_K, MUX(MUX_I2C3), 0, GATE_I2C3),
	STM32_GATE(_I2C4_K, I2C4_K, MUX(MUX_I2C4), 0, GATE_I2C4),
	STM32_GATE(_I2C5_K, I2C5_K, MUX(MUX_I2C5), 0, GATE_I2C5),
	STM32_GATE(_TIM12, TIM12_K, _CKTIMG3, 0, GATE_TIM12),
	STM32_GATE(_TIM15, TIM15_K, _CKTIMG3, 0, GATE_TIM15),

	STM32_GATE(_RTCCK, RTC, MUX(MUX_RTC), 0, GATE_RTCCK),

	STM32_GATE(_GPIOA, GPIOA, _CKMLAHB, 0, GATE_GPIOA),
	STM32_GATE(_GPIOB, GPIOB, _CKMLAHB, 0, GATE_GPIOB),
	STM32_GATE(_GPIOC, GPIOC, _CKMLAHB, 0, GATE_GPIOC),
	STM32_GATE(_GPIOD, GPIOD, _CKMLAHB, 0, GATE_GPIOD),
	STM32_GATE(_GPIOE, GPIOE, _CKMLAHB, 0, GATE_GPIOE),
	STM32_GATE(_GPIOF, GPIOF, _CKMLAHB, 0, GATE_GPIOF),
	STM32_GATE(_GPIOG, GPIOG, _CKMLAHB, 0, GATE_GPIOG),
	STM32_GATE(_GPIOH, GPIOH, _CKMLAHB, 0, GATE_GPIOH),
	STM32_GATE(_GPIOI, GPIOI, _CKMLAHB, 0, GATE_GPIOI),

	STM32_GATE(_PKA, PKA, _CKAXI, 0, GATE_PKA),
	STM32_GATE(_SAES_K, SAES_K, MUX(MUX_SAES), 0, GATE_SAES),
	STM32_GATE(_CRYP1, CRYP1, _PCLK5, 0, GATE_CRYP1),
	STM32_GATE(_HASH1, HASH1, _PCLK5, 0, GATE_HASH1),

	STM32_GATE(_RNG1_K, RNG1_K, MUX(MUX_RNG1), 0, GATE_RNG1),
	STM32_GATE(_BKPSRAM, BKPSRAM, _PCLK5, CLK_IS_CRITICAL, GATE_BKPSRAM),

	STM32_GATE(_SDMMC1_K, SDMMC1_K, MUX(MUX_SDMMC1), 0, GATE_SDMMC1),
	STM32_GATE(_SDMMC2_K, SDMMC2_K, MUX(MUX_SDMMC2), 0, GATE_SDMMC2),
	STM32_GATE(_DBGCK, CK_DBG, _CKAXI, 0, GATE_DBGCK),

/* TODO: CHECK CLOCK FOR BL2/BL32 AND IF ONLY FOR TEST OR NOT */
	STM32_GATE(_USART3_K, USART3_K, MUX(MUX_UART35), 0, GATE_USART3),
	STM32_GATE(_UART4_K, UART4_K, MUX(MUX_UART4), 0, GATE_UART4),
	STM32_GATE(_UART5_K, UART5_K, MUX(MUX_UART35), 0, GATE_UART5),
	STM32_GATE(_UART7_K, UART7_K, MUX(MUX_UART78), 0, GATE_UART7),
	STM32_GATE(_UART8_K, UART8_K, MUX(MUX_UART78), 0, GATE_UART8),
	STM32_GATE(_USART6_K, USART6_K, MUX(MUX_UART6), 0, GATE_USART6),
	STM32_GATE(_MCE, MCE, _CKAXI, CLK_IS_CRITICAL, GATE_MCE),
	STM32_GATE(_FMC_K, FMC_K, MUX(MUX_FMC), 0, GATE_FMC),
	STM32_GATE(_QSPI_K, QSPI_K, MUX(MUX_QSPI), 0, GATE_QSPI),

	STM32_COMPOSITE(_MCO1_K, CK_MCO1, MUX(MUX_MCO1), 0, GATE_MCO1, DIV_MCO1),
	STM32_COMPOSITE(_MCO2_K, CK_MCO2, MUX(MUX_MCO2), 0, GATE_MCO2, DIV_MCO2),
	STM32_COMPOSITE(_TRACECK, CK_TRACE, _CKAXI, 0, GATE_TRACECK, DIV_TRACE),

#if defined(IMAGE_BL32)
	STM32_GATE(_TIM2, TIM2_K, _CKTIMG1, 0, GATE_TIM2),
	STM32_GATE(_TIM3, TIM3_K, _CKTIMG1, 0, GATE_TIM3),
	STM32_GATE(_TIM4, TIM4_K, _CKTIMG1, 0, GATE_TIM4),
	STM32_GATE(_TIM5, TIM5_K, _CKTIMG1, 0, GATE_TIM5),
	STM32_GATE(_TIM6, TIM6_K, _CKTIMG1, 0, GATE_TIM6),
	STM32_GATE(_TIM7, TIM7_K, _CKTIMG1, 0, GATE_TIM7),
	STM32_GATE(_TIM13, TIM13_K, _CKTIMG3, 0, GATE_TIM13),
	STM32_GATE(_TIM14, TIM14_K, _CKTIMG3, 0, GATE_TIM14),
	STM32_GATE(_LPTIM1_K, LPTIM1_K, MUX(MUX_LPTIM1), 0, GATE_LPTIM1),
	STM32_GATE(_SPI2_K, SPI2_K, MUX(MUX_SPI23), 0, GATE_SPI2),
	STM32_GATE(_SPI3_K, SPI3_K, MUX(MUX_SPI23), 0, GATE_SPI3),
	STM32_GATE(_SPDIF_K, SPDIF_K, MUX(MUX_SPDIF), 0, GATE_SPDIF),
	STM32_GATE(_TIM1, TIM1_K, _CKTIMG2, 0, GATE_TIM1),
	STM32_GATE(_TIM8, TIM8_K, _CKTIMG2, 0, GATE_TIM8),
	STM32_GATE(_TIM16, TIM16_K, _CKTIMG3, 0, GATE_TIM16),
	STM32_GATE(_TIM17, TIM17_K, _CKTIMG3, 0, GATE_TIM17),
	STM32_GATE(_SPI1_K, SPI1_K, MUX(MUX_SPI1), 0, GATE_SPI1),
	STM32_GATE(_SPI4_K, SPI4_K, MUX(MUX_SPI4), 0, GATE_SPI4),
	STM32_GATE(_SPI5_K, SPI5_K, MUX(MUX_SPI5), 0, GATE_SPI5),
	STM32_GATE(_SAI1_K, SAI1_K, MUX(MUX_SAI1), 0, GATE_SAI1),
	STM32_GATE(_SAI2_K, SAI2_K, MUX(MUX_SAI2), 0, GATE_SAI2),
	STM32_GATE(_DFSDM, DFSDM_K, MUX(MUX_SAI1), 0, GATE_DFSDM),
	STM32_GATE(_FDCAN_K, FDCAN_K, MUX(MUX_FDCAN), 0, GATE_FDCAN),
	STM32_GATE(_USBH, USBH, _CKAXI, 0, GATE_USBH),
	STM32_GATE(_I2C1_K, I2C1_K, MUX(MUX_I2C12), 0, GATE_I2C1),
	STM32_GATE(_I2C2_K, I2C2_K, MUX(MUX_I2C12), 0, GATE_I2C2),
	STM32_GATE(_ADFSDM, ADFSDM_K, MUX(MUX_SAI1), 0, GATE_ADFSDM),
	STM32_GATE(_LPTIM2_K, LPTIM2_K, MUX(MUX_LPTIM2), 0, GATE_LPTIM2),
	STM32_GATE(_LPTIM3_K, LPTIM3_K, MUX(MUX_LPTIM3), 0, GATE_LPTIM3),
	STM32_GATE(_LPTIM4_K, LPTIM4_K, MUX(MUX_LPTIM45), 0, GATE_LPTIM4),
	STM32_GATE(_LPTIM5_K, LPTIM5_K, MUX(MUX_LPTIM45), 0, GATE_LPTIM5),
	STM32_GATE(_VREF, VREF, _PCLK3, 0, GATE_VREF),
	STM32_GATE(_DTS, TMPSENS, _PCLK3, 0, GATE_DTS),
	STM32_GATE(_PMBCTRL, PMBCTRL, _PCLK3, 0, GATE_HDP),
	STM32_GATE(_HDP, HDP, _PCLK3, 0, GATE_PMBCTRL),
	STM32_GATE(_STGENRO, STGENRO, _PCLK4, 0, GATE_DCMIPP),
	STM32_GATE(_DCMIPP_K, DCMIPP_K, MUX(MUX_DCMIPP), 0, GATE_DCMIPP),
	STM32_GATE(_DMAMUX1, DMAMUX1, _CKAXI, 0, GATE_DMAMUX1),
	STM32_GATE(_DMAMUX2, DMAMUX2, _CKAXI, 0, GATE_DMAMUX2),
	STM32_GATE(_DMA3, DMA3, _CKAXI, 0, GATE_DMAMUX2),
	STM32_GATE(_ADC1_K, ADC1_K, MUX(MUX_ADC1), 0, GATE_ADC1),
	STM32_GATE(_ADC2_K, ADC2_K, MUX(MUX_ADC2), 0, GATE_ADC2),
	STM32_GATE(_TSC, TSC, _CKAXI, 0, GATE_TSC),
	STM32_GATE(_AXIMC, AXIMC, _CKAXI, 0, GATE_AXIMC),
	STM32_GATE(_CRC1, CRC1, _CKAXI, 0, GATE_ETH1TX),
	STM32_GATE(_ETH1CK, ETH1CK_K, MUX(MUX_ETH1), 0, GATE_ETH1CK),
	STM32_GATE(_ETH1TX, ETH1TX, _CKAXI, 0, GATE_ETH1TX),
	STM32_GATE(_ETH1RX, ETH1RX, _CKAXI, 0, GATE_ETH1RX),
	STM32_GATE(_ETH2CK, ETH2CK_K, MUX(MUX_ETH2), 0, GATE_ETH2CK),
	STM32_GATE(_ETH2TX, ETH2TX, _CKAXI, 0, GATE_ETH2TX),
	STM32_GATE(_ETH2RX, ETH2RX, _CKAXI, 0, GATE_ETH2RX),
	STM32_GATE(_ETH2MAC, ETH2MAC, _CKAXI, 0, GATE_ETH2MAC),
#endif
};

static struct stm32_pll_dt_cfg mp13_pll[_PLL_NB];

static struct stm32_osci_dt_cfg mp13_osci[NB_OSCILLATOR];

static uint32_t mp13_clksrc[MUX_MAX];

static uint32_t mp13_clkdiv[DIV_MAX];

static struct stm32_clk_platdata stm32mp13_clock_pdata = {
	.osci		= mp13_osci,
	.nosci		= NB_OSCILLATOR,
	.pll		= mp13_pll,
	.npll		= _PLL_NB,
	.clksrc		= mp13_clksrc,
	.nclksrc	= MUX_MAX,
	.clkdiv		= mp13_clkdiv,
	.nclkdiv	= DIV_MAX,
};

static struct stm32_clk_priv stm32mp13_clock_data = {
	.base		= RCC_BASE,
	.num		= ARRAY_SIZE(stm32mp13_clk),
	.clks		= stm32mp13_clk,
	.parents	= parent_mp13,
	.nb_parents	= ARRAY_SIZE(parent_mp13),
	.gates		= gates_mp13,
	.nb_gates	= ARRAY_SIZE(gates_mp13),
	.div		= dividers_mp13,
	.nb_div		= ARRAY_SIZE(dividers_mp13),
	.osci_data	= stm32mp13_osc_data,
	.nb_osci_data	= ARRAY_SIZE(stm32mp13_osc_data),
	.gate_refcounts	= refcounts_mp13,
	.pdata		= &stm32mp13_clock_pdata,
};

static int stm32mp1_init_clock_tree(void)
{
	struct stm32_clk_priv *priv = clk_stm32_get_priv();
	int ret;

#if STM32MP_USB_PROGRAMMER
	int usbphy_p = _clk_stm32_get_parent(priv, _USBPHY_K);
	int usbo_p = _clk_stm32_get_parent(priv, _USBO_K);

	/* Don't initialize PLL4, when used by BOOTROM */
	pll4_bootrom = stm32mp1_clk_is_pll4_used_by_bootrom(priv, usbphy_p);
#endif

	/*
	 * Switch ON oscillators found in device-tree.
	 * Note: HSI already ON after BootROM stage.
	 */
	stm32_clk_oscillators_enable(priv);

	/* Come back to HSI */
	ret = stm32mp1_come_back_to_hsi();
	if (ret != 0) {
		return ret;
	}

	ret = stm32_clk_hsidiv_configure(priv);
	if (ret != 0) {
		return ret;
	}

	ret = stm32_clk_stgen_configure(priv, _STGENC);
	if (ret != 0) {
		panic();
	}

	ret = stm32_clk_dividers_configure(priv);
	if (ret != 0) {
		panic();
	}

	ret = stm32_clk_pll_configure(priv);
	if (ret != 0) {
		panic();
	}

	/* Wait LSE ready before to use it */
	ret = stm32_clk_oscillators_wait_lse_ready(priv);
	if (ret != 0) {
		panic();
	}

	/* Configure with expected clock source */
	ret = stm32_clk_source_configure(priv);
	if (ret != 0) {
		panic();
	}

	/* Configure LSE css after RTC source configuration */
	ret = stm32_clk_oscillators_lse_set_css(priv);
	if (ret != 0) {
		panic();
	}

#if STM32MP_USB_PROGRAMMER
	ret = stm32mp1_clk_check_usb_conflict(priv, usbphy_p, usbo_p);
	if (ret != 0) {
		return ret;
	}
#endif
	/* reconfigure STGEN with DT config */
	ret = stm32_clk_stgen_configure(priv, _STGENC);
	if (ret != 0) {
		panic();
	}

	/* Software Self-Refresh mode (SSR) during DDR initilialization */
	mmio_clrsetbits_32(priv->base + RCC_DDRITFCR,
			   RCC_DDRITFCR_DDRCKMOD_MASK,
			   RCC_DDRITFCR_DDRCKMOD_SSR <<
			   RCC_DDRITFCR_DDRCKMOD_SHIFT);

	return 0;
}

#define LSEDRV_MEDIUM_HIGH 2

static int clk_stm32_parse_oscillator_fdt(void *fdt, int node, const char *name,
					  struct stm32_osci_dt_cfg *osci)
{
	int subnode = 0;

	/* default value oscillator not found, freq=0 */
	osci->freq = 0;

	fdt_for_each_subnode(subnode, fdt, node) {
		const char *cchar = NULL;
		const fdt32_t *cuint = NULL;
		int ret = 0;

		cchar = fdt_get_name(fdt, subnode, &ret);
		if (cchar == NULL) {
			return ret;
		}

		if (strncmp(cchar, name, (size_t)ret) ||
		    fdt_get_status(subnode) == DT_DISABLED) {
			continue;
		}

		cuint = fdt_getprop(fdt, subnode, "clock-frequency", &ret);
		if (cuint == NULL) {
			return ret;
		}

		osci->freq = fdt32_to_cpu(*cuint);

		if (fdt_getprop(fdt, subnode, "st,bypass", NULL) != NULL) {
			osci->bypass = true;
		}

		if (fdt_getprop(fdt, subnode, "st,digbypass", NULL) != NULL) {
			osci->digbyp = true;
		}

		if (fdt_getprop(fdt, subnode, "st,css", NULL) != NULL) {
			osci->css = true;
		}

		osci->drive = fdt_read_uint32_default(fdt, subnode, "st,drive", LSEDRV_MEDIUM_HIGH);

		return 0;
	}

	return 0;
}

static int stm32_clk_parse_fdt_all_oscillator(void *fdt, struct stm32_clk_platdata *pdata)
{
	int fdt_err = 0;
	uint32_t i = 0;
	int node = 0;

	node = fdt_path_offset(fdt, "/clocks");
	if (node < 0) {
		return -FDT_ERR_NOTFOUND;
	}

	for (i = 0; i < pdata->nosci; i++) {
		const char *name = NULL;

		name = clk_stm32_get_oscillator_name((enum stm32_osc)i);
		if (name == NULL) {
			continue;
		}

		fdt_err = clk_stm32_parse_oscillator_fdt(fdt, node, name, &pdata->osci[i]);
		if (fdt_err < 0) {
			panic();
		}
	}

	return 0;
}

#define RCC_PLL_NAME_SIZE 12

static int clk_stm32_load_vco_config(void *fdt, int subnode, struct stm32_pll_vco *vco)
{
	int err = 0;

	err = fdt_read_uint32_array(fdt, subnode, "divmn", (int)PLL_DIV_MN_NB, vco->div_mn);
	if (err != 0) {
		return err;
	}

	err = fdt_read_uint32_array(fdt, subnode, "csg", (int)PLL_CSG_NB, vco->csg);

	vco->csg_enabled = (err == 0);

	if (err == -FDT_ERR_NOTFOUND) {
		err = 0;
	}

	if (err != 0) {
		return err;
	}

	vco->status = RCC_PLLNCR_DIVPEN | RCC_PLLNCR_DIVQEN | RCC_PLLNCR_DIVREN | RCC_PLLNCR_PLLON;

	vco->frac = fdt_read_uint32_default(fdt, subnode, "frac", 0);

	vco->src = fdt_read_uint32_default(fdt, subnode, "src", UINT32_MAX);

	return 0;
}

static int clk_stm32_load_output_config(void *fdt, int subnode, struct stm32_pll_output *output)
{
	int err = 0;

	err = fdt_read_uint32_array(fdt, subnode, "st,pll_div_pqr", (int)PLL_DIV_PQR_NB,
				    output->output);
	if (err != 0) {
		return err;
	}

	return 0;
}

static int clk_stm32_parse_pll_fdt(void *fdt, int subnode, struct stm32_pll_dt_cfg *pll)
{
	const fdt32_t *cuint = NULL;
	int subnode_pll = 0;
	int subnode_vco = 0;
	int err = 0;

	cuint = fdt_getprop(fdt, subnode, "st,pll", NULL);
	if (!cuint) {
		return -FDT_ERR_NOTFOUND;
	}

	subnode_pll = fdt_node_offset_by_phandle(fdt, fdt32_to_cpu(*cuint));
	if (subnode_pll < 0) {
		return -FDT_ERR_NOTFOUND;
	}

	cuint = fdt_getprop(fdt, subnode_pll, "st,pll_vco", NULL);
	if (!cuint) {
		return -FDT_ERR_NOTFOUND;
	}

	subnode_vco = fdt_node_offset_by_phandle(fdt, fdt32_to_cpu(*cuint));
	if (subnode_vco < 0) {
		return -FDT_ERR_NOTFOUND;
	}

	err = clk_stm32_load_vco_config(fdt, subnode_vco, &pll->vco);
	if (err != 0) {
		return err;
	}

	err = clk_stm32_load_output_config(fdt, subnode_pll, &pll->output);
	if (err != 0) {
		return err;
	}

	return 0;
}

static int stm32_clk_parse_fdt_all_pll(void *fdt, int node, struct stm32_clk_platdata *pdata)
{
	size_t i = 0U;

	for (i = _PLL1; i < pdata->npll; i++) {
		struct stm32_pll_dt_cfg *pll = pdata->pll + i;
		char name[RCC_PLL_NAME_SIZE];
		int subnode = 0;
		int err = 0;

		snprintf(name, sizeof(name), "st,pll@%u", i);

		subnode = fdt_subnode_offset(fdt, node, name);
		if (!fdt_check_node(subnode)) {
			continue;
		}

		err = clk_stm32_parse_pll_fdt(fdt, subnode, pll);
		if (err != 0) {
			panic();
		}
	}

	return 0;
}

static int stm32_clk_parse_fdt(struct stm32_clk_platdata *pdata)
{
	void *fdt = NULL;
	int node;
	uint32_t err;

	if (fdt_get_address(&fdt) == 0) {
		return -ENOENT;
	}

	node = fdt_node_offset_by_compatible(fdt, -1, DT_RCC_CLK_COMPAT);
	if (node < 0) {
		panic();
	}

	err = stm32_clk_parse_fdt_all_oscillator(fdt, pdata);
	if (err != 0) {
		return err;
	}

	err = stm32_clk_parse_fdt_all_pll(fdt, node, pdata);
	if (err != 0) {
		return err;
	}

	err = stm32_clk_parse_fdt_by_name(fdt, node, "st,clkdiv", pdata->clkdiv, &pdata->nclkdiv);
	if (err != 0) {
		return err;
	}

	err = stm32_clk_parse_fdt_by_name(fdt, node, "st,clksrc", pdata->clksrc, &pdata->nclksrc);
	if (err != 0) {
		return err;
	}

	return 0;
}

int stm32mp1_clk_init(void)
{
	return 0;
}

int stm32mp1_clk_probe(void)
{
	uintptr_t base = RCC_BASE;
	int ret;

	ret = stm32_clk_parse_fdt(&stm32mp13_clock_pdata);
	if (ret != 0) {
		return ret;
	}

	ret = clk_stm32_init(&stm32mp13_clock_data, base);
	if (ret != 0) {
		return ret;
	}

	ret = stm32mp1_init_clock_tree();
	if (ret != 0) {
		return ret;
	}

	clk_stm32_enable_critical_clocks();

	return 0;
}
