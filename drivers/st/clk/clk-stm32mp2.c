/*
 * Copyright (C) 2024, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>

#include "clk-stm32-core.h"
#include <common/fdt_wrappers.h>
#include <drivers/clk.h>
#include <drivers/delay_timer.h>
#include <drivers/generic_delay_timer.h>
#include <drivers/st/stm32mp2_clk.h>
#include <drivers/st/stm32mp_clkfunc.h>
#include <lib/mmio.h>
#include <lib/spinlock.h>
#include <lib/utils_def.h>
#include <libfdt.h>

#include <platform_def.h>

struct stm32_osci_dt_cfg {
	unsigned long freq;
	uint32_t drive;
	bool bypass;
	bool digbyp;
	bool css;
};

struct stm32_pll_dt_cfg {
	uint32_t src;
	uint32_t frac;
	uint32_t cfg[PLLCFG_NB];
	uint32_t csg[PLLCSG_NB];
	bool csg_enabled;
	bool enabled;
};

struct stm32_clk_platdata {
	uintptr_t rcc_base;
	uint32_t nosci;
	struct stm32_osci_dt_cfg *osci;
	uint32_t npll;
	struct stm32_pll_dt_cfg *pll;
	uint32_t nflexgen;
	uint32_t *flexgen;
	uint32_t nbusclk;
	uint32_t *busclk;
	uint32_t nkernelclk;
	uint32_t *kernelclk;
};

/* A35 Sub-System which manages its own PLL (PLL1) */
#define A35_SS_CHGCLKREQ	0x0000
#define A35_SS_PLL_FREQ1	0x0080
#define A35_SS_PLL_FREQ2	0x0090
#define A35_SS_PLL_ENABLE	0x00a0

#define A35_SS_CHGCLKREQ_ARM_CHGCLKREQ		BIT(0)
#define A35_SS_CHGCLKREQ_ARM_CHGCLKACK		BIT(1)
#define A35_SS_CHGCLKREQ_ARM_DIVSEL		BIT(16)
#define A35_SS_CHGCLKREQ_ARM_DIVSELACK		BIT(17)

#define A35_SS_PLL_FREQ1_FBDIV_MASK		GENMASK(11, 0)
#define A35_SS_PLL_FREQ1_FBDIV_SHIFT		0
#define A35_SS_PLL_FREQ1_REFDIV_MASK		GENMASK(21, 16)
#define A35_SS_PLL_FREQ1_REFDIV_SHIFT		16

#define A35_SS_PLL_FREQ2_POSTDIV1_MASK		GENMASK(2, 0)
#define A35_SS_PLL_FREQ2_POSTDIV1_SHIFT		0
#define A35_SS_PLL_FREQ2_POSTDIV2_MASK		GENMASK(5, 3)
#define A35_SS_PLL_FREQ2_POSTDIV2_SHIFT		3

#define A35_SS_PLL_ENABLE_PD			BIT(0)
#define A35_SS_PLL_ENABLE_LOCKP			BIT(1)
#define A35_SS_PLL_ENABLE_NRESET_SWPLL_FF	BIT(2)

#define TIMEOUT_US_200MS	U(200000)
#define TIMEOUT_US_1S		U(1000000)

#define PLLRDY_TIMEOUT		TIMEOUT_US_200MS
#define CLKSRC_TIMEOUT		TIMEOUT_US_200MS
#define CLKDIV_TIMEOUT		TIMEOUT_US_200MS
#define OSCRDY_TIMEOUT		TIMEOUT_US_1S

/* PLL minimal frequencies for clock sources */
#define PLL_REFCLK_MIN			UL(5000000)
#define PLL_FRAC_REFCLK_MIN		UL(10000000)

#define XBAR_CHANNEL_NB			64

/* Warning, should be start to 1 */
enum clock {
	_CK_0_MHZ,

	/* ROOT CLOCKS */
	_CK_HSI,
	_CK_HSE,
	_CK_MSI,
	_CK_LSI,
	_CK_LSE,
	_I2SCKIN,
	_SPDIFSYMB,
	_CK_PLL1,
	_CK_PLL2,
#if !STM32MP21
	_CK_PLL3,
#endif /* !STM32MP21 */
	_CK_PLL4,
	_CK_PLL5,
	_CK_PLL6,
	_CK_PLL7,
	_CK_PLL8,
	_CK_HSE_RTC,
	_CK_RTCCK,
	_CK_ICN_HS_MCU,
	_CK_ICN_SDMMC,
	_CK_ICN_DDR,
	_CK_ICN_HSL,
	_CK_ICN_NIC,
	_CK_ICN_LS_MCU,
	_CK_FLEXGEN_07,
	_CK_FLEXGEN_08,
	_CK_FLEXGEN_09,
	_CK_FLEXGEN_10,
	_CK_FLEXGEN_11,
	_CK_FLEXGEN_12,
	_CK_FLEXGEN_13,
	_CK_FLEXGEN_14,
	_CK_FLEXGEN_15,
	_CK_FLEXGEN_16,
	_CK_FLEXGEN_17,
	_CK_FLEXGEN_18,
	_CK_FLEXGEN_19,
	_CK_FLEXGEN_20,
	_CK_FLEXGEN_21,
	_CK_FLEXGEN_22,
	_CK_FLEXGEN_23,
	_CK_FLEXGEN_24,
	_CK_FLEXGEN_25,
	_CK_FLEXGEN_26,
	_CK_FLEXGEN_27,
	_CK_FLEXGEN_28,
	_CK_FLEXGEN_29,
	_CK_FLEXGEN_30,
	_CK_FLEXGEN_31,
	_CK_FLEXGEN_32,
	_CK_FLEXGEN_33,
	_CK_FLEXGEN_34,
	_CK_FLEXGEN_35,
	_CK_FLEXGEN_36,
	_CK_FLEXGEN_37,
	_CK_FLEXGEN_38,
	_CK_FLEXGEN_39,
	_CK_FLEXGEN_40,
	_CK_FLEXGEN_41,
	_CK_FLEXGEN_42,
	_CK_FLEXGEN_43,
	_CK_FLEXGEN_44,
	_CK_FLEXGEN_45,
	_CK_FLEXGEN_46,
	_CK_FLEXGEN_47,
	_CK_FLEXGEN_48,
	_CK_FLEXGEN_49,
	_CK_FLEXGEN_50,
	_CK_FLEXGEN_51,
	_CK_FLEXGEN_52,
	_CK_FLEXGEN_53,
	_CK_FLEXGEN_54,
	_CK_FLEXGEN_55,
	_CK_FLEXGEN_56,
	_CK_FLEXGEN_57,
	_CK_FLEXGEN_58,
	_CK_FLEXGEN_59,
	_CK_FLEXGEN_60,
	_CK_FLEXGEN_61,
	_CK_FLEXGEN_62,
	_CK_FLEXGEN_63,
	_CK_ICN_APB1,
	_CK_ICN_APB2,
	_CK_ICN_APB3,
	_CK_ICN_APB4,
#if STM32MP21
	_CK_ICN_APB5,
#endif /* STM32MP21 */
	_CK_ICN_APBDBG,
	_CK_BKPSRAM,
	_CK_BSEC,
	_CK_CRC,
	_CK_CRYP1,
	_CK_CRYP2,
	_CK_DDR,
	_CK_DDRCAPB,
	_CK_DDRCP,
	_CK_DDRPHYC,
	_CK_FMC,
	_CK_GPIOA,
	_CK_GPIOB,
	_CK_GPIOC,
	_CK_GPIOD,
	_CK_GPIOE,
	_CK_GPIOF,
	_CK_GPIOG,
	_CK_GPIOH,
	_CK_GPIOI,
#if !STM32MP21
	_CK_GPIOJ,
	_CK_GPIOK,
#endif /* !STM32MP21 */
	_CK_GPIOZ,
#if STM32MP21
	_CK_HASH1,
	_CK_HASH2,
#else /* STM32MP21 */
	_CK_HASH,
#endif /* STM32MP21 */
	_CK_I2C1,
	_CK_I2C2,
#if !STM32MP23
	_CK_I2C3,
#endif /* !STM32MP23 */
#if STM32MP25
	_CK_I2C4,
	_CK_I2C5,
	_CK_I2C6,
#endif /* STM32MP25 */
#if !STM32MP21
	_CK_I2C7,
	_CK_I2C8,
#endif /* !STM32MP21 */
	_CK_IWDG1,
	_CK_IWDG2,
	_CK_OSPI1,
#if !STM32MP21
	_CK_OSPI2,
	_CK_OSPIIOM,
#endif /* !STM32MP21 */
	_CK_PKA,
	_CK_RETRAM,
#if STM32MP21
	_CK_RNG1,
	_CK_RNG2,
#else /* STM32MP21 */
	_CK_RNG,
#endif /* STM32MP21 */
	_CK_RTC,
	_CK_SAES,
	_CK_SDMMC1,
	_CK_SDMMC2,
	_CK_SRAM1,
#if !STM32MP21
	_CK_SRAM2,
#endif /* !STM32MP21 */
	_CK_STGEN,
	_CK_SYSCPU1,
	_CK_SYSRAM,
	_CK_UART4,
	_CK_UART5,
	_CK_UART7,
#if STM32MP25
	_CK_UART8,
	_CK_UART9,
#endif /* STM32MP25 */
	_CK_USART1,
	_CK_USART2,
	_CK_USART3,
	_CK_USART6,
#if STM32MP21
	_CK_USBHEHCI,
	_CK_USBHOHCI,
#else /* STM32MP21 */
	_CK_USB2EHCI,
	_CK_USB2OHCI,
#endif /* STM32MP21 */
	_CK_USB2PHY1,
	_CK_USB2PHY2,
#if !STM32MP21
	_CK_USB3DR,
	_CK_USB3PCIEPHY,
	_CK_USBTC,
#endif /* !STM32MP21 */
	_CK_BUS_RISAF4,

	CK_LAST
};

static const uint16_t muxsel_src[] = {
	_CK_HSI, _CK_HSE, _CK_MSI, _CK_0_MHZ
};

static const uint16_t xbarsel_src[] = {
	_CK_PLL4, _CK_PLL5, _CK_PLL6, _CK_PLL7, _CK_PLL8,
	_CK_HSI, _CK_HSE, _CK_MSI, _CK_HSI, _CK_HSE, _CK_MSI,
	_SPDIFSYMB, _I2SCKIN, _CK_LSI, _CK_LSE
};

static const uint16_t rtc_src[] = {
	_CK_0_MHZ, _CK_LSE, _CK_LSI, _CK_HSE_RTC
};

static const uint16_t usb2phy1_src[] = {
	_CK_FLEXGEN_57, _CK_HSE
};

static const uint16_t usb2phy2_src[] = {
	_CK_FLEXGEN_58, _CK_HSE
};

#if !STM32MP21
static const uint16_t usb3pciphy_src[] = {
	_CK_FLEXGEN_34, _CK_HSE
};

static const uint16_t d3per_src[] = {
	_CK_MSI, _CK_LSI, _CK_LSE
};
#endif /* !STM32MP21 */

#define MUX_CONF(id, src, _offset, _shift, _witdh)[id] = {\
	.id_parents	= src,\
	.num_parents	= ARRAY_SIZE(src),\
	.mux		= &(struct mux_cfg) {\
		.offset	= (_offset),\
		.shift	= (_shift),\
		.width	= (_witdh),\
		.bitrdy = UINT8_MAX,\
	},\
}

static const struct parent_cfg parent_mp2[] = {
	MUX_CONF(MUX_MUXSEL0, muxsel_src, RCC_MUXSELCFGR, 0, 2),
	MUX_CONF(MUX_MUXSEL1, muxsel_src, RCC_MUXSELCFGR, 4, 2),
	MUX_CONF(MUX_MUXSEL2, muxsel_src, RCC_MUXSELCFGR, 8, 2),
	MUX_CONF(MUX_MUXSEL3, muxsel_src, RCC_MUXSELCFGR, 12, 2),
	MUX_CONF(MUX_MUXSEL4, muxsel_src, RCC_MUXSELCFGR, 16, 2),
	MUX_CONF(MUX_MUXSEL5, muxsel_src, RCC_MUXSELCFGR, 20, 2),
	MUX_CONF(MUX_MUXSEL6, muxsel_src, RCC_MUXSELCFGR, 24, 2),
	MUX_CONF(MUX_MUXSEL7, muxsel_src, RCC_MUXSELCFGR, 28, 2),
	MUX_CONF(MUX_XBARSEL, xbarsel_src, RCC_XBAR0CFGR, 0, 4),
	MUX_CONF(MUX_RTC, rtc_src, RCC_BDCR, 16, 2),
	MUX_CONF(MUX_USB2PHY1, usb2phy1_src, RCC_USB2PHY1CFGR, 15, 1),
	MUX_CONF(MUX_USB2PHY2, usb2phy2_src, RCC_USB2PHY2CFGR, 15, 1),
#if !STM32MP21
	MUX_CONF(MUX_USB3PCIEPHY, usb3pciphy_src, RCC_USB3PCIEPHYCFGR, 15, 1),
	MUX_CONF(MUX_D3PER, d3per_src, RCC_D3DCR, 16, 2),
#endif /* !STM32MP21 */
};

/* GATES */
enum enum_gate_cfg {
	GATE_ZERO, /* reserved for no gate */
	GATE_LSE,
	GATE_RTCCK,
	GATE_LSI,
	GATE_HSI,
	GATE_MSI,
	GATE_HSE,
	GATE_LSI_RDY,
	GATE_MSI_RDY,
	GATE_LSE_RDY,
	GATE_HSE_RDY,
	GATE_HSI_RDY,
	GATE_SYSRAM,
	GATE_RETRAM,
	GATE_SRAM1,
#if !STM32MP21
	GATE_SRAM2,
#endif /* !STM32MP21 */

	GATE_DDRPHYC,
	GATE_SYSCPU1,
	GATE_CRC,
#if !STM32MP21
	GATE_OSPIIOM,
#endif /* !STM32MP21 */
	GATE_BKPSRAM,
#if STM32MP21
	GATE_HASH1,
	GATE_HASH2,
	GATE_RNG1,
	GATE_RNG2,
#else /* STM32MP21 */
	GATE_HASH,
	GATE_RNG,
#endif /* STM32MP21 */
	GATE_CRYP1,
	GATE_CRYP2,
	GATE_SAES,
	GATE_PKA,

	GATE_GPIOA,
	GATE_GPIOB,
	GATE_GPIOC,
	GATE_GPIOD,
	GATE_GPIOE,
	GATE_GPIOF,
	GATE_GPIOG,
	GATE_GPIOH,
	GATE_GPIOI,
#if !STM32MP21
	GATE_GPIOJ,
	GATE_GPIOK,
#endif /* !STM32MP21 */
	GATE_GPIOZ,
	GATE_RTC,

	GATE_DDRCP,

	/* WARNING 2 CLOCKS FOR ONE GATE */
#if STM32MP21
	GATE_USBHOHCI,
	GATE_USBHEHCI,
#else /* STM32MP21 */
	GATE_USB2OHCI,
	GATE_USB2EHCI,
#endif /* STM32MP21 */

#if !STM32MP21
	GATE_USB3DR,
#endif /* !STM32MP21 */

	GATE_BSEC,
	GATE_IWDG1,
	GATE_IWDG2,

	GATE_DDRCAPB,
	GATE_DDR,

	GATE_USART2,
	GATE_UART4,
	GATE_USART3,
	GATE_UART5,
	GATE_I2C1,
	GATE_I2C2,
#if !STM32MP23
	GATE_I2C3,
#endif /* !STM32MP23 */
#if STM32MP25
	GATE_I2C5,
	GATE_I2C4,
	GATE_I2C6,
#endif /* STM32MP25 */
#if !STM32MP21
	GATE_I2C7,
#endif /* !STM32MP21 */
	GATE_USART1,
	GATE_USART6,
	GATE_UART7,
#if STM32MP25
	GATE_UART8,
	GATE_UART9,
#endif /* STM32MP25 */
	GATE_STGEN,
#if !STM32MP21
	GATE_USB3PCIEPHY,
	GATE_USBTC,
	GATE_I2C8,
#endif /* !STM32MP21 */
	GATE_OSPI1,
#if !STM32MP21
	GATE_OSPI2,
#endif /* !STM32MP21 */
	GATE_FMC,
	GATE_SDMMC1,
	GATE_SDMMC2,
	GATE_USB2PHY1,
	GATE_USB2PHY2,
	LAST_GATE
};

#define GATE_CFG(id, _offset, _bit_idx, _offset_clr)[id] = {\
	.offset		= (_offset),\
	.bit_idx	= (_bit_idx),\
	.set_clr	= (_offset_clr),\
}

static const struct gate_cfg gates_mp2[LAST_GATE] = {
	GATE_CFG(GATE_LSE,		RCC_BDCR,		0,	0),
#if STM32MP21
	GATE_CFG(GATE_LSI,		RCC_LSICR,		0,	0),
#else /* STM32MP21 */
	GATE_CFG(GATE_LSI,		RCC_BDCR,		9,	0),
#endif /* STM32MP21 */
	GATE_CFG(GATE_RTCCK,		RCC_BDCR,		20,	0),
	GATE_CFG(GATE_HSI,		RCC_OCENSETR,		0,	1),
	GATE_CFG(GATE_HSE,		RCC_OCENSETR,		8,	1),
#if STM32MP21
	GATE_CFG(GATE_MSI,		RCC_OCENSETR,		2,	0),
#else /* STM32MP21 */
	GATE_CFG(GATE_MSI,		RCC_D3DCR,		0,	0),
#endif /* STM32MP21 */

#if STM32MP21
	GATE_CFG(GATE_LSI_RDY,		RCC_LSICR,		1,	0),
#else /* STM32MP21 */
	GATE_CFG(GATE_LSI_RDY,		RCC_BDCR,		10,	0),
#endif /* STM32MP21 */
	GATE_CFG(GATE_LSE_RDY,		RCC_BDCR,		2,	0),
#if STM32MP21
	GATE_CFG(GATE_MSI_RDY,		RCC_OCRDYR,		2,	0),
#else /* STM32MP21 */
	GATE_CFG(GATE_MSI_RDY,		RCC_D3DCR,		2,	0),
#endif /* STM32MP21 */
	GATE_CFG(GATE_HSE_RDY,		RCC_OCRDYR,		8,	0),
	GATE_CFG(GATE_HSI_RDY,		RCC_OCRDYR,		0,	0),
	GATE_CFG(GATE_SYSRAM,		RCC_SYSRAMCFGR,		1,	0),
	GATE_CFG(GATE_RETRAM,		RCC_RETRAMCFGR,		1,	0),
	GATE_CFG(GATE_SRAM1,		RCC_SRAM1CFGR,		1,	0),
#if !STM32MP21
	GATE_CFG(GATE_SRAM2,		RCC_SRAM2CFGR,		1,	0),
#endif /* !STM32MP21 */
	GATE_CFG(GATE_DDRPHYC,		RCC_DDRPHYCAPBCFGR,	1,	0),
	GATE_CFG(GATE_SYSCPU1,		RCC_SYSCPU1CFGR,	1,	0),
	GATE_CFG(GATE_CRC,		RCC_CRCCFGR,		1,	0),
#if !STM32MP21
	GATE_CFG(GATE_OSPIIOM,		RCC_OSPIIOMCFGR,	1,	0),
#endif /* !STM32MP21 */
	GATE_CFG(GATE_BKPSRAM,		RCC_BKPSRAMCFGR,	1,	0),
#if STM32MP21
	GATE_CFG(GATE_HASH1,		RCC_HASH1CFGR,		1,	0),
	GATE_CFG(GATE_HASH2,		RCC_HASH2CFGR,		1,	0),
	GATE_CFG(GATE_RNG1,		RCC_RNG1CFGR,		1,	0),
	GATE_CFG(GATE_RNG2,		RCC_RNG2CFGR,		1,	0),
#else /* STM32MP21 */
	GATE_CFG(GATE_HASH,		RCC_HASHCFGR,		1,	0),
	GATE_CFG(GATE_RNG,		RCC_RNGCFGR,		1,	0),
#endif /* STM32MP21 */
	GATE_CFG(GATE_CRYP1,		RCC_CRYP1CFGR,		1,	0),
	GATE_CFG(GATE_CRYP2,		RCC_CRYP2CFGR,		1,	0),
	GATE_CFG(GATE_SAES,		RCC_SAESCFGR,		1,	0),
	GATE_CFG(GATE_PKA,		RCC_PKACFGR,		1,	0),
	GATE_CFG(GATE_GPIOA,		RCC_GPIOACFGR,		1,	0),
	GATE_CFG(GATE_GPIOB,		RCC_GPIOBCFGR,		1,	0),
	GATE_CFG(GATE_GPIOC,		RCC_GPIOCCFGR,		1,	0),
	GATE_CFG(GATE_GPIOD,		RCC_GPIODCFGR,		1,	0),
	GATE_CFG(GATE_GPIOE,		RCC_GPIOECFGR,		1,	0),
	GATE_CFG(GATE_GPIOF,		RCC_GPIOFCFGR,		1,	0),
	GATE_CFG(GATE_GPIOG,		RCC_GPIOGCFGR,		1,	0),
	GATE_CFG(GATE_GPIOH,		RCC_GPIOHCFGR,		1,	0),
	GATE_CFG(GATE_GPIOI,		RCC_GPIOICFGR,		1,	0),
#if !STM32MP21
	GATE_CFG(GATE_GPIOJ,		RCC_GPIOJCFGR,		1,	0),
	GATE_CFG(GATE_GPIOK,		RCC_GPIOKCFGR,		1,	0),
#endif /* !STM32MP21 */
	GATE_CFG(GATE_GPIOZ,		RCC_GPIOZCFGR,		1,	0),
	GATE_CFG(GATE_RTC,		RCC_RTCCFGR,		1,	0),
	GATE_CFG(GATE_DDRCP,		RCC_DDRCPCFGR,		1,	0),

	/* WARNING 2 CLOCKS FOR ONE GATE */
#if STM32MP21
	GATE_CFG(GATE_USBHOHCI,		RCC_USBHCFGR,		1,	0),
	GATE_CFG(GATE_USBHEHCI,		RCC_USBHCFGR,		1,	0),
#else /* STM32MP21 */
	GATE_CFG(GATE_USB2OHCI,		RCC_USB2CFGR,		1,	0),
	GATE_CFG(GATE_USB2EHCI,		RCC_USB2CFGR,		1,	0),
	GATE_CFG(GATE_USB3DR,		RCC_USB3DRCFGR,		1,	0),
#endif /* STM32MP21 */
	GATE_CFG(GATE_BSEC,		RCC_BSECCFGR,		1,	0),
	GATE_CFG(GATE_IWDG1,		RCC_IWDG1CFGR,		1,	0),
	GATE_CFG(GATE_IWDG2,		RCC_IWDG2CFGR,		1,	0),
	GATE_CFG(GATE_DDRCAPB,		RCC_DDRCAPBCFGR,	1,	0),
	GATE_CFG(GATE_DDR,		RCC_DDRCFGR,		1,	0),
	GATE_CFG(GATE_USART2,		RCC_USART2CFGR,		1,	0),
	GATE_CFG(GATE_UART4,		RCC_UART4CFGR,		1,	0),
	GATE_CFG(GATE_USART3,		RCC_USART3CFGR,		1,	0),
	GATE_CFG(GATE_UART5,		RCC_UART5CFGR,		1,	0),
	GATE_CFG(GATE_I2C1,		RCC_I2C1CFGR,		1,	0),
	GATE_CFG(GATE_I2C2,		RCC_I2C2CFGR,		1,	0),
#if !STM32MP23
	GATE_CFG(GATE_I2C3,		RCC_I2C3CFGR,		1,	0),
#endif /* !STM32MP23 */
#if STM32MP25
	GATE_CFG(GATE_I2C5,		RCC_I2C5CFGR,		1,	0),
	GATE_CFG(GATE_I2C4,		RCC_I2C4CFGR,		1,	0),
	GATE_CFG(GATE_I2C6,		RCC_I2C6CFGR,		1,	0),
#endif /* STM32MP25 */
#if !STM32MP21
	GATE_CFG(GATE_I2C7,		RCC_I2C7CFGR,		1,	0),
#endif /* !STM32MP21 */
	GATE_CFG(GATE_USART1,		RCC_USART1CFGR,		1,	0),
	GATE_CFG(GATE_USART6,		RCC_USART6CFGR,		1,	0),
	GATE_CFG(GATE_UART7,		RCC_UART7CFGR,		1,	0),
#if STM32MP25
	GATE_CFG(GATE_UART8,		RCC_UART8CFGR,		1,	0),
	GATE_CFG(GATE_UART9,		RCC_UART9CFGR,		1,	0),
#endif /* STM32MP25 */
	GATE_CFG(GATE_STGEN,		RCC_STGENCFGR,		1,	0),
#if !STM32MP21
	GATE_CFG(GATE_USB3PCIEPHY,	RCC_USB3PCIEPHYCFGR,	1,	0),
	GATE_CFG(GATE_USBTC,		RCC_UCPDCFGR,		1,	0),
	GATE_CFG(GATE_I2C8,		RCC_I2C8CFGR,		1,	0),
#endif /* !STM32MP21 */
	GATE_CFG(GATE_OSPI1,		RCC_OSPI1CFGR,		1,	0),
#if !STM32MP21
	GATE_CFG(GATE_OSPI2,		RCC_OSPI2CFGR,		1,	0),
#endif /* !STM32MP21 */
	GATE_CFG(GATE_FMC,		RCC_FMCCFGR,		1,	0),
	GATE_CFG(GATE_SDMMC1,		RCC_SDMMC1CFGR,		1,	0),
	GATE_CFG(GATE_SDMMC2,		RCC_SDMMC2CFGR,		1,	0),
	GATE_CFG(GATE_USB2PHY1,		RCC_USB2PHY1CFGR,	1,	0),
	GATE_CFG(GATE_USB2PHY2,		RCC_USB2PHY2CFGR,	1,	0),
};

static const struct clk_div_table apb_div_table[] = {
	{ 0, 1 },  { 1, 2 },  { 2, 4 },  { 3, 8 }, { 4, 16 },
	{ 5, 16 }, { 6, 16 }, { 7, 16 }, { 0 },
};

#undef DIV_CFG
#define DIV_CFG(id, _offset, _shift, _width, _flags, _table, _bitrdy)[id] = {\
		.offset	= _offset,\
		.shift	= _shift,\
		.width	= _width,\
		.flags	= _flags,\
		.table	= _table,\
		.bitrdy	= _bitrdy,\
}

static const struct div_cfg dividers_mp2[] = {
	DIV_CFG(DIV_APB1, RCC_APB1DIVR, 0, 3, 0, apb_div_table, 31),
	DIV_CFG(DIV_APB2, RCC_APB2DIVR, 0, 3, 0, apb_div_table, 31),
	DIV_CFG(DIV_APB3, RCC_APB3DIVR, 0, 3, 0, apb_div_table, 31),
	DIV_CFG(DIV_APB4, RCC_APB4DIVR, 0, 3, 0, apb_div_table, 31),
#if STM32MP21
	DIV_CFG(DIV_APB5, RCC_APB5DIVR, 0, 3, 0, apb_div_table, 31),
#endif /* STM32MP21 */
	DIV_CFG(DIV_APBDBG, RCC_APBDBGDIVR, 0, 3, 0, apb_div_table, 31),
	DIV_CFG(DIV_LSMCU, RCC_LSMCUDIVR, 0, 1, 0, NULL, 31),
	DIV_CFG(DIV_RTC, RCC_RTCDIVR, 0, 6, 0, NULL, 0),
};

enum stm32_osc {
	OSC_HSI,
	OSC_HSE,
	OSC_MSI,
	OSC_LSI,
	OSC_LSE,
	OSC_I2SCKIN,
	OSC_SPDIFSYMB,
	NB_OSCILLATOR
};

static struct clk_oscillator_data stm32mp2_osc_data[] = {
	OSCILLATOR(OSC_HSI, _CK_HSI, "clk-hsi", GATE_HSI, GATE_HSI_RDY,
		   NULL, NULL, NULL),

	OSCILLATOR(OSC_LSI, _CK_LSI, "clk-lsi", GATE_LSI, GATE_LSI_RDY,
		   NULL, NULL, NULL),

	OSCILLATOR(OSC_MSI, _CK_MSI, "clk-msi", GATE_MSI, GATE_MSI_RDY,
		   NULL, NULL, NULL),

	OSCILLATOR(OSC_HSE, _CK_HSE, "clk-hse", GATE_HSE, GATE_HSE_RDY,
		   BYPASS(RCC_OCENSETR, 10, 7),
		   CSS(RCC_OCENSETR, 11),
		   NULL),

	OSCILLATOR(OSC_LSE, _CK_LSE, "clk-lse", GATE_LSE, GATE_LSE_RDY,
		   BYPASS(RCC_BDCR, 1, 3),
		   CSS(RCC_BDCR, 8),
		   DRIVE(RCC_BDCR, 4, 2, 2)),

	OSCILLATOR(OSC_I2SCKIN, _I2SCKIN, "i2s_ckin", NO_GATE, NO_GATE,
		   NULL, NULL, NULL),

	OSCILLATOR(OSC_SPDIFSYMB, _SPDIFSYMB, "spdif_symb", NO_GATE, NO_GATE,
		   NULL, NULL, NULL),
};

#ifdef IMAGE_BL2
static const char *clk_stm32_get_oscillator_name(enum stm32_osc id)
{
	if (id < NB_OSCILLATOR) {
		return stm32mp2_osc_data[id].name;
	}

	return NULL;
}
#endif

enum pll_id {
	_PLL1,
	_PLL2,
	_PLL3,
	_PLL4,
	_PLL5,
	_PLL6,
	_PLL7,
	_PLL8,
	_PLL_NB
};

/* PLL configuration registers offsets from RCC_PLLxCFGR1 */
#define RCC_OFFSET_PLLXCFGR1		0x00
#define RCC_OFFSET_PLLXCFGR2		0x04
#define RCC_OFFSET_PLLXCFGR3		0x08
#define RCC_OFFSET_PLLXCFGR4		0x0C
#define RCC_OFFSET_PLLXCFGR5		0x10
#define RCC_OFFSET_PLLXCFGR6		0x18
#define RCC_OFFSET_PLLXCFGR7		0x1C

struct stm32_clk_pll {
	uint16_t clk_id;
	uint16_t reg_pllxcfgr1;
};

#define CLK_PLL_CFG(_idx, _clk_id, _reg)\
	[(_idx)] = {\
		.clk_id = (_clk_id),\
		.reg_pllxcfgr1 = (_reg),\
	}

static const struct stm32_clk_pll stm32mp2_clk_pll[_PLL_NB] = {
	CLK_PLL_CFG(_PLL1, _CK_PLL1, A35_SS_CHGCLKREQ),
	CLK_PLL_CFG(_PLL2, _CK_PLL2, RCC_PLL2CFGR1),
#if !STM32MP21
	CLK_PLL_CFG(_PLL3, _CK_PLL3, RCC_PLL3CFGR1),
#endif /* !STM32MP21 */
	CLK_PLL_CFG(_PLL4, _CK_PLL4, RCC_PLL4CFGR1),
	CLK_PLL_CFG(_PLL5, _CK_PLL5, RCC_PLL5CFGR1),
	CLK_PLL_CFG(_PLL6, _CK_PLL6, RCC_PLL6CFGR1),
	CLK_PLL_CFG(_PLL7, _CK_PLL7, RCC_PLL7CFGR1),
	CLK_PLL_CFG(_PLL8, _CK_PLL8, RCC_PLL8CFGR1),
};

static const struct stm32_clk_pll *clk_stm32_pll_data(unsigned int idx)
{
	return &stm32mp2_clk_pll[idx];
}

static unsigned long clk_get_pll_fvco(struct stm32_clk_priv *priv,
				      const struct stm32_clk_pll *pll,
				      unsigned long prate)
{
	unsigned long refclk, fvco;
	uint32_t fracin, fbdiv, refdiv;
	uintptr_t pllxcfgr1 = priv->base + pll->reg_pllxcfgr1;
	uintptr_t pllxcfgr2 = pllxcfgr1 + RCC_OFFSET_PLLXCFGR2;
	uintptr_t pllxcfgr3 = pllxcfgr1 + RCC_OFFSET_PLLXCFGR3;

	refclk = prate;

	fracin = mmio_read_32(pllxcfgr3) & RCC_PLLxCFGR3_FRACIN_MASK;
	fbdiv = (mmio_read_32(pllxcfgr2) & RCC_PLLxCFGR2_FBDIV_MASK) >>
		RCC_PLLxCFGR2_FBDIV_SHIFT;
	refdiv = mmio_read_32(pllxcfgr2) & RCC_PLLxCFGR2_FREFDIV_MASK;

	if (fracin != 0U) {
		uint64_t numerator, denominator;

		numerator = ((uint64_t)fbdiv << 24) + fracin;
		numerator = refclk * numerator;
		denominator = (uint64_t)refdiv << 24;
		fvco = (unsigned long)(numerator / denominator);
	} else {
		fvco = (unsigned long)(refclk * fbdiv / refdiv);
	}

	return fvco;
}

struct stm32_pll_cfg {
	uint16_t pll_id;
};

static bool _clk_stm32_pll_is_enabled(struct stm32_clk_priv *priv, const struct stm32_clk_pll *pll)
{
	uintptr_t pllxcfgr1 = priv->base + pll->reg_pllxcfgr1;

	return ((mmio_read_32(pllxcfgr1) & RCC_PLLxCFGR1_PLLEN) != 0U);
}

static void _clk_stm32_pll_set_on(struct stm32_clk_priv *priv, const struct stm32_clk_pll *pll)
{
	uintptr_t pllxcfgr1 = priv->base + pll->reg_pllxcfgr1;

	mmio_setbits_32(pllxcfgr1, RCC_PLLxCFGR1_PLLEN);
}

static void _clk_stm32_pll_set_off(struct stm32_clk_priv *priv, const struct stm32_clk_pll *pll)
{
	uintptr_t pllxcfgr1 = priv->base + pll->reg_pllxcfgr1;

	/* Stop PLL */
	mmio_clrbits_32(pllxcfgr1, RCC_PLLxCFGR1_PLLEN);
}

static int _clk_stm32_pll_wait_ready_on(struct stm32_clk_priv *priv,
					const struct stm32_clk_pll *pll)
{
	uintptr_t pllxcfgr1 = priv->base + pll->reg_pllxcfgr1;
	uint64_t timeout = timeout_init_us(PLLRDY_TIMEOUT);

	/* Wait PLL lock */
	while ((mmio_read_32(pllxcfgr1) & RCC_PLLxCFGR1_PLLRDY) == 0U) {
		if (timeout_elapsed(timeout)) {
			ERROR("PLL%d start failed @ 0x%x: 0x%x\n",
			      pll->clk_id - _CK_PLL1 + 1, pll->reg_pllxcfgr1,
			      mmio_read_32(pllxcfgr1));
			return -ETIMEDOUT;
		}
	}

	return 0;
}

static int _clk_stm32_pll_wait_ready_off(struct stm32_clk_priv *priv,
					 const struct stm32_clk_pll *pll)
{
	uintptr_t pllxcfgr1 = priv->base + pll->reg_pllxcfgr1;
	uint64_t timeout = timeout_init_us(PLLRDY_TIMEOUT);

	/* Wait PLL stopped */
	while ((mmio_read_32(pllxcfgr1) & RCC_PLLxCFGR1_PLLRDY) != 0U) {
		if (timeout_elapsed(timeout)) {
			ERROR("PLL%d stop failed @ 0x%lx: 0x%x\n",
			      pll->clk_id - _CK_PLL1 + 1, pllxcfgr1, mmio_read_32(pllxcfgr1));
			return -ETIMEDOUT;
		}
	}

	return 0;
}

static int _clk_stm32_pll_enable(struct stm32_clk_priv *priv, const struct stm32_clk_pll *pll)
{
	if (_clk_stm32_pll_is_enabled(priv, pll)) {
		return 0;
	}

	_clk_stm32_pll_set_on(priv, pll);

	return _clk_stm32_pll_wait_ready_on(priv, pll);
}

static void _clk_stm32_pll_disable(struct stm32_clk_priv *priv, const struct stm32_clk_pll *pll)
{
	if (!_clk_stm32_pll_is_enabled(priv, pll)) {
		return;
	}

	_clk_stm32_pll_set_off(priv, pll);

	_clk_stm32_pll_wait_ready_off(priv, pll);
}

static bool clk_stm32_pll_is_enabled(struct stm32_clk_priv *priv, int id)
{
	const struct clk_stm32 *clk = _clk_get(priv, id);
	struct stm32_pll_cfg *pll_cfg = clk->clock_cfg;
	const struct stm32_clk_pll *pll = clk_stm32_pll_data(pll_cfg->pll_id);

	return _clk_stm32_pll_is_enabled(priv, pll);
}

static int clk_stm32_pll_enable(struct stm32_clk_priv *priv, int id)
{
	const struct clk_stm32 *clk = _clk_get(priv, id);
	struct stm32_pll_cfg *pll_cfg = clk->clock_cfg;
	const struct stm32_clk_pll *pll = clk_stm32_pll_data(pll_cfg->pll_id);

	return _clk_stm32_pll_enable(priv, pll);
}

static void clk_stm32_pll_disable(struct stm32_clk_priv *priv, int id)
{
	const struct clk_stm32 *clk = _clk_get(priv, id);
	struct stm32_pll_cfg *pll_cfg = clk->clock_cfg;
	const struct stm32_clk_pll *pll = clk_stm32_pll_data(pll_cfg->pll_id);

	_clk_stm32_pll_disable(priv, pll);
}

static unsigned long clk_stm32_pll_recalc_rate(struct stm32_clk_priv *priv, int id,
					       unsigned long prate)
{
	const struct clk_stm32 *clk = _clk_get(priv, id);
	struct stm32_pll_cfg *pll_cfg = clk->clock_cfg;
	const struct stm32_clk_pll *pll = clk_stm32_pll_data(pll_cfg->pll_id);
	uintptr_t pllxcfgr1 = priv->base + pll->reg_pllxcfgr1;
	uintptr_t pllxcfgr4 = pllxcfgr1 + RCC_OFFSET_PLLXCFGR4;
	uintptr_t pllxcfgr6 = pllxcfgr1 + RCC_OFFSET_PLLXCFGR6;
	uintptr_t pllxcfgr7 = pllxcfgr1 + RCC_OFFSET_PLLXCFGR7;
	unsigned long dfout;
	uint32_t postdiv1, postdiv2;

	postdiv1 = mmio_read_32(pllxcfgr6) & RCC_PLLxCFGR6_POSTDIV1_MASK;
	postdiv2 = mmio_read_32(pllxcfgr7) & RCC_PLLxCFGR7_POSTDIV2_MASK;

	if ((mmio_read_32(pllxcfgr4) & RCC_PLLxCFGR4_BYPASS) != 0U) {
		dfout = prate;
	} else {
		if ((postdiv1 == 0U) || (postdiv2 == 0U)) {
			dfout = prate;
		} else {
			dfout = clk_get_pll_fvco(priv, pll, prate) / (postdiv1 * postdiv2);
		}
	}

	return dfout;
}

static const struct stm32_clk_ops clk_stm32_pll_ops = {
	.recalc_rate	= clk_stm32_pll_recalc_rate,
	.enable		= clk_stm32_pll_enable,
	.disable	= clk_stm32_pll_disable,
	.is_enabled	= clk_stm32_pll_is_enabled,
};

#define CLK_PLL(idx, _idx, _parent, _pll_id, _flags)[idx] = {\
	.binding	= _idx,\
	.parent		= _parent,\
	.flags		= (_flags),\
	.clock_cfg	= &(struct stm32_pll_cfg) {\
		.pll_id	= _pll_id,\
	},\
	.ops		= STM32_PLL_OPS,\
}

static unsigned long clk_get_pll1_fvco(unsigned long refclk)
{
	uintptr_t pll_freq1_reg = A35SSC_BASE + A35_SS_PLL_FREQ1;
	uint32_t reg, fbdiv, refdiv;

	reg = mmio_read_32(pll_freq1_reg);

	fbdiv = (reg & A35_SS_PLL_FREQ1_FBDIV_MASK) >> A35_SS_PLL_FREQ1_FBDIV_SHIFT;
	refdiv = (reg & A35_SS_PLL_FREQ1_REFDIV_MASK) >> A35_SS_PLL_FREQ1_REFDIV_SHIFT;

	return (unsigned long)(refclk * fbdiv / refdiv);
}

static unsigned long clk_stm32_pll1_recalc_rate(struct stm32_clk_priv *priv,
						int id, unsigned long prate)
{
	uintptr_t pll_freq2_reg = A35SSC_BASE + A35_SS_PLL_FREQ2;
	uint32_t postdiv1, postdiv2;
	unsigned long dfout;

	postdiv1 = (mmio_read_32(pll_freq2_reg) & A35_SS_PLL_FREQ2_POSTDIV1_MASK) >>
		   A35_SS_PLL_FREQ2_POSTDIV1_SHIFT;
	postdiv2 = (mmio_read_32(pll_freq2_reg) & A35_SS_PLL_FREQ2_POSTDIV2_MASK) >>
		   A35_SS_PLL_FREQ2_POSTDIV2_SHIFT;

	if ((postdiv1 == 0U) || (postdiv2 == 0U)) {
		dfout = prate;
	} else {
		dfout = clk_get_pll1_fvco(prate) / (postdiv1 * postdiv2);
	}

	return dfout;
}

static const struct stm32_clk_ops clk_stm32_pll1_ops = {
	.recalc_rate = clk_stm32_pll1_recalc_rate,
};

#define CLK_PLL1(idx, _idx, _parent, _pll_id, _flags)[idx] = {\
	.binding	= _idx,\
	.parent		= _parent,\
	.flags		= (_flags),\
	.clock_cfg	= &(struct stm32_pll_cfg) {\
		.pll_id	= _pll_id,\
	},\
	.ops		= STM32_PLL1_OPS,\
}

struct stm32_clk_flexgen_cfg {
	uint8_t id;
};

static unsigned long clk_flexgen_recalc(struct stm32_clk_priv *priv, int idx,
					unsigned long prate)
{
	const struct clk_stm32 *clk = _clk_get(priv, idx);
	struct stm32_clk_flexgen_cfg *cfg = clk->clock_cfg;
	uintptr_t rcc_base = priv->base;
	uint32_t prediv, findiv;
	uint8_t channel = cfg->id;
	unsigned long freq = prate;

	prediv = mmio_read_32(rcc_base + RCC_PREDIV0CFGR + (0x4U * channel)) &
		RCC_PREDIVxCFGR_PREDIVx_MASK;
	findiv = mmio_read_32(rcc_base + RCC_FINDIV0CFGR + (0x4U * channel)) &
		RCC_FINDIVxCFGR_FINDIVx_MASK;

	if (freq == 0UL) {
		return 0U;
	}

	switch (prediv) {
	case 0x0:
	case 0x1:
	case 0x3:
	case 0x3FF:
		break;

	default:
		ERROR("Unsupported PREDIV value (%x)\n", prediv);
		panic();
		break;
	}

	freq /= (prediv + 1U);
	freq /= (findiv + 1U);

	return freq;
}

static int clk_flexgen_get_parent(struct stm32_clk_priv *priv, int idx)
{
	const struct clk_stm32 *clk = _clk_get(priv, idx);
	struct stm32_clk_flexgen_cfg *cfg = clk->clock_cfg;
	uint32_t sel;
	uint32_t address;
	uintptr_t rcc_base = priv->base;

	address = RCC_XBAR0CFGR + (cfg->id * 4);

	sel = mmio_read_32(rcc_base + address) & RCC_XBARxCFGR_XBARxSEL_MASK;

	return sel;
}

static int clk_flexgen_gate_enable(struct stm32_clk_priv *priv, int idx)
{
	const struct clk_stm32 *clk = _clk_get(priv, idx);
	struct stm32_clk_flexgen_cfg *cfg = clk->clock_cfg;
	uintptr_t rcc_base = priv->base;
	uint8_t channel = cfg->id;

	mmio_setbits_32(rcc_base + RCC_FINDIV0CFGR + (0x4U * channel),
			RCC_FINDIVxCFGR_FINDIVxEN);

	return 0;
}

static void clk_flexgen_gate_disable(struct stm32_clk_priv *priv, int id)
{
	const struct clk_stm32 *clk = _clk_get(priv, id);
	struct stm32_clk_flexgen_cfg *cfg = clk->clock_cfg;
	uintptr_t rcc_base = priv->base;
	uint8_t channel = cfg->id;

	mmio_clrbits_32(rcc_base + RCC_FINDIV0CFGR + (0x4U * channel),
			RCC_FINDIVxCFGR_FINDIVxEN);
}

static bool clk_flexgen_gate_is_enabled(struct stm32_clk_priv *priv, int id)
{
	const struct clk_stm32 *clk = _clk_get(priv, id);
	struct stm32_clk_flexgen_cfg *cfg = clk->clock_cfg;
	uintptr_t rcc_base = priv->base;
	uint8_t channel = cfg->id;

	return !!(mmio_read_32(rcc_base + RCC_FINDIV0CFGR + (0x4U * channel)) &
		RCC_FINDIVxCFGR_FINDIVxEN);
}

static const struct stm32_clk_ops clk_stm32_flexgen_ops = {
	.recalc_rate = clk_flexgen_recalc,
	.get_parent = clk_flexgen_get_parent,
	.enable = clk_flexgen_gate_enable,
	.disable = clk_flexgen_gate_disable,
	.is_enabled = clk_flexgen_gate_is_enabled,
};

#define FLEXGEN(idx, _idx, _flags, _id)[idx] = {\
	.binding = _idx,\
	.parent =  MUX(MUX_XBARSEL),\
	.flags = (_flags),\
	.clock_cfg	= &(struct stm32_clk_flexgen_cfg) {\
		.id	= _id,\
	},\
	.ops = STM32_FLEXGEN_OPS,\
}

#define RCC_0_MHZ	UL(0)
#define RCC_4_MHZ	UL(4000000)
#define RCC_16_MHZ	UL(16000000)

#ifdef IMAGE_BL2
#if !STM32MP21
static int clk_stm32_osc_msi_set_rate(struct stm32_clk_priv *priv, int id, unsigned long rate,
				      unsigned long prate)
{
	uintptr_t address = priv->base + RCC_BDCR;
	uint32_t mask = RCC_BDCR_MSIFREQSEL;
	int ret = -1;

	switch (rate) {
	case RCC_4_MHZ:
		mmio_clrbits_32(address, mask);
		ret = 0;
		break;

	case RCC_16_MHZ:
		mmio_setbits_32(address, mask);
		ret = 0;
		break;

	default:
		break;
	}

	return ret;
}
#endif /* !STM32MP21 */
#endif /* IMAGE_BL2 */

static unsigned long clk_stm32_osc_msi_recalc_rate(struct stm32_clk_priv *priv,
						   int id __unused,
						   unsigned long prate __unused)
{
#if STM32MP21
	return RCC_16_MHZ;
#else /* STM32MP21 */
	uintptr_t address = priv->base + RCC_BDCR;

	if ((mmio_read_32(address) & RCC_BDCR_MSIFREQSEL) == 0U) {
		return RCC_4_MHZ;
	} else {
		return RCC_16_MHZ;
	}
#endif /* STM32MP21 */
}

static const struct stm32_clk_ops clk_stm32_osc_msi_ops = {
	.recalc_rate	= clk_stm32_osc_msi_recalc_rate,
	.is_enabled	= clk_stm32_osc_gate_is_enabled,
	.enable		= clk_stm32_osc_gate_enable,
	.disable	= clk_stm32_osc_gate_disable,
	.init		= clk_stm32_osc_init,
};

#define CLK_OSC_MSI(idx, _idx, _parent, _osc_id) \
	[(idx)] = (struct clk_stm32){ \
		.binding	= (_idx),\
		.parent		= (_parent),\
		.flags		= CLK_IS_CRITICAL,\
		.clock_cfg	= &(struct stm32_osc_cfg){\
			.osc_id = (_osc_id),\
		},\
		.ops		= STM32_OSC_MSI_OPS,\
	}

static const struct stm32_clk_ops clk_stm32_rtc_ops = {
	.enable = clk_stm32_gate_enable,
	.disable = clk_stm32_gate_disable,
	.is_enabled = clk_stm32_gate_is_enabled,
};

#define CLK_RTC(idx, _binding, _parent, _flags, _gate_id)[idx] = {\
	.binding = (_binding),\
	.parent =  (_parent),\
	.flags = (_flags),\
	.clock_cfg	= &(struct clk_stm32_gate_cfg) {\
		.id	= (_gate_id),\
	},\
	.ops = STM32_RTC_OPS,\
}

enum {
	STM32_PLL_OPS = STM32_LAST_OPS,
	STM32_PLL1_OPS,
	STM32_FLEXGEN_OPS,
	STM32_OSC_MSI_OPS,
	STM32_RTC_OPS,

	MP2_LAST_OPS
};

static const struct stm32_clk_ops *ops_array_mp2[MP2_LAST_OPS] = {
	[NO_OPS] =  NULL,
	[FIXED_FACTOR_OPS] = &clk_fixed_factor_ops,
	[GATE_OPS] = &clk_gate_ops,
	[STM32_MUX_OPS] = &clk_mux_ops,
	[STM32_DIVIDER_OPS] = &clk_stm32_divider_ops,
	[STM32_GATE_OPS] = &clk_stm32_gate_ops,
	[STM32_TIMER_OPS] = &clk_timer_ops,
	[STM32_FIXED_RATE_OPS] = &clk_stm32_fixed_rate_ops,
	[STM32_OSC_OPS] = &clk_stm32_osc_ops,
	[STM32_OSC_NOGATE_OPS] = &clk_stm32_osc_nogate_ops,

	[STM32_PLL_OPS] = &clk_stm32_pll_ops,
	[STM32_PLL1_OPS] = &clk_stm32_pll1_ops,
	[STM32_FLEXGEN_OPS] = &clk_stm32_flexgen_ops,
	[STM32_OSC_MSI_OPS] = &clk_stm32_osc_msi_ops,
	[STM32_RTC_OPS] = &clk_stm32_rtc_ops
};

static const struct clk_stm32 stm32mp2_clk[CK_LAST] = {
	CLK_FIXED_RATE(_CK_0_MHZ, _NO_ID, RCC_0_MHZ),

	/* ROOT CLOCKS */
	CLK_OSC(_CK_HSE, HSE_CK, CLK_IS_ROOT, OSC_HSE),
	CLK_OSC(_CK_LSE, LSE_CK, CLK_IS_ROOT, OSC_LSE),
	CLK_OSC(_CK_HSI, HSI_CK, CLK_IS_ROOT, OSC_HSI),
	CLK_OSC(_CK_LSI, LSI_CK, CLK_IS_ROOT, OSC_LSI),
	CLK_OSC_MSI(_CK_MSI, MSI_CK, CLK_IS_ROOT, OSC_MSI),

	CLK_OSC_FIXED(_I2SCKIN, _NO_ID, CLK_IS_ROOT, OSC_I2SCKIN),
	CLK_OSC_FIXED(_SPDIFSYMB, _NO_ID, CLK_IS_ROOT, OSC_SPDIFSYMB),

	STM32_DIV(_CK_HSE_RTC, _NO_ID, _CK_HSE, 0, DIV_RTC),

	CLK_RTC(_CK_RTCCK, RTC_CK, MUX(MUX_RTC), 0, GATE_RTCCK),

	CLK_PLL1(_CK_PLL1, PLL1_CK, MUX(MUX_MUXSEL5), _PLL1, 0),

	CLK_PLL(_CK_PLL2, PLL2_CK, MUX(MUX_MUXSEL6), _PLL2, 0),
#if !STM32MP21
	CLK_PLL(_CK_PLL3, PLL3_CK, MUX(MUX_MUXSEL7), _PLL3, 0),
#endif /* !STM32MP21 */
	CLK_PLL(_CK_PLL4, PLL4_CK, MUX(MUX_MUXSEL0), _PLL4, 0),
	CLK_PLL(_CK_PLL5, PLL5_CK, MUX(MUX_MUXSEL1), _PLL5, 0),
	CLK_PLL(_CK_PLL6, PLL6_CK, MUX(MUX_MUXSEL2), _PLL6, 0),
	CLK_PLL(_CK_PLL7, PLL7_CK, MUX(MUX_MUXSEL3), _PLL7, 0),
	CLK_PLL(_CK_PLL8, PLL8_CK, MUX(MUX_MUXSEL4), _PLL8, 0),

	FLEXGEN(_CK_ICN_HS_MCU,	CK_ICN_HS_MCU, CLK_IS_CRITICAL, 0),
	FLEXGEN(_CK_ICN_SDMMC, CK_ICN_SDMMC, CLK_IS_CRITICAL, 1),
	FLEXGEN(_CK_ICN_DDR, CK_ICN_DDR, CLK_IS_CRITICAL, 2),
	FLEXGEN(_CK_ICN_HSL, CK_ICN_HSL, CLK_IS_CRITICAL, 4),
	FLEXGEN(_CK_ICN_NIC, CK_ICN_NIC, CLK_IS_CRITICAL, 5),

	STM32_DIV(_CK_ICN_LS_MCU, CK_ICN_LS_MCU, _CK_ICN_HS_MCU, 0, DIV_LSMCU),

	FLEXGEN(_CK_FLEXGEN_07, CK_FLEXGEN_07, 0, 7),
	FLEXGEN(_CK_FLEXGEN_08, CK_FLEXGEN_08, 0, 8),
	FLEXGEN(_CK_FLEXGEN_09, CK_FLEXGEN_09, 0, 9),
	FLEXGEN(_CK_FLEXGEN_10, CK_FLEXGEN_10, 0, 10),
	FLEXGEN(_CK_FLEXGEN_11, CK_FLEXGEN_11, 0, 11),
	FLEXGEN(_CK_FLEXGEN_12, CK_FLEXGEN_12, 0, 12),
	FLEXGEN(_CK_FLEXGEN_13, CK_FLEXGEN_13, 0, 13),
	FLEXGEN(_CK_FLEXGEN_14, CK_FLEXGEN_14, 0, 14),
	FLEXGEN(_CK_FLEXGEN_15, CK_FLEXGEN_15, 0, 15),
	FLEXGEN(_CK_FLEXGEN_16, CK_FLEXGEN_16, 0, 16),
	FLEXGEN(_CK_FLEXGEN_17, CK_FLEXGEN_17, 0, 17),
	FLEXGEN(_CK_FLEXGEN_18, CK_FLEXGEN_18, 0, 18),
	FLEXGEN(_CK_FLEXGEN_19, CK_FLEXGEN_19, 0, 19),
	FLEXGEN(_CK_FLEXGEN_20, CK_FLEXGEN_20, 0, 20),
	FLEXGEN(_CK_FLEXGEN_21, CK_FLEXGEN_21, 0, 21),
	FLEXGEN(_CK_FLEXGEN_22, CK_FLEXGEN_22, 0, 22),
	FLEXGEN(_CK_FLEXGEN_23, CK_FLEXGEN_23, 0, 23),
	FLEXGEN(_CK_FLEXGEN_24, CK_FLEXGEN_24, 0, 24),
	FLEXGEN(_CK_FLEXGEN_25, CK_FLEXGEN_25, 0, 25),
	FLEXGEN(_CK_FLEXGEN_26, CK_FLEXGEN_26, 0, 26),
	FLEXGEN(_CK_FLEXGEN_27, CK_FLEXGEN_27, 0, 27),
	FLEXGEN(_CK_FLEXGEN_28, CK_FLEXGEN_28, 0, 28),
	FLEXGEN(_CK_FLEXGEN_29, CK_FLEXGEN_29, 0, 29),
	FLEXGEN(_CK_FLEXGEN_30, CK_FLEXGEN_30, 0, 30),
	FLEXGEN(_CK_FLEXGEN_31, CK_FLEXGEN_31, 0, 31),
	FLEXGEN(_CK_FLEXGEN_32, CK_FLEXGEN_32, 0, 32),
	FLEXGEN(_CK_FLEXGEN_33, CK_FLEXGEN_33, 0, 33),
	FLEXGEN(_CK_FLEXGEN_34, CK_FLEXGEN_34, 0, 34),
	FLEXGEN(_CK_FLEXGEN_35, CK_FLEXGEN_35, 0, 35),
	FLEXGEN(_CK_FLEXGEN_36, CK_FLEXGEN_36, 0, 36),
	FLEXGEN(_CK_FLEXGEN_37, CK_FLEXGEN_37, 0, 37),
	FLEXGEN(_CK_FLEXGEN_38, CK_FLEXGEN_38, 0, 38),
	FLEXGEN(_CK_FLEXGEN_39, CK_FLEXGEN_39, 0, 39),
	FLEXGEN(_CK_FLEXGEN_40, CK_FLEXGEN_40, 0, 40),
	FLEXGEN(_CK_FLEXGEN_41, CK_FLEXGEN_41, 0, 41),
	FLEXGEN(_CK_FLEXGEN_42, CK_FLEXGEN_42, 0, 42),
	FLEXGEN(_CK_FLEXGEN_43, CK_FLEXGEN_43, 0, 43),
	FLEXGEN(_CK_FLEXGEN_44, CK_FLEXGEN_44, 0, 44),
	FLEXGEN(_CK_FLEXGEN_45, CK_FLEXGEN_45, 0, 45),
	FLEXGEN(_CK_FLEXGEN_46, CK_FLEXGEN_46, 0, 46),
	FLEXGEN(_CK_FLEXGEN_47, CK_FLEXGEN_47, 0, 47),
	FLEXGEN(_CK_FLEXGEN_48, CK_FLEXGEN_48, 0, 48),
	FLEXGEN(_CK_FLEXGEN_49, CK_FLEXGEN_49, 0, 49),
	FLEXGEN(_CK_FLEXGEN_50, CK_FLEXGEN_50, 0, 50),
	FLEXGEN(_CK_FLEXGEN_51, CK_FLEXGEN_51, 0, 51),
	FLEXGEN(_CK_FLEXGEN_52, CK_FLEXGEN_52, 0, 52),
	FLEXGEN(_CK_FLEXGEN_53, CK_FLEXGEN_53, 0, 53),
	FLEXGEN(_CK_FLEXGEN_54, CK_FLEXGEN_54, 0, 54),
	FLEXGEN(_CK_FLEXGEN_55, CK_FLEXGEN_55, 0, 55),
	FLEXGEN(_CK_FLEXGEN_56, CK_FLEXGEN_56, 0, 56),
	FLEXGEN(_CK_FLEXGEN_57, CK_FLEXGEN_57, 0, 57),
	FLEXGEN(_CK_FLEXGEN_58, CK_FLEXGEN_58, 0, 58),
	FLEXGEN(_CK_FLEXGEN_59, CK_FLEXGEN_59, 0, 59),
	FLEXGEN(_CK_FLEXGEN_60, CK_FLEXGEN_60, 0, 60),
	FLEXGEN(_CK_FLEXGEN_61, CK_FLEXGEN_61, 0, 61),
	FLEXGEN(_CK_FLEXGEN_62, CK_FLEXGEN_62, 0, 62),
	FLEXGEN(_CK_FLEXGEN_63, CK_FLEXGEN_63, 0, 63),

	STM32_DIV(_CK_ICN_APB1, CK_ICN_APB1, _CK_ICN_LS_MCU, 0, DIV_APB1),
	STM32_DIV(_CK_ICN_APB2, CK_ICN_APB2, _CK_ICN_LS_MCU, 0, DIV_APB2),
	STM32_DIV(_CK_ICN_APB3, CK_ICN_APB3, _CK_ICN_LS_MCU, 0, DIV_APB3),
	STM32_DIV(_CK_ICN_APB4, CK_ICN_APB4, _CK_ICN_LS_MCU, 0, DIV_APB4),
#if STM32MP21
	STM32_DIV(_CK_ICN_APB5, CK_ICN_APB5, _CK_ICN_LS_MCU, 0, DIV_APB5),
#endif /* STM32MP21 */
	STM32_DIV(_CK_ICN_APBDBG, CK_ICN_APBDBG, _CK_ICN_LS_MCU, 0, DIV_APBDBG),

	/* KERNEL CLOCK */
	STM32_GATE(_CK_SYSRAM, CK_BUS_SYSRAM, _CK_ICN_HS_MCU, 0, GATE_SYSRAM),
	STM32_GATE(_CK_RETRAM, CK_BUS_RETRAM, _CK_ICN_HS_MCU, 0, GATE_RETRAM),
	STM32_GATE(_CK_SRAM1, CK_BUS_SRAM1, _CK_ICN_HS_MCU, CLK_IS_CRITICAL, GATE_SRAM1),
#if !STM32MP21
	STM32_GATE(_CK_SRAM2, CK_BUS_SRAM2, _CK_ICN_HS_MCU, CLK_IS_CRITICAL, GATE_SRAM2),
#endif /* !STM32MP21 */

	STM32_GATE(_CK_DDRPHYC, CK_BUS_DDRPHYC, _CK_ICN_LS_MCU, 0, GATE_DDRPHYC),
	STM32_GATE(_CK_SYSCPU1, CK_BUS_SYSCPU1, _CK_ICN_LS_MCU, 0, GATE_SYSCPU1),
	STM32_GATE(_CK_CRC, CK_BUS_CRC, _CK_ICN_LS_MCU, 0, GATE_CRC),
#if !STM32MP21
	STM32_GATE(_CK_OSPIIOM, CK_BUS_OSPIIOM, _CK_ICN_LS_MCU, 0, GATE_OSPIIOM),
#endif /* !STM32MP21 */
	STM32_GATE(_CK_BKPSRAM, CK_BUS_BKPSRAM, _CK_ICN_LS_MCU, 0, GATE_BKPSRAM),
#if STM32MP21
	STM32_GATE(_CK_HASH1, CK_BUS_HASH1, _CK_ICN_LS_MCU, 0, GATE_HASH1),
	STM32_GATE(_CK_HASH2, CK_BUS_HASH2, _CK_ICN_LS_MCU, 0, GATE_HASH2),
	STM32_GATE(_CK_RNG1, CK_BUS_RNG1, _CK_ICN_LS_MCU, 0, GATE_RNG1),
	STM32_GATE(_CK_RNG2, CK_BUS_RNG2, _CK_ICN_LS_MCU, 0, GATE_RNG2),
#else /* STM32MP21 */
	STM32_GATE(_CK_HASH, CK_BUS_HASH, _CK_ICN_LS_MCU, 0, GATE_HASH),
	STM32_GATE(_CK_RNG, CK_BUS_RNG, _CK_ICN_LS_MCU, 0, GATE_RNG),
#endif /* STM32MP21 */
	STM32_GATE(_CK_CRYP1, CK_BUS_CRYP1, _CK_ICN_LS_MCU, 0, GATE_CRYP1),
	STM32_GATE(_CK_CRYP2, CK_BUS_CRYP2, _CK_ICN_LS_MCU, 0, GATE_CRYP2),
	STM32_GATE(_CK_SAES, CK_BUS_SAES, _CK_ICN_LS_MCU, 0, GATE_SAES),
	STM32_GATE(_CK_PKA, CK_BUS_PKA, _CK_ICN_LS_MCU, 0, GATE_PKA),

	STM32_GATE(_CK_GPIOA, CK_BUS_GPIOA, _CK_ICN_LS_MCU, 0, GATE_GPIOA),
	STM32_GATE(_CK_GPIOB, CK_BUS_GPIOB, _CK_ICN_LS_MCU, 0, GATE_GPIOB),
	STM32_GATE(_CK_GPIOC, CK_BUS_GPIOC, _CK_ICN_LS_MCU, 0, GATE_GPIOC),
	STM32_GATE(_CK_GPIOD, CK_BUS_GPIOD, _CK_ICN_LS_MCU, 0, GATE_GPIOD),
	STM32_GATE(_CK_GPIOE, CK_BUS_GPIOE, _CK_ICN_LS_MCU, 0, GATE_GPIOE),
	STM32_GATE(_CK_GPIOF, CK_BUS_GPIOF, _CK_ICN_LS_MCU, 0, GATE_GPIOF),
	STM32_GATE(_CK_GPIOG, CK_BUS_GPIOG, _CK_ICN_LS_MCU, 0, GATE_GPIOG),
	STM32_GATE(_CK_GPIOH, CK_BUS_GPIOH, _CK_ICN_LS_MCU, 0, GATE_GPIOH),
	STM32_GATE(_CK_GPIOI, CK_BUS_GPIOI, _CK_ICN_LS_MCU, 0, GATE_GPIOI),
#if !STM32MP21
	STM32_GATE(_CK_GPIOJ, CK_BUS_GPIOJ, _CK_ICN_LS_MCU, 0, GATE_GPIOJ),
	STM32_GATE(_CK_GPIOK, CK_BUS_GPIOK, _CK_ICN_LS_MCU, 0, GATE_GPIOK),
#endif /* !STM32MP21 */
	STM32_GATE(_CK_GPIOZ, CK_BUS_GPIOZ, _CK_ICN_LS_MCU, 0, GATE_GPIOZ),
	STM32_GATE(_CK_RTC, CK_BUS_RTC, _CK_ICN_LS_MCU, 0, GATE_RTC),

	STM32_GATE(_CK_BUS_RISAF4, CK_BUS_RISAF4, _CK_ICN_LS_MCU, CLK_IS_CRITICAL, GATE_DDRCP),
	STM32_GATE(_CK_DDRCP, CK_BUS_DDR, _CK_ICN_DDR, CLK_IS_CRITICAL, GATE_DDRCP),

	/* WARNING 2 CLOCKS FOR ONE GATE */
#if STM32MP21
	STM32_GATE(_CK_USBHOHCI, CK_BUS_USBHOHCI, _CK_ICN_HSL, 0, GATE_USBHOHCI),
	STM32_GATE(_CK_USBHEHCI, CK_BUS_USBHEHCI, _CK_ICN_HSL, 0, GATE_USBHEHCI),
#else /* STM32MP21 */
	STM32_GATE(_CK_USB2OHCI, CK_BUS_USB2OHCI, _CK_ICN_HSL, 0, GATE_USB2OHCI),
	STM32_GATE(_CK_USB2EHCI, CK_BUS_USB2EHCI, _CK_ICN_HSL, 0, GATE_USB2EHCI),
#endif /* STM32MP21 */

#if !STM32MP21
	STM32_GATE(_CK_USB3DR, CK_BUS_USB3DR, _CK_ICN_HSL, 0, GATE_USB3DR),
#endif /* !STM32MP21 */

	STM32_GATE(_CK_BSEC, CK_BUS_BSEC, _CK_ICN_APB3, 0, GATE_BSEC),
	STM32_GATE(_CK_IWDG1, CK_BUS_IWDG1, _CK_ICN_APB3, 0, GATE_IWDG1),
	STM32_GATE(_CK_IWDG2, CK_BUS_IWDG2, _CK_ICN_APB3, 0, GATE_IWDG2),

	STM32_GATE(_CK_DDRCAPB, CK_BUS_DDRC, _CK_ICN_APB4, 0, GATE_DDRCAPB),
	STM32_GATE(_CK_DDR, CK_BUS_DDRCFG, _CK_ICN_APB4, 0, GATE_DDR),

	STM32_GATE(_CK_USART2, CK_KER_USART2, _CK_FLEXGEN_08, 0, GATE_USART2),
	STM32_GATE(_CK_UART4, CK_KER_UART4, _CK_FLEXGEN_08, 0, GATE_UART4),
	STM32_GATE(_CK_USART3, CK_KER_USART3, _CK_FLEXGEN_09, 0, GATE_USART3),
	STM32_GATE(_CK_UART5, CK_KER_UART5, _CK_FLEXGEN_09, 0, GATE_UART5),
#if STM32MP21
	STM32_GATE(_CK_I2C1, CK_KER_I2C1, _CK_FLEXGEN_13, 0, GATE_I2C1),
	STM32_GATE(_CK_I2C2, CK_KER_I2C2, _CK_FLEXGEN_13, 0, GATE_I2C2),
	STM32_GATE(_CK_USART1, CK_KER_USART1, _CK_FLEXGEN_18, 0, GATE_USART1),
	STM32_GATE(_CK_USART6, CK_KER_USART6, _CK_FLEXGEN_19, 0, GATE_USART6),
	STM32_GATE(_CK_UART7, CK_KER_UART7, _CK_FLEXGEN_20, 0, GATE_UART7),
	STM32_GATE(_CK_I2C3, CK_KER_I2C3, _CK_FLEXGEN_38, 0, GATE_I2C3),
#else /* STM32MP21 */
	STM32_GATE(_CK_I2C1, CK_KER_I2C1, _CK_FLEXGEN_12, 0, GATE_I2C1),
	STM32_GATE(_CK_I2C2, CK_KER_I2C2, _CK_FLEXGEN_12, 0, GATE_I2C2),
#if STM32MP25
	STM32_GATE(_CK_I2C3, CK_KER_I2C3, _CK_FLEXGEN_13, 0, GATE_I2C3),
	STM32_GATE(_CK_I2C5, CK_KER_I2C5, _CK_FLEXGEN_13, 0, GATE_I2C5),
	STM32_GATE(_CK_I2C4, CK_KER_I2C4, _CK_FLEXGEN_14, 0, GATE_I2C4),
	STM32_GATE(_CK_I2C6, CK_KER_I2C6, _CK_FLEXGEN_14, 0, GATE_I2C6),
#endif /* STM32MP25 */
	STM32_GATE(_CK_I2C7, CK_KER_I2C7, _CK_FLEXGEN_15, 0, GATE_I2C7),
	STM32_GATE(_CK_USART1, CK_KER_USART1, _CK_FLEXGEN_19, 0, GATE_USART1),
	STM32_GATE(_CK_USART6, CK_KER_USART6, _CK_FLEXGEN_20, 0, GATE_USART6),
	STM32_GATE(_CK_UART7, CK_KER_UART7, _CK_FLEXGEN_21, 0, GATE_UART7),
#if STM32MP25
	STM32_GATE(_CK_UART8, CK_KER_UART8, _CK_FLEXGEN_21, 0, GATE_UART8),
	STM32_GATE(_CK_UART9, CK_KER_UART9, _CK_FLEXGEN_22, 0, GATE_UART9),
#endif /* STM32MP25 */
#endif /* STM32MP21 */
	STM32_GATE(_CK_STGEN, CK_KER_STGEN, _CK_FLEXGEN_33, 0, GATE_STGEN),
#if !STM32MP21
	STM32_GATE(_CK_USB3PCIEPHY, CK_KER_USB3PCIEPHY, _CK_FLEXGEN_34, 0, GATE_USB3PCIEPHY),
	STM32_GATE(_CK_USBTC, CK_KER_USBTC, _CK_FLEXGEN_35, 0, GATE_USBTC),
	STM32_GATE(_CK_I2C8, CK_KER_I2C8, _CK_FLEXGEN_38, 0, GATE_I2C8),
#endif /* !STM32MP21 */
	STM32_GATE(_CK_OSPI1, CK_KER_OSPI1, _CK_FLEXGEN_48, 0, GATE_OSPI1),
#if !STM32MP21
	STM32_GATE(_CK_OSPI2, CK_KER_OSPI2, _CK_FLEXGEN_49, 0, GATE_OSPI2),
#endif /* !STM32MP21 */
	STM32_GATE(_CK_FMC, CK_KER_FMC, _CK_FLEXGEN_50, 0, GATE_FMC),
	STM32_GATE(_CK_SDMMC1, CK_KER_SDMMC1, _CK_FLEXGEN_51, 0, GATE_SDMMC1),
	STM32_GATE(_CK_SDMMC2, CK_KER_SDMMC2, _CK_FLEXGEN_52, 0, GATE_SDMMC2),
	STM32_GATE(_CK_USB2PHY1, CK_KER_USB2PHY1, _CK_FLEXGEN_57, 0, GATE_USB2PHY1),
	STM32_GATE(_CK_USB2PHY2, CK_KER_USB2PHY2, _CK_FLEXGEN_58, 0, GATE_USB2PHY2),
};

enum clksrc_id {
	CLKSRC_CA35SS,
	CLKSRC_PLL1,
	CLKSRC_PLL2,
#if !STM32MP21
	CLKSRC_PLL3,
#endif /* !STM32MP21 */
	CLKSRC_PLL4,
	CLKSRC_PLL5,
	CLKSRC_PLL6,
	CLKSRC_PLL7,
	CLKSRC_PLL8,
	CLKSRC_XBAR_CHANNEL0,
	CLKSRC_XBAR_CHANNEL1,
	CLKSRC_XBAR_CHANNEL2,
	CLKSRC_XBAR_CHANNEL3,
	CLKSRC_XBAR_CHANNEL4,
	CLKSRC_XBAR_CHANNEL5,
	CLKSRC_XBAR_CHANNEL6,
	CLKSRC_XBAR_CHANNEL7,
	CLKSRC_XBAR_CHANNEL8,
	CLKSRC_XBAR_CHANNEL9,
	CLKSRC_XBAR_CHANNEL10,
	CLKSRC_XBAR_CHANNEL11,
	CLKSRC_XBAR_CHANNEL12,
	CLKSRC_XBAR_CHANNEL13,
	CLKSRC_XBAR_CHANNEL14,
	CLKSRC_XBAR_CHANNEL15,
	CLKSRC_XBAR_CHANNEL16,
	CLKSRC_XBAR_CHANNEL17,
	CLKSRC_XBAR_CHANNEL18,
	CLKSRC_XBAR_CHANNEL19,
	CLKSRC_XBAR_CHANNEL20,
	CLKSRC_XBAR_CHANNEL21,
	CLKSRC_XBAR_CHANNEL22,
	CLKSRC_XBAR_CHANNEL23,
	CLKSRC_XBAR_CHANNEL24,
	CLKSRC_XBAR_CHANNEL25,
	CLKSRC_XBAR_CHANNEL26,
	CLKSRC_XBAR_CHANNEL27,
	CLKSRC_XBAR_CHANNEL28,
	CLKSRC_XBAR_CHANNEL29,
	CLKSRC_XBAR_CHANNEL30,
	CLKSRC_XBAR_CHANNEL31,
	CLKSRC_XBAR_CHANNEL32,
	CLKSRC_XBAR_CHANNEL33,
	CLKSRC_XBAR_CHANNEL34,
	CLKSRC_XBAR_CHANNEL35,
	CLKSRC_XBAR_CHANNEL36,
	CLKSRC_XBAR_CHANNEL37,
	CLKSRC_XBAR_CHANNEL38,
	CLKSRC_XBAR_CHANNEL39,
	CLKSRC_XBAR_CHANNEL40,
	CLKSRC_XBAR_CHANNEL41,
	CLKSRC_XBAR_CHANNEL42,
	CLKSRC_XBAR_CHANNEL43,
	CLKSRC_XBAR_CHANNEL44,
	CLKSRC_XBAR_CHANNEL45,
	CLKSRC_XBAR_CHANNEL46,
	CLKSRC_XBAR_CHANNEL47,
	CLKSRC_XBAR_CHANNEL48,
	CLKSRC_XBAR_CHANNEL49,
	CLKSRC_XBAR_CHANNEL50,
	CLKSRC_XBAR_CHANNEL51,
	CLKSRC_XBAR_CHANNEL52,
	CLKSRC_XBAR_CHANNEL53,
	CLKSRC_XBAR_CHANNEL54,
	CLKSRC_XBAR_CHANNEL55,
	CLKSRC_XBAR_CHANNEL56,
	CLKSRC_XBAR_CHANNEL57,
	CLKSRC_XBAR_CHANNEL58,
	CLKSRC_XBAR_CHANNEL59,
	CLKSRC_XBAR_CHANNEL60,
	CLKSRC_XBAR_CHANNEL61,
	CLKSRC_XBAR_CHANNEL62,
	CLKSRC_XBAR_CHANNEL63,
	CLKSRC_RTC,
	CLKSRC_MCO1,
	CLKSRC_MCO2,
	CLKSRC_NB
};

static void stm32mp2_a35_ss_on_hsi(void)
{
	uintptr_t a35_ss_address = A35SSC_BASE;
	uintptr_t chgclkreq_reg = a35_ss_address + A35_SS_CHGCLKREQ;
	uintptr_t pll_enable_reg = a35_ss_address + A35_SS_PLL_ENABLE;
	uint32_t chgclkreq;
	uint64_t timeout;

	chgclkreq = mmio_read_32(chgclkreq_reg);
	if ((chgclkreq & A35_SS_CHGCLKREQ_ARM_CHGCLKACK) == A35_SS_CHGCLKREQ_ARM_CHGCLKACK) {
		/* Nothing to do, clock source is already set on bypass clock */
		return;
	}

	/* for clkext2f frequency at 400MHZ, default flexgen63 config, divider by 2 is required */
	if ((chgclkreq & A35_SS_CHGCLKREQ_ARM_DIVSEL) == A35_SS_CHGCLKREQ_ARM_DIVSEL) {
		mmio_clrbits_32(chgclkreq_reg, A35_SS_CHGCLKREQ_ARM_DIVSEL);
		timeout = timeout_init_us(CLKSRC_TIMEOUT);
		while ((mmio_read_32(chgclkreq_reg) & A35_SS_CHGCLKREQ_ARM_DIVSELACK) ==
		       A35_SS_CHGCLKREQ_ARM_DIVSELACK) {
			if (timeout_elapsed(timeout)) {
				EARLY_ERROR("Cannot set div on A35 bypass clock\n");
				panic();
			}
		}
	}

	mmio_setbits_32(chgclkreq_reg, A35_SS_CHGCLKREQ_ARM_CHGCLKREQ);

	timeout = timeout_init_us(CLKSRC_TIMEOUT);
	while ((mmio_read_32(chgclkreq_reg) & A35_SS_CHGCLKREQ_ARM_CHGCLKACK) !=
	       A35_SS_CHGCLKREQ_ARM_CHGCLKACK) {
		if (timeout_elapsed(timeout)) {
			EARLY_ERROR("Cannot switch A35 to bypass clock\n");
			panic();
		}
	}

	mmio_clrbits_32(pll_enable_reg, A35_SS_PLL_ENABLE_NRESET_SWPLL_FF);
}

#ifdef IMAGE_BL2
static void stm32mp2_clk_muxsel_on_hsi(struct stm32_clk_priv *priv)
{
	mmio_clrbits_32(priv->base + RCC_MUXSELCFGR,
			RCC_MUXSELCFGR_MUXSEL0_MASK |
			RCC_MUXSELCFGR_MUXSEL1_MASK |
			RCC_MUXSELCFGR_MUXSEL2_MASK |
			RCC_MUXSELCFGR_MUXSEL3_MASK |
			RCC_MUXSELCFGR_MUXSEL4_MASK |
			RCC_MUXSELCFGR_MUXSEL5_MASK |
			RCC_MUXSELCFGR_MUXSEL6_MASK |
			RCC_MUXSELCFGR_MUXSEL7_MASK);
}

static void stm32mp2_clk_xbar_on_hsi(struct stm32_clk_priv *priv)
{
	uintptr_t xbar0cfgr = priv->base + RCC_XBAR0CFGR;
	uint32_t i;

	for (i = 0; i < XBAR_CHANNEL_NB; i++) {
		mmio_clrsetbits_32(xbar0cfgr + (0x4 * i),
				   RCC_XBAR0CFGR_XBAR0SEL_MASK,
				   XBAR_SRC_HSI);
	}
}

static int stm32mp2_a35_pll1_start(void)
{
	uintptr_t a35_ss_address = A35SSC_BASE;
	uintptr_t pll_enable_reg = a35_ss_address + A35_SS_PLL_ENABLE;
	uintptr_t chgclkreq_reg = a35_ss_address + A35_SS_CHGCLKREQ;
	uint64_t timeout = timeout_init_us(PLLRDY_TIMEOUT);

	mmio_setbits_32(pll_enable_reg, A35_SS_PLL_ENABLE_PD);

	/* Wait PLL lock */
	while ((mmio_read_32(pll_enable_reg) & A35_SS_PLL_ENABLE_LOCKP) == 0U) {
		if (timeout_elapsed(timeout)) {
			EARLY_ERROR("PLL1 start failed @ 0x%lx: 0x%x\n",
				    pll_enable_reg, mmio_read_32(pll_enable_reg));
			return -ETIMEDOUT;
		}
	}

	/* De-assert reset on PLL output clock path */
	mmio_setbits_32(pll_enable_reg, A35_SS_PLL_ENABLE_NRESET_SWPLL_FF);

	/* Switch CPU clock to PLL clock */
	mmio_clrbits_32(chgclkreq_reg, A35_SS_CHGCLKREQ_ARM_CHGCLKREQ);

	/* Wait for clock change acknowledge */
	timeout = timeout_init_us(CLKSRC_TIMEOUT);
	while ((mmio_read_32(chgclkreq_reg) & A35_SS_CHGCLKREQ_ARM_CHGCLKACK) != 0U) {
		if (timeout_elapsed(timeout)) {
			EARLY_ERROR("CA35SS switch to PLL1 failed @ 0x%lx: 0x%x\n",
				    chgclkreq_reg, mmio_read_32(chgclkreq_reg));
			return -ETIMEDOUT;
		}
	}

	return 0;
}

static void stm32mp2_a35_pll1_config(uint32_t fbdiv, uint32_t refdiv, uint32_t postdiv1,
				     uint32_t postdiv2)
{
	uintptr_t a35_ss_address = A35SSC_BASE;
	uintptr_t pll_freq1_reg = a35_ss_address + A35_SS_PLL_FREQ1;
	uintptr_t pll_freq2_reg = a35_ss_address + A35_SS_PLL_FREQ2;

	mmio_clrsetbits_32(pll_freq1_reg, A35_SS_PLL_FREQ1_REFDIV_MASK,
			   (refdiv << A35_SS_PLL_FREQ1_REFDIV_SHIFT) &
			   A35_SS_PLL_FREQ1_REFDIV_MASK);

	mmio_clrsetbits_32(pll_freq1_reg, A35_SS_PLL_FREQ1_FBDIV_MASK,
			   (fbdiv << A35_SS_PLL_FREQ1_FBDIV_SHIFT) &
			   A35_SS_PLL_FREQ1_FBDIV_MASK);

	mmio_clrsetbits_32(pll_freq2_reg, A35_SS_PLL_FREQ2_POSTDIV1_MASK,
			   (postdiv1 << A35_SS_PLL_FREQ2_POSTDIV1_SHIFT) &
			   A35_SS_PLL_FREQ2_POSTDIV1_MASK);

	mmio_clrsetbits_32(pll_freq2_reg, A35_SS_PLL_FREQ2_POSTDIV2_MASK,
			   (postdiv2 << A35_SS_PLL_FREQ2_POSTDIV2_SHIFT) &
			   A35_SS_PLL_FREQ2_POSTDIV2_MASK);
}

static int clk_stm32_pll_config_output(struct stm32_clk_priv *priv,
				       const struct stm32_clk_pll *pll,
				       uint32_t *pllcfg,
				       uint32_t fracv)
{
	uintptr_t pllxcfgr1 = priv->base + pll->reg_pllxcfgr1;
	uintptr_t pllxcfgr2 = pllxcfgr1 + RCC_OFFSET_PLLXCFGR2;
	uintptr_t pllxcfgr3 = pllxcfgr1 + RCC_OFFSET_PLLXCFGR3;
	uintptr_t pllxcfgr4 = pllxcfgr1 + RCC_OFFSET_PLLXCFGR4;
	uintptr_t pllxcfgr6 = pllxcfgr1 + RCC_OFFSET_PLLXCFGR6;
	uintptr_t pllxcfgr7 = pllxcfgr1 + RCC_OFFSET_PLLXCFGR7;
	unsigned long refclk;

	refclk = _clk_stm32_get_parent_rate(priv, pll->clk_id);

	if (fracv == 0U) {
		/* PLL in integer mode */

		/*
		 * No need to check max clock, as oscillator reference clocks
		 * will always be less than 1.2GHz
		 */
		if (refclk < PLL_REFCLK_MIN) {
			panic();
		}

		mmio_clrbits_32(pllxcfgr3, RCC_PLLxCFGR3_FRACIN_MASK);
		mmio_clrbits_32(pllxcfgr4, RCC_PLLxCFGR4_DSMEN);
		mmio_clrbits_32(pllxcfgr3, RCC_PLLxCFGR3_DACEN);
		mmio_setbits_32(pllxcfgr3, RCC_PLLxCFGR3_SSCGDIS);
		mmio_setbits_32(pllxcfgr1, RCC_PLLxCFGR1_SSMODRST);
	} else {
		/* PLL in frac mode */

		/*
		 * No need to check max clock, as oscillator reference clocks
		 * will always be less than 1.2GHz
		 */
		if (refclk < PLL_FRAC_REFCLK_MIN) {
			panic();
		}

		mmio_clrsetbits_32(pllxcfgr3, RCC_PLLxCFGR3_FRACIN_MASK,
				   fracv & RCC_PLLxCFGR3_FRACIN_MASK);
		mmio_setbits_32(pllxcfgr3, RCC_PLLxCFGR3_SSCGDIS);
		mmio_setbits_32(pllxcfgr4, RCC_PLLxCFGR4_DSMEN);
	}

	assert(pllcfg[REFDIV] != 0U);

	mmio_clrsetbits_32(pllxcfgr2, RCC_PLLxCFGR2_FBDIV_MASK,
			   (pllcfg[FBDIV] << RCC_PLLxCFGR2_FBDIV_SHIFT) &
			   RCC_PLLxCFGR2_FBDIV_MASK);
	mmio_clrsetbits_32(pllxcfgr2, RCC_PLLxCFGR2_FREFDIV_MASK,
			   pllcfg[REFDIV] & RCC_PLLxCFGR2_FREFDIV_MASK);
	mmio_clrsetbits_32(pllxcfgr6, RCC_PLLxCFGR6_POSTDIV1_MASK,
			   pllcfg[POSTDIV1] & RCC_PLLxCFGR6_POSTDIV1_MASK);
	mmio_clrsetbits_32(pllxcfgr7, RCC_PLLxCFGR7_POSTDIV2_MASK,
			   pllcfg[POSTDIV2] & RCC_PLLxCFGR7_POSTDIV2_MASK);

	if ((pllcfg[POSTDIV1] == 0U) || (pllcfg[POSTDIV2] == 0U)) {
		/* Bypass mode */
		mmio_setbits_32(pllxcfgr4, RCC_PLLxCFGR4_BYPASS);
		mmio_clrbits_32(pllxcfgr4, RCC_PLLxCFGR4_FOUTPOSTDIVEN);
	} else {
		mmio_clrbits_32(pllxcfgr4, RCC_PLLxCFGR4_BYPASS);
		mmio_setbits_32(pllxcfgr4, RCC_PLLxCFGR4_FOUTPOSTDIVEN);
	}

	return 0;
}

static void clk_stm32_pll_config_csg(struct stm32_clk_priv *priv,
				     const struct stm32_clk_pll *pll,
				     uint32_t *csg)
{
	uintptr_t pllxcfgr1 = priv->base + pll->reg_pllxcfgr1;
	uintptr_t pllxcfgr3 = pllxcfgr1 + RCC_OFFSET_PLLXCFGR3;
	uintptr_t pllxcfgr4 = pllxcfgr1 + RCC_OFFSET_PLLXCFGR4;
	uintptr_t pllxcfgr5 = pllxcfgr1 + RCC_OFFSET_PLLXCFGR5;


	mmio_clrsetbits_32(pllxcfgr5, RCC_PLLxCFGR5_DIVVAL_MASK,
			   csg[DIVVAL] & RCC_PLLxCFGR5_DIVVAL_MASK);
	mmio_clrsetbits_32(pllxcfgr5, RCC_PLLxCFGR5_SPREAD_MASK,
			   (csg[SPREAD] << RCC_PLLxCFGR5_SPREAD_SHIFT) &
			   RCC_PLLxCFGR5_SPREAD_MASK);

	if (csg[DOWNSPREAD] != 0) {
		mmio_setbits_32(pllxcfgr3, RCC_PLLxCFGR3_DOWNSPREAD);
	} else {
		mmio_clrbits_32(pllxcfgr3, RCC_PLLxCFGR3_DOWNSPREAD);
	}

	mmio_clrbits_32(pllxcfgr3, RCC_PLLxCFGR3_SSCGDIS);

	mmio_clrbits_32(pllxcfgr1, RCC_PLLxCFGR1_PLLEN);
	udelay(1);

	mmio_setbits_32(pllxcfgr4, RCC_PLLxCFGR4_DSMEN);
	mmio_setbits_32(pllxcfgr3, RCC_PLLxCFGR3_DACEN);
}

static int stm32_clk_configure_mux(struct stm32_clk_priv *priv, uint32_t data);

static inline struct stm32_pll_dt_cfg *clk_stm32_pll_get_pdata(int pll_idx)
{
	struct stm32_clk_priv *priv = clk_stm32_get_priv();
	struct stm32_clk_platdata *pdata = priv->pdata;

	return  &pdata->pll[pll_idx];
}

static int _clk_stm32_pll1_init(struct stm32_clk_priv *priv, int pll_idx,
				struct stm32_pll_dt_cfg *pll_conf)
{
	const struct stm32_clk_pll *pll = clk_stm32_pll_data(pll_idx);
	unsigned long refclk;
	int ret = 0;

	stm32mp2_a35_ss_on_hsi();

	ret = stm32_clk_configure_mux(priv, pll_conf->src);
	if (ret != 0) {
		panic();
	}

	refclk = _clk_stm32_get_parent_rate(priv, pll->clk_id);

	/*
	 * No need to check max clock, as oscillator reference clocks will
	 * always be less than 1.2 GHz
	 */
	if (refclk < PLL_REFCLK_MIN) {
		EARLY_ERROR("%s: %d\n", __func__, __LINE__);
		panic();
	}

	stm32mp2_a35_pll1_config(pll_conf->cfg[FBDIV], pll_conf->cfg[REFDIV],
				 pll_conf->cfg[POSTDIV1], pll_conf->cfg[POSTDIV2]);

	ret = stm32mp2_a35_pll1_start();
	if (ret != 0) {
		panic();
	}

	return 0;
}

static int clk_stm32_pll_wait_mux_ready(struct stm32_clk_priv *priv,
					const struct stm32_clk_pll *pll)
{
	uintptr_t pllxcfgr1 = priv->base + pll->reg_pllxcfgr1;
	uint64_t timeout = timeout_init_us(CLKSRC_TIMEOUT);

	while ((mmio_read_32(pllxcfgr1) & RCC_PLLxCFGR1_CKREFST) !=
	       RCC_PLLxCFGR1_CKREFST) {
		if (timeout_elapsed(timeout)) {
			EARLY_ERROR("PLL%d ref clock not started\n", pll->clk_id - _CK_PLL1 + 1);
			return -ETIMEDOUT;
		}
	}

	return 0;
}

static int _clk_stm32_pll_init(struct stm32_clk_priv *priv, int pll_idx,
			       struct stm32_pll_dt_cfg *pll_conf)
{
	const struct stm32_clk_pll *pll = clk_stm32_pll_data(pll_idx);
	uintptr_t pllxcfgr1 = priv->base + pll->reg_pllxcfgr1;
	bool spread_spectrum = false;
	int ret = 0;

	_clk_stm32_pll_disable(priv, pll);

	ret = stm32_clk_configure_mux(priv, pll_conf->src);
	if (ret != 0) {
		panic();
	}

	ret = clk_stm32_pll_wait_mux_ready(priv, pll);
	if (ret != 0) {
		panic();
	}

	ret = clk_stm32_pll_config_output(priv, pll, pll_conf->cfg, pll_conf->frac);
	if (ret != 0) {
		panic();
	}

	if (pll_conf->csg_enabled) {
		clk_stm32_pll_config_csg(priv, pll, pll_conf->csg);
		spread_spectrum = true;
	}

	_clk_stm32_pll_enable(priv, pll);

	if (spread_spectrum) {
		mmio_clrbits_32(pllxcfgr1, RCC_PLLxCFGR1_SSMODRST);
	}

	return 0;
}

static int clk_stm32_pll_init(struct stm32_clk_priv *priv, int pll_idx)
{
	struct stm32_pll_dt_cfg *pll_conf = clk_stm32_pll_get_pdata(pll_idx);

	if (pll_conf->enabled) {
		if (pll_idx == _PLL1) {
			return _clk_stm32_pll1_init(priv, pll_idx, pll_conf);
		} else  {
			return _clk_stm32_pll_init(priv, pll_idx, pll_conf);
		}
	}

	return 0;
}

static int stm32mp2_clk_pll_configure(struct stm32_clk_priv *priv)
{
	enum pll_id i;
	int err;

	for (i = _PLL1; i < _PLL_NB; i++) {
#if STM32MP21
		if (i == _PLL3) {
			continue;
		}
#endif
		err = clk_stm32_pll_init(priv, i);
		if (err) {
			return err;
		}
	}

	return 0;
}

static int wait_predivsr(uint16_t channel)
{
	struct stm32_clk_priv *priv = clk_stm32_get_priv();
	uintptr_t rcc_base = priv->base;
	uintptr_t previvsr;
	uint32_t channel_bit;
	uint64_t timeout;

	if (channel < __WORD_BIT) {
		previvsr = rcc_base + RCC_PREDIVSR1;
		channel_bit = BIT(channel);
	} else {
		previvsr = rcc_base + RCC_PREDIVSR2;
		channel_bit = BIT(channel - __WORD_BIT);
	}

	timeout = timeout_init_us(CLKDIV_TIMEOUT);
	while ((mmio_read_32(previvsr) & channel_bit) != 0U) {
		if (timeout_elapsed(timeout)) {
			EARLY_ERROR("Pre divider status: %x\n",
			      mmio_read_32(previvsr));
			return -ETIMEDOUT;
		}
	}

	return 0;
}

static int wait_findivsr(uint16_t channel)
{
	struct stm32_clk_priv *priv = clk_stm32_get_priv();
	uintptr_t rcc_base = priv->base;
	uintptr_t finvivsr;
	uint32_t channel_bit;
	uint64_t timeout;

	if (channel < __WORD_BIT) {
		finvivsr = rcc_base + RCC_FINDIVSR1;
		channel_bit = BIT(channel);
	} else {
		finvivsr = rcc_base + RCC_FINDIVSR2;
		channel_bit = BIT(channel - __WORD_BIT);
	}

	timeout = timeout_init_us(CLKDIV_TIMEOUT);
	while ((mmio_read_32(finvivsr) & channel_bit) != 0U) {
		if (timeout_elapsed(timeout)) {
			EARLY_ERROR("Final divider status: %x\n",
			      mmio_read_32(finvivsr));
			return -ETIMEDOUT;
		}
	}

	return 0;
}

static int wait_xbar_sts(uint16_t channel)
{
	struct stm32_clk_priv *priv = clk_stm32_get_priv();
	uintptr_t rcc_base = priv->base;
	uintptr_t xbar_cfgr = rcc_base + RCC_XBAR0CFGR + (0x4U * channel);
	uint64_t timeout;

	timeout = timeout_init_us(CLKDIV_TIMEOUT);
	while ((mmio_read_32(xbar_cfgr) & RCC_XBAR0CFGR_XBAR0STS) != 0U) {
		if (timeout_elapsed(timeout)) {
			EARLY_ERROR("XBAR%uCFGR: %x\n", channel,
			      mmio_read_32(xbar_cfgr));
			return -ETIMEDOUT;
		}
	}

	return 0;
}

static void flexclkgen_config_channel(uint16_t channel, unsigned int clk_src,
				      unsigned int prediv, unsigned int findiv)
{
	struct stm32_clk_priv *priv = clk_stm32_get_priv();
	uintptr_t rcc_base = priv->base;

	if (wait_predivsr(channel) != 0) {
		panic();
	}

	mmio_clrsetbits_32(rcc_base + RCC_PREDIV0CFGR + (0x4U * channel),
			   RCC_PREDIV0CFGR_PREDIV0_MASK,
			   prediv);

	if (wait_predivsr(channel) != 0) {
		panic();
	}

	if (wait_findivsr(channel) != 0) {
		panic();
	}

	mmio_clrsetbits_32(rcc_base + RCC_FINDIV0CFGR + (0x4U * channel),
			   RCC_FINDIV0CFGR_FINDIV0_MASK,
			   findiv);

	if (wait_findivsr(channel) != 0) {
		panic();
	}

	if (wait_xbar_sts(channel) != 0) {
		panic();
	}

	mmio_clrsetbits_32(rcc_base + RCC_XBAR0CFGR + (0x4U * channel),
			   RCC_XBARxCFGR_XBARxSEL_MASK,
			   clk_src);
	mmio_setbits_32(rcc_base + RCC_XBAR0CFGR + (0x4U * channel),
			RCC_XBARxCFGR_XBARxEN);

	if (wait_xbar_sts(channel) != 0) {
		panic();
	}
}

static int stm32mp2_clk_flexgen_configure(struct stm32_clk_priv *priv)
{
	struct stm32_clk_platdata *pdata = priv->pdata;
	uint32_t i;

	for (i = 0U; i < pdata->nflexgen; i++) {
		uint32_t val = pdata->flexgen[i];
		uint32_t cmd, cmd_data;
		unsigned int channel, clk_src, pdiv, fdiv;

		cmd = (val & CMD_MASK) >> CMD_SHIFT;
		cmd_data = val & ~CMD_MASK;

		if (cmd != CMD_FLEXGEN) {
			continue;
		}

		channel = (cmd_data & FLEX_ID_MASK) >> FLEX_ID_SHIFT;
		clk_src = (cmd_data & FLEX_SEL_MASK) >> FLEX_SEL_SHIFT;
		pdiv = (cmd_data & FLEX_PDIV_MASK) >> FLEX_PDIV_SHIFT;
		fdiv = (cmd_data & FLEX_FDIV_MASK) >> FLEX_FDIV_SHIFT;

		switch (channel) {
		case 33U: /* STGEN */
			break;

		default:
			flexclkgen_config_channel(channel, clk_src, pdiv, fdiv);
			break;
		}
	}

	return 0;
}

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

	/* Do not reconfigure LSE if already enabled */
	if (_clk_stm32_gate_is_enabled(priv, osc_data->gate_id)) {
		return;
	}

	clk_oscillator_set_bypass(priv, _CK_LSE, digbyp, bypass);

	clk_oscillator_set_drive(priv, _CK_LSE, drive);

	_clk_stm32_gate_enable(priv, osc_data->gate_id);
}

static int stm32mp2_clk_switch_to_hsi(struct stm32_clk_priv *priv)
{
	stm32mp2_a35_ss_on_hsi();
	stm32mp2_clk_muxsel_on_hsi(priv);
	stm32mp2_clk_xbar_on_hsi(priv);

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

static void stm32_enable_oscillator_msi(struct stm32_clk_priv *priv)
{
#if !STM32MP21
	struct stm32_clk_platdata *pdata = priv->pdata;
	struct stm32_osci_dt_cfg *osci = &pdata->osci[OSC_MSI];
	int err;

	err = clk_stm32_osc_msi_set_rate(priv, _CK_MSI, osci->freq, 0);
	if (err != 0) {
		EARLY_ERROR("Invalid rate %lu MHz for MSI ! (4 or 16 only)\n",
			    osci->freq / 1000000U);
		panic();
	}
#endif /* !STM32MP21 */

	_clk_stm32_enable(priv, _CK_MSI);
}

static void stm32_clk_oscillators_enable(struct stm32_clk_priv *priv)
{
	stm32_enable_oscillator_hse(priv);
	stm32_enable_oscillator_lse(priv);
	stm32_enable_oscillator_msi(priv);
	_clk_stm32_enable(priv, _CK_LSI);
}

static int stm32_clk_configure_div(struct stm32_clk_priv *priv, uint32_t data)
{
	int div_id = (data & DIV_ID_MASK) >> DIV_ID_SHIFT;
	int div_n = (data & DIV_DIVN_MASK) >> DIV_DIVN_SHIFT;

	return clk_stm32_set_div(priv, div_id, div_n);
}

static int stm32_clk_configure_mux(struct stm32_clk_priv *priv, uint32_t data)
{
	int mux_id = (data & MUX_ID_MASK) >> MUX_ID_SHIFT;
	int sel = (data & MUX_SEL_MASK) >> MUX_SEL_SHIFT;

	return clk_mux_set_parent(priv, mux_id, sel);
}

static int stm32_clk_configure_clk_get_binding_id(struct stm32_clk_priv *priv, uint32_t data)
{
	unsigned long binding_id = ((unsigned long)data & CLK_ID_MASK) >> CLK_ID_SHIFT;

	return clk_get_index(priv, binding_id);
}

static int stm32_clk_configure_clk(struct stm32_clk_priv *priv, uint32_t data)
{
	int sel = (data & CLK_SEL_MASK) >> CLK_SEL_SHIFT;
	bool enable = ((data & CLK_ON_MASK) >> CLK_ON_SHIFT) != 0U;
	int clk_id = 0;
	int ret = 0;

	clk_id = stm32_clk_configure_clk_get_binding_id(priv, data);
	if (clk_id < 0) {
		return clk_id;
	}

	if (sel != CLK_NOMUX) {
		ret = _clk_stm32_set_parent_by_index(priv, clk_id, sel);
		if (ret != 0) {
			return ret;
		}
	}

	if (enable) {
		clk_stm32_enable_call_ops(priv, clk_id);
	} else {
		clk_stm32_disable_call_ops(priv, clk_id);
	}

	return 0;
}

static int stm32_clk_configure(struct stm32_clk_priv *priv, uint32_t val)
{
	uint32_t cmd = (val & CMD_MASK) >> CMD_SHIFT;
	uint32_t cmd_data = val & ~CMD_MASK;
	int ret = -1;

	switch (cmd) {
	case CMD_DIV:
		ret = stm32_clk_configure_div(priv, cmd_data);
		break;

	case CMD_MUX:
		ret = stm32_clk_configure_mux(priv, cmd_data);
		break;

	case CMD_CLK:
		ret = stm32_clk_configure_clk(priv, cmd_data);
		break;

	default:
		EARLY_ERROR("%s: cmd unknown ! : 0x%x\n", __func__, val);
		break;
	}

	return ret;
}

static int stm32_clk_bus_configure(struct stm32_clk_priv *priv)
{
	struct stm32_clk_platdata *pdata = priv->pdata;
	uint32_t i;

	for (i = 0; i < pdata->nbusclk; i++) {
		int ret;

		ret = stm32_clk_configure(priv, pdata->busclk[i]);
		if (ret != 0) {
			return ret;
		}
	}

	return 0;
}

static int stm32_clk_kernel_configure(struct stm32_clk_priv *priv)
{
	struct stm32_clk_platdata *pdata = priv->pdata;
	uint32_t i;

	for (i = 0U; i < pdata->nkernelclk; i++) {
		int ret;

		ret = stm32_clk_configure(priv, pdata->kernelclk[i]);
		if (ret != 0) {
			return ret;
		}
	}

	return 0;
}

static int stm32mp2_init_clock_tree(void)
{
	struct stm32_clk_priv *priv = clk_stm32_get_priv();
	int ret;

	/* Set timer with STGEN without changing its clock source */
	stm32mp_stgen_restore_rate();
	generic_delay_timer_init();

	stm32_clk_oscillators_enable(priv);

	/* Come back to HSI */
	ret = stm32mp2_clk_switch_to_hsi(priv);
	if (ret != 0) {
		panic();
	}

	ret = stm32mp2_clk_pll_configure(priv);
	if (ret != 0) {
		panic();
	}

	/* Wait LSE ready before to use it */
	ret = stm32_clk_oscillators_wait_lse_ready(priv);
	if (ret != 0) {
		panic();
	}

	ret = stm32mp2_clk_flexgen_configure(priv);
	if (ret != 0) {
		panic();
	}

	ret = stm32_clk_bus_configure(priv);
	if (ret != 0) {
		panic();
	}

	ret = stm32_clk_kernel_configure(priv);
	if (ret != 0) {
		panic();
	}

	return 0;
}

static int clk_stm32_parse_oscillator_fdt(void *fdt, int node, const char *name,
					  struct stm32_osci_dt_cfg *osci)
{
	int subnode = 0;

	/* Default value oscillator not found, freq=0 */
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

static int clk_stm32_parse_pll_fdt(void *fdt, int subnode, struct stm32_pll_dt_cfg *pll)
{
	const fdt32_t *cuint = NULL;
	int subnode_pll = 0;
	uint32_t val = 0;
	int err = 0;

	cuint = fdt_getprop(fdt, subnode, "st,pll", NULL);
	if (!cuint) {
		return -FDT_ERR_NOTFOUND;
	}

	subnode_pll = fdt_node_offset_by_phandle(fdt, fdt32_to_cpu(*cuint));
	if (subnode_pll < 0) {
		return -FDT_ERR_NOTFOUND;
	}

	err = fdt_read_uint32_array(fdt, subnode_pll, "cfg", (int)PLLCFG_NB, pll->cfg);
	if (err != 0) {
		return err;
	}

	err = fdt_read_uint32_array(fdt, subnode_pll, "csg", (int)PLLCSG_NB, pll->csg);

	pll->csg_enabled = (err == 0);

	if (err == -FDT_ERR_NOTFOUND) {
		err = 0;
	}

	if (err != 0) {
		return err;
	}

	pll->enabled = true;

	pll->frac = fdt_read_uint32_default(fdt, subnode_pll, "frac", 0);

	pll->src = UINT32_MAX;

	err = fdt_read_uint32(fdt, subnode_pll, "src", &val);
	if  (err == 0) {
		pll->src = val;
	}

	return 0;
}

#define RCC_PLL_NAME_SIZE 12

static int stm32_clk_parse_fdt_all_pll(void *fdt, int node, struct stm32_clk_platdata *pdata)
{
	unsigned int i = 0;

	for (i = _PLL1; i < pdata->npll; i++) {
		struct stm32_pll_dt_cfg *pll = pdata->pll + i;
		char name[RCC_PLL_NAME_SIZE];
		int subnode = 0;
		int err = 0;

#if STM32MP21
		if (i == _PLL3) {
			continue;
		}
#endif
		snprintf(name, sizeof(name), "st,pll-%u", i + 1);

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
	int err;

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

	err = stm32_clk_parse_fdt_by_name(fdt, node, "st,busclk", pdata->busclk, &pdata->nbusclk);
	if (err != 0) {
		return err;
	}

	err = stm32_clk_parse_fdt_by_name(fdt, node, "st,flexgen", pdata->flexgen,
					  &pdata->nflexgen);
	if (err != 0) {
		return err;
	}

	err = stm32_clk_parse_fdt_by_name(fdt, node, "st,kerclk", pdata->kernelclk,
					  &pdata->nkernelclk);
	if (err != 0) {
		return err;
	}

	return 0;
}
#endif /* IMAGE_BL2 */

static struct stm32_osci_dt_cfg mp2_osci[NB_OSCILLATOR];

static struct stm32_pll_dt_cfg mp2_pll[_PLL_NB];

#define DT_FLEXGEN_CLK_MAX	64
static uint32_t mp2_flexgen[DT_FLEXGEN_CLK_MAX];

#if STM32MP21
#define DT_BUS_CLK_MAX		7
#else /* STM32MP21 */
#define DT_BUS_CLK_MAX		6
#endif /* STM32MP21 */
static uint32_t mp2_busclk[DT_BUS_CLK_MAX];

#define DT_KERNEL_CLK_MAX	20
static uint32_t mp2_kernelclk[DT_KERNEL_CLK_MAX];

static struct stm32_clk_platdata stm32mp2_pdata = {
	.osci = mp2_osci,
	.nosci = NB_OSCILLATOR,
	.pll = mp2_pll,
	.npll = _PLL_NB,
	.flexgen = mp2_flexgen,
	.nflexgen = DT_FLEXGEN_CLK_MAX,
	.busclk	= mp2_busclk,
	.nbusclk = DT_BUS_CLK_MAX,
	.kernelclk = mp2_kernelclk,
	.nkernelclk = DT_KERNEL_CLK_MAX,
};

static uint8_t refcounts_mp2[CK_LAST];

static struct stm32_clk_priv stm32mp2_clock_data = {
	.base		= RCC_BASE,
	.num		= ARRAY_SIZE(stm32mp2_clk),
	.clks		= stm32mp2_clk,
	.parents	= parent_mp2,
	.nb_parents	= ARRAY_SIZE(parent_mp2),
	.gates		= gates_mp2,
	.nb_gates	= ARRAY_SIZE(gates_mp2),
	.div		= dividers_mp2,
	.nb_div		= ARRAY_SIZE(dividers_mp2),
	.osci_data	= stm32mp2_osc_data,
	.nb_osci_data	= ARRAY_SIZE(stm32mp2_osc_data),
	.gate_refcounts	= refcounts_mp2,
	.pdata		= &stm32mp2_pdata,
	.ops_array	= ops_array_mp2,
};

int stm32mp2_clk_init(void)
{
	uintptr_t base = RCC_BASE;
	int ret;

#ifdef IMAGE_BL2
	ret = stm32_clk_parse_fdt(&stm32mp2_pdata);
	if (ret != 0) {
		return ret;
	}
#endif

	ret = clk_stm32_init(&stm32mp2_clock_data, base);
	if (ret != 0) {
		return ret;
	}

#ifdef IMAGE_BL2
	ret = stm32mp2_init_clock_tree();
	if (ret != 0) {
		return ret;
	}

	clk_stm32_enable_critical_clocks();
#endif

	return 0;
}

int stm32mp2_pll1_disable(void)
{
#ifdef IMAGE_BL2
	return -EPERM;
#else
	uintptr_t a35_ss_address = A35SSC_BASE;
	uintptr_t pll_enable_reg = a35_ss_address + A35_SS_PLL_ENABLE;

	stm32mp2_a35_ss_on_hsi();

	mmio_clrbits_32(pll_enable_reg, A35_SS_PLL_ENABLE_PD);

	return 0;
#endif
}
