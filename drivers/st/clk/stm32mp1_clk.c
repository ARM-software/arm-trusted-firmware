/*
 * Copyright (C) 2018-2024, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>

#include <arch.h>
#include <arch_helpers.h>
#include <common/debug.h>
#include <common/fdt_wrappers.h>
#include <drivers/clk.h>
#include <drivers/delay_timer.h>
#include <drivers/st/stm32mp_clkfunc.h>
#include <drivers/st/stm32mp1_clk.h>
#include <drivers/st/stm32mp1_rcc.h>
#include <dt-bindings/clock/stm32mp1-clksrc.h>
#include <lib/mmio.h>
#include <lib/spinlock.h>
#include <lib/utils_def.h>
#include <libfdt.h>
#include <plat/common/platform.h>

#include <platform_def.h>

#define MAX_HSI_HZ		64000000
#define USB_PHY_48_MHZ		48000000

#define TIMEOUT_US_200MS	U(200000)
#define TIMEOUT_US_1S		U(1000000)

#define PLLRDY_TIMEOUT		TIMEOUT_US_200MS
#define CLKSRC_TIMEOUT		TIMEOUT_US_200MS
#define CLKDIV_TIMEOUT		TIMEOUT_US_200MS
#define HSIDIV_TIMEOUT		TIMEOUT_US_200MS
#define OSCRDY_TIMEOUT		TIMEOUT_US_1S

const char *stm32mp_osc_node_label[NB_OSC] = {
	[_LSI] = "clk-lsi",
	[_LSE] = "clk-lse",
	[_HSI] = "clk-hsi",
	[_HSE] = "clk-hse",
	[_CSI] = "clk-csi",
	[_I2S_CKIN] = "i2s_ckin",
};

enum stm32mp1_parent_id {
/* Oscillators are defined in enum stm32mp_osc_id */

/* Other parent source */
	_HSI_KER = NB_OSC,
	_HSE_KER,
	_HSE_KER_DIV2,
	_HSE_RTC,
	_CSI_KER,
	_PLL1_P,
	_PLL1_Q,
	_PLL1_R,
	_PLL2_P,
	_PLL2_Q,
	_PLL2_R,
	_PLL3_P,
	_PLL3_Q,
	_PLL3_R,
	_PLL4_P,
	_PLL4_Q,
	_PLL4_R,
	_ACLK,
	_PCLK1,
	_PCLK2,
	_PCLK3,
	_PCLK4,
	_PCLK5,
	_HCLK6,
	_HCLK2,
	_CK_PER,
	_CK_MPU,
	_CK_MCU,
	_USB_PHY_48,
	_PARENT_NB,
	_UNKNOWN_ID = 0xff,
};

/* Lists only the parent clock we are interested in */
enum stm32mp1_parent_sel {
	_I2C12_SEL,
	_I2C35_SEL,
	_STGEN_SEL,
	_I2C46_SEL,
	_SPI6_SEL,
	_UART1_SEL,
	_RNG1_SEL,
	_UART6_SEL,
	_UART24_SEL,
	_UART35_SEL,
	_UART78_SEL,
	_SDMMC12_SEL,
	_SDMMC3_SEL,
	_QSPI_SEL,
	_FMC_SEL,
	_AXIS_SEL,
	_MCUS_SEL,
	_USBPHY_SEL,
	_USBO_SEL,
	_MPU_SEL,
	_CKPER_SEL,
	_RTC_SEL,
	_PARENT_SEL_NB,
	_UNKNOWN_SEL = 0xff,
};

/* State the parent clock ID straight related to a clock */
static const uint8_t parent_id_clock_id[_PARENT_NB] = {
	[_HSE] = CK_HSE,
	[_HSI] = CK_HSI,
	[_CSI] = CK_CSI,
	[_LSE] = CK_LSE,
	[_LSI] = CK_LSI,
	[_I2S_CKIN] = _UNKNOWN_ID,
	[_USB_PHY_48] = _UNKNOWN_ID,
	[_HSI_KER] = CK_HSI,
	[_HSE_KER] = CK_HSE,
	[_HSE_KER_DIV2] = CK_HSE_DIV2,
	[_HSE_RTC] = _UNKNOWN_ID,
	[_CSI_KER] = CK_CSI,
	[_PLL1_P] = PLL1_P,
	[_PLL1_Q] = PLL1_Q,
	[_PLL1_R] = PLL1_R,
	[_PLL2_P] = PLL2_P,
	[_PLL2_Q] = PLL2_Q,
	[_PLL2_R] = PLL2_R,
	[_PLL3_P] = PLL3_P,
	[_PLL3_Q] = PLL3_Q,
	[_PLL3_R] = PLL3_R,
	[_PLL4_P] = PLL4_P,
	[_PLL4_Q] = PLL4_Q,
	[_PLL4_R] = PLL4_R,
	[_ACLK] = CK_AXI,
	[_PCLK1] = CK_AXI,
	[_PCLK2] = CK_AXI,
	[_PCLK3] = CK_AXI,
	[_PCLK4] = CK_AXI,
	[_PCLK5] = CK_AXI,
	[_CK_PER] = CK_PER,
	[_CK_MPU] = CK_MPU,
	[_CK_MCU] = CK_MCU,
};

static unsigned int clock_id2parent_id(unsigned long id)
{
	unsigned int n;

	for (n = 0U; n < ARRAY_SIZE(parent_id_clock_id); n++) {
		if (parent_id_clock_id[n] == id) {
			return n;
		}
	}

	return _UNKNOWN_ID;
}

enum stm32mp1_pll_id {
	_PLL1,
	_PLL2,
	_PLL3,
	_PLL4,
	_PLL_NB
};

enum stm32mp1_div_id {
	_DIV_P,
	_DIV_Q,
	_DIV_R,
	_DIV_NB,
};

enum stm32mp1_clksrc_id {
	CLKSRC_MPU,
	CLKSRC_AXI,
	CLKSRC_MCU,
	CLKSRC_PLL12,
	CLKSRC_PLL3,
	CLKSRC_PLL4,
	CLKSRC_RTC,
	CLKSRC_MCO1,
	CLKSRC_MCO2,
	CLKSRC_NB
};

enum stm32mp1_clkdiv_id {
	CLKDIV_MPU,
	CLKDIV_AXI,
	CLKDIV_MCU,
	CLKDIV_APB1,
	CLKDIV_APB2,
	CLKDIV_APB3,
	CLKDIV_APB4,
	CLKDIV_APB5,
	CLKDIV_RTC,
	CLKDIV_MCO1,
	CLKDIV_MCO2,
	CLKDIV_NB
};

enum stm32mp1_pllcfg {
	PLLCFG_M,
	PLLCFG_N,
	PLLCFG_P,
	PLLCFG_Q,
	PLLCFG_R,
	PLLCFG_O,
	PLLCFG_NB
};

enum stm32mp1_pllcsg {
	PLLCSG_MOD_PER,
	PLLCSG_INC_STEP,
	PLLCSG_SSCG_MODE,
	PLLCSG_NB
};

enum stm32mp1_plltype {
	PLL_800,
	PLL_1600,
	PLL_TYPE_NB
};

struct stm32mp1_pll {
	uint8_t refclk_min;
	uint8_t refclk_max;
};

struct stm32mp1_clk_gate {
	uint16_t offset;
	uint8_t bit;
	uint8_t index;
	uint8_t set_clr;
	uint8_t secure;
	uint8_t sel; /* Relates to enum stm32mp1_parent_sel */
	uint8_t fixed; /* Relates to enum stm32mp1_parent_id */
};

struct stm32mp1_clk_sel {
	uint16_t offset;
	uint8_t src;
	uint8_t msk;
	uint8_t nb_parent;
	const uint8_t *parent;
};

#define REFCLK_SIZE 4
struct stm32mp1_clk_pll {
	enum stm32mp1_plltype plltype;
	uint16_t rckxselr;
	uint16_t pllxcfgr1;
	uint16_t pllxcfgr2;
	uint16_t pllxfracr;
	uint16_t pllxcr;
	uint16_t pllxcsgr;
	enum stm32mp_osc_id refclk[REFCLK_SIZE];
};

/* Clocks with selectable source and non set/clr register access */
#define _CLK_SELEC(sec, off, b, idx, s)			\
	{						\
		.offset = (off),			\
		.bit = (b),				\
		.index = (idx),				\
		.set_clr = 0,				\
		.secure = (sec),			\
		.sel = (s),				\
		.fixed = _UNKNOWN_ID,			\
	}

/* Clocks with fixed source and non set/clr register access */
#define _CLK_FIXED(sec, off, b, idx, f)			\
	{						\
		.offset = (off),			\
		.bit = (b),				\
		.index = (idx),				\
		.set_clr = 0,				\
		.secure = (sec),			\
		.sel = _UNKNOWN_SEL,			\
		.fixed = (f),				\
	}

/* Clocks with selectable source and set/clr register access */
#define _CLK_SC_SELEC(sec, off, b, idx, s)			\
	{						\
		.offset = (off),			\
		.bit = (b),				\
		.index = (idx),				\
		.set_clr = 1,				\
		.secure = (sec),			\
		.sel = (s),				\
		.fixed = _UNKNOWN_ID,			\
	}

/* Clocks with fixed source and set/clr register access */
#define _CLK_SC_FIXED(sec, off, b, idx, f)			\
	{						\
		.offset = (off),			\
		.bit = (b),				\
		.index = (idx),				\
		.set_clr = 1,				\
		.secure = (sec),			\
		.sel = _UNKNOWN_SEL,			\
		.fixed = (f),				\
	}

#define _CLK_PARENT_SEL(_label, _rcc_selr, _parents)		\
	[_ ## _label ## _SEL] = {				\
		.offset = _rcc_selr,				\
		.src = _rcc_selr ## _ ## _label ## SRC_SHIFT,	\
		.msk = (_rcc_selr ## _ ## _label ## SRC_MASK) >> \
		       (_rcc_selr ## _ ## _label ## SRC_SHIFT), \
		.parent = (_parents),				\
		.nb_parent = ARRAY_SIZE(_parents)		\
	}

#define _CLK_PLL(idx, type, off1, off2, off3,		\
		 off4, off5, off6,			\
		 p1, p2, p3, p4)			\
	[(idx)] = {					\
		.plltype = (type),			\
		.rckxselr = (off1),			\
		.pllxcfgr1 = (off2),			\
		.pllxcfgr2 = (off3),			\
		.pllxfracr = (off4),			\
		.pllxcr = (off5),			\
		.pllxcsgr = (off6),			\
		.refclk[0] = (p1),			\
		.refclk[1] = (p2),			\
		.refclk[2] = (p3),			\
		.refclk[3] = (p4),			\
	}

#define NB_GATES	ARRAY_SIZE(stm32mp1_clk_gate)

#define SEC		1
#define N_S		0

static const struct stm32mp1_clk_gate stm32mp1_clk_gate[] = {
	_CLK_FIXED(SEC, RCC_DDRITFCR, 0, DDRC1, _ACLK),
	_CLK_FIXED(SEC, RCC_DDRITFCR, 1, DDRC1LP, _ACLK),
	_CLK_FIXED(SEC, RCC_DDRITFCR, 2, DDRC2, _ACLK),
	_CLK_FIXED(SEC, RCC_DDRITFCR, 3, DDRC2LP, _ACLK),
	_CLK_FIXED(SEC, RCC_DDRITFCR, 4, DDRPHYC, _PLL2_R),
	_CLK_FIXED(SEC, RCC_DDRITFCR, 5, DDRPHYCLP, _PLL2_R),
	_CLK_FIXED(SEC, RCC_DDRITFCR, 6, DDRCAPB, _PCLK4),
	_CLK_FIXED(SEC, RCC_DDRITFCR, 7, DDRCAPBLP, _PCLK4),
	_CLK_FIXED(SEC, RCC_DDRITFCR, 8, AXIDCG, _ACLK),
	_CLK_FIXED(SEC, RCC_DDRITFCR, 9, DDRPHYCAPB, _PCLK4),
	_CLK_FIXED(SEC, RCC_DDRITFCR, 10, DDRPHYCAPBLP, _PCLK4),

#if defined(IMAGE_BL32)
	_CLK_SC_FIXED(N_S, RCC_MP_APB1ENSETR, 6, TIM12_K, _PCLK1),
#endif
	_CLK_SC_SELEC(N_S, RCC_MP_APB1ENSETR, 14, USART2_K, _UART24_SEL),
	_CLK_SC_SELEC(N_S, RCC_MP_APB1ENSETR, 15, USART3_K, _UART35_SEL),
	_CLK_SC_SELEC(N_S, RCC_MP_APB1ENSETR, 16, UART4_K, _UART24_SEL),
	_CLK_SC_SELEC(N_S, RCC_MP_APB1ENSETR, 17, UART5_K, _UART35_SEL),
	_CLK_SC_SELEC(N_S, RCC_MP_APB1ENSETR, 18, UART7_K, _UART78_SEL),
	_CLK_SC_SELEC(N_S, RCC_MP_APB1ENSETR, 19, UART8_K, _UART78_SEL),
	_CLK_SC_SELEC(N_S, RCC_MP_APB1ENSETR, 21, I2C1_K, _I2C12_SEL),
	_CLK_SC_SELEC(N_S, RCC_MP_APB1ENSETR, 22, I2C2_K, _I2C12_SEL),
	_CLK_SC_SELEC(N_S, RCC_MP_APB1ENSETR, 23, I2C3_K, _I2C35_SEL),
	_CLK_SC_SELEC(N_S, RCC_MP_APB1ENSETR, 24, I2C5_K, _I2C35_SEL),

#if defined(IMAGE_BL32)
	_CLK_SC_FIXED(N_S, RCC_MP_APB2ENSETR, 2, TIM15_K, _PCLK2),
#endif
	_CLK_SC_SELEC(N_S, RCC_MP_APB2ENSETR, 13, USART6_K, _UART6_SEL),

	_CLK_SC_FIXED(N_S, RCC_MP_APB3ENSETR, 11, SYSCFG, _UNKNOWN_ID),

	_CLK_SC_SELEC(N_S, RCC_MP_APB4ENSETR, 8, DDRPERFM, _UNKNOWN_SEL),
	_CLK_SC_SELEC(N_S, RCC_MP_APB4ENSETR, 15, IWDG2, _UNKNOWN_SEL),
	_CLK_SC_SELEC(N_S, RCC_MP_APB4ENSETR, 16, USBPHY_K, _USBPHY_SEL),

	_CLK_SC_SELEC(SEC, RCC_MP_APB5ENSETR, 0, SPI6_K, _SPI6_SEL),
	_CLK_SC_SELEC(SEC, RCC_MP_APB5ENSETR, 2, I2C4_K, _I2C46_SEL),
	_CLK_SC_SELEC(SEC, RCC_MP_APB5ENSETR, 3, I2C6_K, _I2C46_SEL),
	_CLK_SC_SELEC(SEC, RCC_MP_APB5ENSETR, 4, USART1_K, _UART1_SEL),
	_CLK_SC_FIXED(SEC, RCC_MP_APB5ENSETR, 8, RTCAPB, _PCLK5),
	_CLK_SC_FIXED(SEC, RCC_MP_APB5ENSETR, 11, TZC1, _PCLK5),
	_CLK_SC_FIXED(SEC, RCC_MP_APB5ENSETR, 12, TZC2, _PCLK5),
	_CLK_SC_FIXED(SEC, RCC_MP_APB5ENSETR, 13, TZPC, _PCLK5),
	_CLK_SC_FIXED(SEC, RCC_MP_APB5ENSETR, 15, IWDG1, _PCLK5),
	_CLK_SC_FIXED(SEC, RCC_MP_APB5ENSETR, 16, BSEC, _PCLK5),
	_CLK_SC_SELEC(SEC, RCC_MP_APB5ENSETR, 20, STGEN_K, _STGEN_SEL),

#if defined(IMAGE_BL32)
	_CLK_SC_SELEC(N_S, RCC_MP_AHB2ENSETR, 8, USBO_K, _USBO_SEL),
	_CLK_SC_SELEC(N_S, RCC_MP_AHB2ENSETR, 16, SDMMC3_K, _SDMMC3_SEL),
#endif

	_CLK_SC_SELEC(N_S, RCC_MP_AHB4ENSETR, 0, GPIOA, _UNKNOWN_SEL),
	_CLK_SC_SELEC(N_S, RCC_MP_AHB4ENSETR, 1, GPIOB, _UNKNOWN_SEL),
	_CLK_SC_SELEC(N_S, RCC_MP_AHB4ENSETR, 2, GPIOC, _UNKNOWN_SEL),
	_CLK_SC_SELEC(N_S, RCC_MP_AHB4ENSETR, 3, GPIOD, _UNKNOWN_SEL),
	_CLK_SC_SELEC(N_S, RCC_MP_AHB4ENSETR, 4, GPIOE, _UNKNOWN_SEL),
	_CLK_SC_SELEC(N_S, RCC_MP_AHB4ENSETR, 5, GPIOF, _UNKNOWN_SEL),
	_CLK_SC_SELEC(N_S, RCC_MP_AHB4ENSETR, 6, GPIOG, _UNKNOWN_SEL),
	_CLK_SC_SELEC(N_S, RCC_MP_AHB4ENSETR, 7, GPIOH, _UNKNOWN_SEL),
	_CLK_SC_SELEC(N_S, RCC_MP_AHB4ENSETR, 8, GPIOI, _UNKNOWN_SEL),
	_CLK_SC_SELEC(N_S, RCC_MP_AHB4ENSETR, 9, GPIOJ, _UNKNOWN_SEL),
	_CLK_SC_SELEC(N_S, RCC_MP_AHB4ENSETR, 10, GPIOK, _UNKNOWN_SEL),

	_CLK_SC_FIXED(SEC, RCC_MP_AHB5ENSETR, 0, GPIOZ, _PCLK5),
	_CLK_SC_FIXED(SEC, RCC_MP_AHB5ENSETR, 4, CRYP1, _PCLK5),
	_CLK_SC_FIXED(SEC, RCC_MP_AHB5ENSETR, 5, HASH1, _PCLK5),
	_CLK_SC_SELEC(SEC, RCC_MP_AHB5ENSETR, 6, RNG1_K, _RNG1_SEL),
	_CLK_SC_FIXED(SEC, RCC_MP_AHB5ENSETR, 8, BKPSRAM, _PCLK5),

#if defined(IMAGE_BL2)
	_CLK_SC_SELEC(N_S, RCC_MP_AHB6ENSETR, 12, FMC_K, _FMC_SEL),
	_CLK_SC_SELEC(N_S, RCC_MP_AHB6ENSETR, 14, QSPI_K, _QSPI_SEL),
#endif
	_CLK_SC_SELEC(N_S, RCC_MP_AHB6ENSETR, 16, SDMMC1_K, _SDMMC12_SEL),
	_CLK_SC_SELEC(N_S, RCC_MP_AHB6ENSETR, 17, SDMMC2_K, _SDMMC12_SEL),
#if defined(IMAGE_BL32)
	_CLK_SC_SELEC(N_S, RCC_MP_AHB6ENSETR, 24, USBH, _UNKNOWN_SEL),
#endif

	_CLK_SELEC(SEC, RCC_BDCR, 20, RTC, _RTC_SEL),
	_CLK_SELEC(N_S, RCC_DBGCFGR, 8, CK_DBG, _UNKNOWN_SEL),
};

static const uint8_t i2c12_parents[] = {
	_PCLK1, _PLL4_R, _HSI_KER, _CSI_KER
};

static const uint8_t i2c35_parents[] = {
	_PCLK1, _PLL4_R, _HSI_KER, _CSI_KER
};

static const uint8_t stgen_parents[] = {
	_HSI_KER, _HSE_KER
};

static const uint8_t i2c46_parents[] = {
	_PCLK5, _PLL3_Q, _HSI_KER, _CSI_KER
};

static const uint8_t spi6_parents[] = {
	_PCLK5, _PLL4_Q, _HSI_KER, _CSI_KER, _HSE_KER, _PLL3_Q
};

static const uint8_t usart1_parents[] = {
	_PCLK5, _PLL3_Q, _HSI_KER, _CSI_KER, _PLL4_Q, _HSE_KER
};

static const uint8_t rng1_parents[] = {
	_CSI, _PLL4_R, _LSE, _LSI
};

static const uint8_t uart6_parents[] = {
	_PCLK2, _PLL4_Q, _HSI_KER, _CSI_KER, _HSE_KER
};

static const uint8_t uart234578_parents[] = {
	_PCLK1, _PLL4_Q, _HSI_KER, _CSI_KER, _HSE_KER
};

static const uint8_t sdmmc12_parents[] = {
	_HCLK6, _PLL3_R, _PLL4_P, _HSI_KER
};

static const uint8_t sdmmc3_parents[] = {
	_HCLK2, _PLL3_R, _PLL4_P, _HSI_KER
};

static const uint8_t qspi_parents[] = {
	_ACLK, _PLL3_R, _PLL4_P, _CK_PER
};

static const uint8_t fmc_parents[] = {
	_ACLK, _PLL3_R, _PLL4_P, _CK_PER
};

static const uint8_t axiss_parents[] = {
	_HSI, _HSE, _PLL2_P
};

static const uint8_t mcuss_parents[] = {
	_HSI, _HSE, _CSI, _PLL3_P
};

static const uint8_t usbphy_parents[] = {
	_HSE_KER, _PLL4_R, _HSE_KER_DIV2
};

static const uint8_t usbo_parents[] = {
	_PLL4_R, _USB_PHY_48
};

static const uint8_t mpu_parents[] = {
	_HSI, _HSE, _PLL1_P, _PLL1_P /* specific div */
};

static const uint8_t per_parents[] = {
	_HSI, _HSE, _CSI,
};

static const uint8_t rtc_parents[] = {
	_UNKNOWN_ID, _LSE, _LSI, _HSE_RTC
};

static const struct stm32mp1_clk_sel stm32mp1_clk_sel[_PARENT_SEL_NB] = {
	_CLK_PARENT_SEL(I2C12, RCC_I2C12CKSELR, i2c12_parents),
	_CLK_PARENT_SEL(I2C35, RCC_I2C35CKSELR, i2c35_parents),
	_CLK_PARENT_SEL(STGEN, RCC_STGENCKSELR, stgen_parents),
	_CLK_PARENT_SEL(I2C46, RCC_I2C46CKSELR, i2c46_parents),
	_CLK_PARENT_SEL(SPI6, RCC_SPI6CKSELR, spi6_parents),
	_CLK_PARENT_SEL(UART1, RCC_UART1CKSELR, usart1_parents),
	_CLK_PARENT_SEL(RNG1, RCC_RNG1CKSELR, rng1_parents),
	_CLK_PARENT_SEL(MPU, RCC_MPCKSELR, mpu_parents),
	_CLK_PARENT_SEL(CKPER, RCC_CPERCKSELR, per_parents),
	_CLK_PARENT_SEL(RTC, RCC_BDCR, rtc_parents),
	_CLK_PARENT_SEL(UART6, RCC_UART6CKSELR, uart6_parents),
	_CLK_PARENT_SEL(UART24, RCC_UART24CKSELR, uart234578_parents),
	_CLK_PARENT_SEL(UART35, RCC_UART35CKSELR, uart234578_parents),
	_CLK_PARENT_SEL(UART78, RCC_UART78CKSELR, uart234578_parents),
	_CLK_PARENT_SEL(SDMMC12, RCC_SDMMC12CKSELR, sdmmc12_parents),
	_CLK_PARENT_SEL(SDMMC3, RCC_SDMMC3CKSELR, sdmmc3_parents),
	_CLK_PARENT_SEL(QSPI, RCC_QSPICKSELR, qspi_parents),
	_CLK_PARENT_SEL(FMC, RCC_FMCCKSELR, fmc_parents),
	_CLK_PARENT_SEL(AXIS, RCC_ASSCKSELR, axiss_parents),
	_CLK_PARENT_SEL(MCUS, RCC_MSSCKSELR, mcuss_parents),
	_CLK_PARENT_SEL(USBPHY, RCC_USBCKSELR, usbphy_parents),
	_CLK_PARENT_SEL(USBO, RCC_USBCKSELR, usbo_parents),
};

/* Define characteristic of PLL according type */
#define DIVN_MIN	24
static const struct stm32mp1_pll stm32mp1_pll[PLL_TYPE_NB] = {
	[PLL_800] = {
		.refclk_min = 4,
		.refclk_max = 16,
	},
	[PLL_1600] = {
		.refclk_min = 8,
		.refclk_max = 16,
	},
};

/* PLLNCFGR2 register divider by output */
static const uint8_t pllncfgr2[_DIV_NB] = {
	[_DIV_P] = RCC_PLLNCFGR2_DIVP_SHIFT,
	[_DIV_Q] = RCC_PLLNCFGR2_DIVQ_SHIFT,
	[_DIV_R] = RCC_PLLNCFGR2_DIVR_SHIFT,
};

static const struct stm32mp1_clk_pll stm32mp1_clk_pll[_PLL_NB] = {
	_CLK_PLL(_PLL1, PLL_1600,
		 RCC_RCK12SELR, RCC_PLL1CFGR1, RCC_PLL1CFGR2,
		 RCC_PLL1FRACR, RCC_PLL1CR, RCC_PLL1CSGR,
		 _HSI, _HSE, _UNKNOWN_OSC_ID, _UNKNOWN_OSC_ID),
	_CLK_PLL(_PLL2, PLL_1600,
		 RCC_RCK12SELR, RCC_PLL2CFGR1, RCC_PLL2CFGR2,
		 RCC_PLL2FRACR, RCC_PLL2CR, RCC_PLL2CSGR,
		 _HSI, _HSE, _UNKNOWN_OSC_ID, _UNKNOWN_OSC_ID),
	_CLK_PLL(_PLL3, PLL_800,
		 RCC_RCK3SELR, RCC_PLL3CFGR1, RCC_PLL3CFGR2,
		 RCC_PLL3FRACR, RCC_PLL3CR, RCC_PLL3CSGR,
		 _HSI, _HSE, _CSI, _UNKNOWN_OSC_ID),
	_CLK_PLL(_PLL4, PLL_800,
		 RCC_RCK4SELR, RCC_PLL4CFGR1, RCC_PLL4CFGR2,
		 RCC_PLL4FRACR, RCC_PLL4CR, RCC_PLL4CSGR,
		 _HSI, _HSE, _CSI, _I2S_CKIN),
};

/* Prescaler table lookups for clock computation */
/* div = /1 /2 /4 /8 / 16 /64 /128 /512 */
static const uint8_t stm32mp1_mcu_div[16] = {
	0, 1, 2, 3, 4, 6, 7, 8, 9, 9, 9, 9, 9, 9, 9, 9
};

/* div = /1 /2 /4 /8 /16 : same divider for PMU and APBX */
#define stm32mp1_mpu_div stm32mp1_mpu_apbx_div
#define stm32mp1_apbx_div stm32mp1_mpu_apbx_div
static const uint8_t stm32mp1_mpu_apbx_div[8] = {
	0, 1, 2, 3, 4, 4, 4, 4
};

/* div = /1 /2 /3 /4 */
static const uint8_t stm32mp1_axi_div[8] = {
	1, 2, 3, 4, 4, 4, 4, 4
};

static const char * const stm32mp1_clk_parent_name[_PARENT_NB] __unused = {
	[_HSI] = "HSI",
	[_HSE] = "HSE",
	[_CSI] = "CSI",
	[_LSI] = "LSI",
	[_LSE] = "LSE",
	[_I2S_CKIN] = "I2S_CKIN",
	[_HSI_KER] = "HSI_KER",
	[_HSE_KER] = "HSE_KER",
	[_HSE_KER_DIV2] = "HSE_KER_DIV2",
	[_HSE_RTC] = "HSE_RTC",
	[_CSI_KER] = "CSI_KER",
	[_PLL1_P] = "PLL1_P",
	[_PLL1_Q] = "PLL1_Q",
	[_PLL1_R] = "PLL1_R",
	[_PLL2_P] = "PLL2_P",
	[_PLL2_Q] = "PLL2_Q",
	[_PLL2_R] = "PLL2_R",
	[_PLL3_P] = "PLL3_P",
	[_PLL3_Q] = "PLL3_Q",
	[_PLL3_R] = "PLL3_R",
	[_PLL4_P] = "PLL4_P",
	[_PLL4_Q] = "PLL4_Q",
	[_PLL4_R] = "PLL4_R",
	[_ACLK] = "ACLK",
	[_PCLK1] = "PCLK1",
	[_PCLK2] = "PCLK2",
	[_PCLK3] = "PCLK3",
	[_PCLK4] = "PCLK4",
	[_PCLK5] = "PCLK5",
	[_HCLK6] = "KCLK6",
	[_HCLK2] = "HCLK2",
	[_CK_PER] = "CK_PER",
	[_CK_MPU] = "CK_MPU",
	[_CK_MCU] = "CK_MCU",
	[_USB_PHY_48] = "USB_PHY_48",
};

/* RCC clock device driver private */
static unsigned long stm32mp1_osc[NB_OSC];
static struct spinlock reg_lock;
static unsigned int gate_refcounts[NB_GATES];
static struct spinlock refcount_lock;

static const struct stm32mp1_clk_gate *gate_ref(unsigned int idx)
{
	return &stm32mp1_clk_gate[idx];
}

#if defined(IMAGE_BL32)
static bool gate_is_non_secure(const struct stm32mp1_clk_gate *gate)
{
	return gate->secure == N_S;
}
#endif

static const struct stm32mp1_clk_sel *clk_sel_ref(unsigned int idx)
{
	return &stm32mp1_clk_sel[idx];
}

static const struct stm32mp1_clk_pll *pll_ref(unsigned int idx)
{
	return &stm32mp1_clk_pll[idx];
}

static void stm32mp1_clk_lock(struct spinlock *lock)
{
	if (stm32mp_lock_available()) {
		/* Assume interrupts are masked */
		spin_lock(lock);
	}
}

static void stm32mp1_clk_unlock(struct spinlock *lock)
{
	if (stm32mp_lock_available()) {
		spin_unlock(lock);
	}
}

bool stm32mp1_rcc_is_secure(void)
{
	uintptr_t rcc_base = stm32mp_rcc_base();
	uint32_t mask = RCC_TZCR_TZEN;

	return (mmio_read_32(rcc_base + RCC_TZCR) & mask) == mask;
}

bool stm32mp1_rcc_is_mckprot(void)
{
	uintptr_t rcc_base = stm32mp_rcc_base();
	uint32_t mask = RCC_TZCR_TZEN | RCC_TZCR_MCKPROT;

	return (mmio_read_32(rcc_base + RCC_TZCR) & mask) == mask;
}

void stm32mp1_clk_rcc_regs_lock(void)
{
	stm32mp1_clk_lock(&reg_lock);
}

void stm32mp1_clk_rcc_regs_unlock(void)
{
	stm32mp1_clk_unlock(&reg_lock);
}

static unsigned long stm32mp1_clk_get_fixed(enum stm32mp_osc_id idx)
{
	if (idx >= NB_OSC) {
		return 0;
	}

	return stm32mp1_osc[idx];
}

static int stm32mp1_clk_get_gated_id(unsigned long id)
{
	unsigned int i;

	for (i = 0U; i < NB_GATES; i++) {
		if (gate_ref(i)->index == id) {
			return i;
		}
	}

	ERROR("%s: clk id %lu not found\n", __func__, id);

	return -EINVAL;
}

static enum stm32mp1_parent_sel stm32mp1_clk_get_sel(int i)
{
	return (enum stm32mp1_parent_sel)(gate_ref(i)->sel);
}

static enum stm32mp1_parent_id stm32mp1_clk_get_fixed_parent(int i)
{
	return (enum stm32mp1_parent_id)(gate_ref(i)->fixed);
}

static int stm32mp1_clk_get_parent(unsigned long id)
{
	const struct stm32mp1_clk_sel *sel;
	uint32_t p_sel;
	int i;
	enum stm32mp1_parent_id p;
	enum stm32mp1_parent_sel s;
	uintptr_t rcc_base = stm32mp_rcc_base();

	/* Few non gateable clock have a static parent ID, find them */
	i = (int)clock_id2parent_id(id);
	if (i != _UNKNOWN_ID) {
		return i;
	}

	i = stm32mp1_clk_get_gated_id(id);
	if (i < 0) {
		panic();
	}

	p = stm32mp1_clk_get_fixed_parent(i);
	if (p < _PARENT_NB) {
		return (int)p;
	}

	s = stm32mp1_clk_get_sel(i);
	if (s == _UNKNOWN_SEL) {
		return -EINVAL;
	}
	if (s >= _PARENT_SEL_NB) {
		panic();
	}

	sel = clk_sel_ref(s);
	p_sel = (mmio_read_32(rcc_base + sel->offset) &
		 (sel->msk << sel->src)) >> sel->src;
	if (p_sel < sel->nb_parent) {
		return (int)sel->parent[p_sel];
	}

	return -EINVAL;
}

static unsigned long stm32mp1_pll_get_fref(const struct stm32mp1_clk_pll *pll)
{
	uint32_t selr = mmio_read_32(stm32mp_rcc_base() + pll->rckxselr);
	uint32_t src = selr & RCC_SELR_REFCLK_SRC_MASK;

	return stm32mp1_clk_get_fixed(pll->refclk[src]);
}

/*
 * pll_get_fvco() : return the VCO or (VCO / 2) frequency for the requested PLL
 * - PLL1 & PLL2 => return VCO / 2 with Fpll_y_ck = FVCO / 2 * (DIVy + 1)
 * - PLL3 & PLL4 => return VCO     with Fpll_y_ck = FVCO / (DIVy + 1)
 * => in all cases Fpll_y_ck = pll_get_fvco() / (DIVy + 1)
 */
static unsigned long stm32mp1_pll_get_fvco(const struct stm32mp1_clk_pll *pll)
{
	unsigned long refclk, fvco;
	uint32_t cfgr1, fracr, divm, divn;
	uintptr_t rcc_base = stm32mp_rcc_base();

	cfgr1 = mmio_read_32(rcc_base + pll->pllxcfgr1);
	fracr = mmio_read_32(rcc_base + pll->pllxfracr);

	divm = (cfgr1 & (RCC_PLLNCFGR1_DIVM_MASK)) >> RCC_PLLNCFGR1_DIVM_SHIFT;
	divn = cfgr1 & RCC_PLLNCFGR1_DIVN_MASK;

	refclk = stm32mp1_pll_get_fref(pll);

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
		numerator = refclk * numerator;
		denominator = ((unsigned long long)divm + 1U) << 13;
		fvco = (unsigned long)(numerator / denominator);
	} else {
		fvco = (unsigned long)(refclk * (divn + 1U) / (divm + 1U));
	}

	return fvco;
}

static unsigned long stm32mp1_read_pll_freq(enum stm32mp1_pll_id pll_id,
					    enum stm32mp1_div_id div_id)
{
	const struct stm32mp1_clk_pll *pll = pll_ref(pll_id);
	unsigned long dfout;
	uint32_t cfgr2, divy;

	if (div_id >= _DIV_NB) {
		return 0;
	}

	cfgr2 = mmio_read_32(stm32mp_rcc_base() + pll->pllxcfgr2);
	divy = (cfgr2 >> pllncfgr2[div_id]) & RCC_PLLNCFGR2_DIVX_MASK;

	dfout = stm32mp1_pll_get_fvco(pll) / (divy + 1U);

	return dfout;
}

static unsigned long get_clock_rate(int p)
{
	uint32_t reg, clkdiv;
	unsigned long clock = 0;
	uintptr_t rcc_base = stm32mp_rcc_base();

	switch (p) {
	case _CK_MPU:
	/* MPU sub system */
		reg = mmio_read_32(rcc_base + RCC_MPCKSELR);
		switch (reg & RCC_SELR_SRC_MASK) {
		case RCC_MPCKSELR_HSI:
			clock = stm32mp1_clk_get_fixed(_HSI);
			break;
		case RCC_MPCKSELR_HSE:
			clock = stm32mp1_clk_get_fixed(_HSE);
			break;
		case RCC_MPCKSELR_PLL:
			clock = stm32mp1_read_pll_freq(_PLL1, _DIV_P);
			break;
		case RCC_MPCKSELR_PLL_MPUDIV:
			clock = stm32mp1_read_pll_freq(_PLL1, _DIV_P);

			reg = mmio_read_32(rcc_base + RCC_MPCKDIVR);
			clkdiv = reg & RCC_MPUDIV_MASK;
			clock >>= stm32mp1_mpu_div[clkdiv];
			break;
		default:
			break;
		}
		break;
	/* AXI sub system */
	case _ACLK:
	case _HCLK2:
	case _HCLK6:
	case _PCLK4:
	case _PCLK5:
		reg = mmio_read_32(rcc_base + RCC_ASSCKSELR);
		switch (reg & RCC_SELR_SRC_MASK) {
		case RCC_ASSCKSELR_HSI:
			clock = stm32mp1_clk_get_fixed(_HSI);
			break;
		case RCC_ASSCKSELR_HSE:
			clock = stm32mp1_clk_get_fixed(_HSE);
			break;
		case RCC_ASSCKSELR_PLL:
			clock = stm32mp1_read_pll_freq(_PLL2, _DIV_P);
			break;
		default:
			break;
		}

		/* System clock divider */
		reg = mmio_read_32(rcc_base + RCC_AXIDIVR);
		clock /= stm32mp1_axi_div[reg & RCC_AXIDIV_MASK];

		switch (p) {
		case _PCLK4:
			reg = mmio_read_32(rcc_base + RCC_APB4DIVR);
			clock >>= stm32mp1_apbx_div[reg & RCC_APBXDIV_MASK];
			break;
		case _PCLK5:
			reg = mmio_read_32(rcc_base + RCC_APB5DIVR);
			clock >>= stm32mp1_apbx_div[reg & RCC_APBXDIV_MASK];
			break;
		default:
			break;
		}
		break;
	/* MCU sub system */
	case _CK_MCU:
	case _PCLK1:
	case _PCLK2:
	case _PCLK3:
		reg = mmio_read_32(rcc_base + RCC_MSSCKSELR);
		switch (reg & RCC_SELR_SRC_MASK) {
		case RCC_MSSCKSELR_HSI:
			clock = stm32mp1_clk_get_fixed(_HSI);
			break;
		case RCC_MSSCKSELR_HSE:
			clock = stm32mp1_clk_get_fixed(_HSE);
			break;
		case RCC_MSSCKSELR_CSI:
			clock = stm32mp1_clk_get_fixed(_CSI);
			break;
		case RCC_MSSCKSELR_PLL:
			clock = stm32mp1_read_pll_freq(_PLL3, _DIV_P);
			break;
		default:
			break;
		}

		/* MCU clock divider */
		reg = mmio_read_32(rcc_base + RCC_MCUDIVR);
		clock >>= stm32mp1_mcu_div[reg & RCC_MCUDIV_MASK];

		switch (p) {
		case _PCLK1:
			reg = mmio_read_32(rcc_base + RCC_APB1DIVR);
			clock >>= stm32mp1_apbx_div[reg & RCC_APBXDIV_MASK];
			break;
		case _PCLK2:
			reg = mmio_read_32(rcc_base + RCC_APB2DIVR);
			clock >>= stm32mp1_apbx_div[reg & RCC_APBXDIV_MASK];
			break;
		case _PCLK3:
			reg = mmio_read_32(rcc_base + RCC_APB3DIVR);
			clock >>= stm32mp1_apbx_div[reg & RCC_APBXDIV_MASK];
			break;
		case _CK_MCU:
		default:
			break;
		}
		break;
	case _CK_PER:
		reg = mmio_read_32(rcc_base + RCC_CPERCKSELR);
		switch (reg & RCC_SELR_SRC_MASK) {
		case RCC_CPERCKSELR_HSI:
			clock = stm32mp1_clk_get_fixed(_HSI);
			break;
		case RCC_CPERCKSELR_HSE:
			clock = stm32mp1_clk_get_fixed(_HSE);
			break;
		case RCC_CPERCKSELR_CSI:
			clock = stm32mp1_clk_get_fixed(_CSI);
			break;
		default:
			break;
		}
		break;
	case _HSI:
	case _HSI_KER:
		clock = stm32mp1_clk_get_fixed(_HSI);
		break;
	case _CSI:
	case _CSI_KER:
		clock = stm32mp1_clk_get_fixed(_CSI);
		break;
	case _HSE:
	case _HSE_KER:
		clock = stm32mp1_clk_get_fixed(_HSE);
		break;
	case _HSE_KER_DIV2:
		clock = stm32mp1_clk_get_fixed(_HSE) >> 1;
		break;
	case _HSE_RTC:
		clock = stm32mp1_clk_get_fixed(_HSE);
		clock /= (mmio_read_32(rcc_base + RCC_RTCDIVR) & RCC_DIVR_DIV_MASK) + 1U;
		break;
	case _LSI:
		clock = stm32mp1_clk_get_fixed(_LSI);
		break;
	case _LSE:
		clock = stm32mp1_clk_get_fixed(_LSE);
		break;
	/* PLL */
	case _PLL1_P:
		clock = stm32mp1_read_pll_freq(_PLL1, _DIV_P);
		break;
	case _PLL1_Q:
		clock = stm32mp1_read_pll_freq(_PLL1, _DIV_Q);
		break;
	case _PLL1_R:
		clock = stm32mp1_read_pll_freq(_PLL1, _DIV_R);
		break;
	case _PLL2_P:
		clock = stm32mp1_read_pll_freq(_PLL2, _DIV_P);
		break;
	case _PLL2_Q:
		clock = stm32mp1_read_pll_freq(_PLL2, _DIV_Q);
		break;
	case _PLL2_R:
		clock = stm32mp1_read_pll_freq(_PLL2, _DIV_R);
		break;
	case _PLL3_P:
		clock = stm32mp1_read_pll_freq(_PLL3, _DIV_P);
		break;
	case _PLL3_Q:
		clock = stm32mp1_read_pll_freq(_PLL3, _DIV_Q);
		break;
	case _PLL3_R:
		clock = stm32mp1_read_pll_freq(_PLL3, _DIV_R);
		break;
	case _PLL4_P:
		clock = stm32mp1_read_pll_freq(_PLL4, _DIV_P);
		break;
	case _PLL4_Q:
		clock = stm32mp1_read_pll_freq(_PLL4, _DIV_Q);
		break;
	case _PLL4_R:
		clock = stm32mp1_read_pll_freq(_PLL4, _DIV_R);
		break;
	/* Other */
	case _USB_PHY_48:
		clock = USB_PHY_48_MHZ;
		break;
	default:
		break;
	}

	return clock;
}

static void __clk_enable(struct stm32mp1_clk_gate const *gate)
{
	uintptr_t rcc_base = stm32mp_rcc_base();

	VERBOSE("Enable clock %u\n", gate->index);

	if (gate->set_clr != 0U) {
		mmio_write_32(rcc_base + gate->offset, BIT(gate->bit));
	} else {
		mmio_setbits_32(rcc_base + gate->offset, BIT(gate->bit));
	}
}

static void __clk_disable(struct stm32mp1_clk_gate const *gate)
{
	uintptr_t rcc_base = stm32mp_rcc_base();

	VERBOSE("Disable clock %u\n", gate->index);

	if (gate->set_clr != 0U) {
		mmio_write_32(rcc_base + gate->offset + RCC_MP_ENCLRR_OFFSET,
			      BIT(gate->bit));
	} else {
		mmio_clrbits_32(rcc_base + gate->offset, BIT(gate->bit));
	}
}

static bool __clk_is_enabled(struct stm32mp1_clk_gate const *gate)
{
	uintptr_t rcc_base = stm32mp_rcc_base();

	return mmio_read_32(rcc_base + gate->offset) & BIT(gate->bit);
}

/* Oscillators and PLLs are not gated at runtime */
static bool clock_is_always_on(unsigned long id)
{
	switch (id) {
	case CK_HSE:
	case CK_CSI:
	case CK_LSI:
	case CK_LSE:
	case CK_HSI:
	case CK_HSE_DIV2:
	case PLL1_Q:
	case PLL1_R:
	case PLL2_P:
	case PLL2_Q:
	case PLL2_R:
	case PLL3_P:
	case PLL3_Q:
	case PLL3_R:
	case CK_AXI:
	case CK_MPU:
	case CK_MCU:
	case RTC:
		return true;
	default:
		return false;
	}
}

static void __stm32mp1_clk_enable(unsigned long id, bool with_refcnt)
{
	const struct stm32mp1_clk_gate *gate;
	int i;

	if (clock_is_always_on(id)) {
		return;
	}

	i = stm32mp1_clk_get_gated_id(id);
	if (i < 0) {
		ERROR("Clock %lu can't be enabled\n", id);
		panic();
	}

	gate = gate_ref(i);

	if (!with_refcnt) {
		__clk_enable(gate);
		return;
	}

#if defined(IMAGE_BL32)
	if (gate_is_non_secure(gate)) {
		/* Enable non-secure clock w/o any refcounting */
		__clk_enable(gate);
		return;
	}
#endif

	stm32mp1_clk_lock(&refcount_lock);

	if (gate_refcounts[i] == 0U) {
		__clk_enable(gate);
	}

	gate_refcounts[i]++;
	if (gate_refcounts[i] == UINT_MAX) {
		ERROR("Clock %lu refcount reached max value\n", id);
		panic();
	}

	stm32mp1_clk_unlock(&refcount_lock);
}

static void __stm32mp1_clk_disable(unsigned long id, bool with_refcnt)
{
	const struct stm32mp1_clk_gate *gate;
	int i;

	if (clock_is_always_on(id)) {
		return;
	}

	i = stm32mp1_clk_get_gated_id(id);
	if (i < 0) {
		ERROR("Clock %lu can't be disabled\n", id);
		panic();
	}

	gate = gate_ref(i);

	if (!with_refcnt) {
		__clk_disable(gate);
		return;
	}

#if defined(IMAGE_BL32)
	if (gate_is_non_secure(gate)) {
		/* Don't disable non-secure clocks */
		return;
	}
#endif

	stm32mp1_clk_lock(&refcount_lock);

	if (gate_refcounts[i] == 0U) {
		ERROR("Clock %lu refcount reached 0\n", id);
		panic();
	}
	gate_refcounts[i]--;

	if (gate_refcounts[i] == 0U) {
		__clk_disable(gate);
	}

	stm32mp1_clk_unlock(&refcount_lock);
}

static int stm32mp_clk_enable(unsigned long id)
{
	__stm32mp1_clk_enable(id, true);

	return 0;
}

static void stm32mp_clk_disable(unsigned long id)
{
	__stm32mp1_clk_disable(id, true);
}

static bool stm32mp_clk_is_enabled(unsigned long id)
{
	int i;

	if (clock_is_always_on(id)) {
		return true;
	}

	i = stm32mp1_clk_get_gated_id(id);
	if (i < 0) {
		panic();
	}

	return __clk_is_enabled(gate_ref(i));
}

static unsigned long stm32mp_clk_get_rate(unsigned long id)
{
	uintptr_t rcc_base = stm32mp_rcc_base();
	int p = stm32mp1_clk_get_parent(id);
	uint32_t prescaler, timpre;
	unsigned long parent_rate;

	if (p < 0) {
		return 0;
	}

	parent_rate = get_clock_rate(p);

	switch (id) {
	case TIM2_K:
	case TIM3_K:
	case TIM4_K:
	case TIM5_K:
	case TIM6_K:
	case TIM7_K:
	case TIM12_K:
	case TIM13_K:
	case TIM14_K:
		prescaler = mmio_read_32(rcc_base + RCC_APB1DIVR) &
			    RCC_APBXDIV_MASK;
		timpre = mmio_read_32(rcc_base + RCC_TIMG1PRER) &
			 RCC_TIMGXPRER_TIMGXPRE;
		break;

	case TIM1_K:
	case TIM8_K:
	case TIM15_K:
	case TIM16_K:
	case TIM17_K:
		prescaler = mmio_read_32(rcc_base + RCC_APB2DIVR) &
			    RCC_APBXDIV_MASK;
		timpre = mmio_read_32(rcc_base + RCC_TIMG2PRER) &
			 RCC_TIMGXPRER_TIMGXPRE;
		break;

	default:
		return parent_rate;
	}

	if (prescaler == 0U) {
		return parent_rate;
	}

	return parent_rate * (timpre + 1U) * 2U;
}

static void stm32mp1_ls_osc_set(bool enable, uint32_t offset, uint32_t mask_on)
{
	uintptr_t address = stm32mp_rcc_base() + offset;

	if (enable) {
		mmio_setbits_32(address, mask_on);
	} else {
		mmio_clrbits_32(address, mask_on);
	}
}

static void stm32mp1_hs_ocs_set(bool enable, uint32_t mask_on)
{
	uint32_t offset = enable ? RCC_OCENSETR : RCC_OCENCLRR;
	uintptr_t address = stm32mp_rcc_base() + offset;

	mmio_write_32(address, mask_on);
}

static int stm32mp1_osc_wait(bool enable, uint32_t offset, uint32_t mask_rdy)
{
	uint64_t timeout;
	uint32_t mask_test;
	uintptr_t address = stm32mp_rcc_base() + offset;

	if (enable) {
		mask_test = mask_rdy;
	} else {
		mask_test = 0;
	}

	timeout = timeout_init_us(OSCRDY_TIMEOUT);
	while ((mmio_read_32(address) & mask_rdy) != mask_test) {
		if (timeout_elapsed(timeout)) {
			ERROR("OSC %x @ %lx timeout for enable=%d : 0x%x\n",
			      mask_rdy, address, enable, mmio_read_32(address));
			return -ETIMEDOUT;
		}
	}

	return 0;
}

static void stm32mp1_lse_enable(bool bypass, bool digbyp, uint32_t lsedrv)
{
	uint32_t value;
	uintptr_t rcc_base = stm32mp_rcc_base();

	if (digbyp) {
		mmio_setbits_32(rcc_base + RCC_BDCR, RCC_BDCR_DIGBYP);
	}

	if (bypass || digbyp) {
		mmio_setbits_32(rcc_base + RCC_BDCR, RCC_BDCR_LSEBYP);
	}

	/*
	 * Warning: not recommended to switch directly from "high drive"
	 * to "medium low drive", and vice-versa.
	 */
	value = (mmio_read_32(rcc_base + RCC_BDCR) & RCC_BDCR_LSEDRV_MASK) >>
		RCC_BDCR_LSEDRV_SHIFT;

	while (value != lsedrv) {
		if (value > lsedrv) {
			value--;
		} else {
			value++;
		}

		mmio_clrsetbits_32(rcc_base + RCC_BDCR,
				   RCC_BDCR_LSEDRV_MASK,
				   value << RCC_BDCR_LSEDRV_SHIFT);
	}

	stm32mp1_ls_osc_set(true, RCC_BDCR, RCC_BDCR_LSEON);
}

static void stm32mp1_lse_wait(void)
{
	if (stm32mp1_osc_wait(true, RCC_BDCR, RCC_BDCR_LSERDY) != 0) {
		VERBOSE("%s: failed\n", __func__);
	}
}

static void stm32mp1_lsi_set(bool enable)
{
	stm32mp1_ls_osc_set(enable, RCC_RDLSICR, RCC_RDLSICR_LSION);

	if (stm32mp1_osc_wait(enable, RCC_RDLSICR, RCC_RDLSICR_LSIRDY) != 0) {
		VERBOSE("%s: failed\n", __func__);
	}
}

static void stm32mp1_hse_enable(bool bypass, bool digbyp, bool css)
{
	uintptr_t rcc_base = stm32mp_rcc_base();

	if (digbyp) {
		mmio_write_32(rcc_base + RCC_OCENSETR, RCC_OCENR_DIGBYP);
	}

	if (bypass || digbyp) {
		mmio_write_32(rcc_base + RCC_OCENSETR, RCC_OCENR_HSEBYP);
	}

	stm32mp1_hs_ocs_set(true, RCC_OCENR_HSEON);
	if (stm32mp1_osc_wait(true, RCC_OCRDYR, RCC_OCRDYR_HSERDY) != 0) {
		VERBOSE("%s: failed\n", __func__);
	}

	if (css) {
		mmio_write_32(rcc_base + RCC_OCENSETR, RCC_OCENR_HSECSSON);
	}

#if STM32MP_UART_PROGRAMMER || STM32MP_USB_PROGRAMMER
	if ((mmio_read_32(rcc_base + RCC_OCENSETR) & RCC_OCENR_HSEBYP) &&
	    (!(digbyp || bypass))) {
		panic();
	}
#endif
}

static void stm32mp1_csi_set(bool enable)
{
	stm32mp1_hs_ocs_set(enable, RCC_OCENR_CSION);
	if (stm32mp1_osc_wait(enable, RCC_OCRDYR, RCC_OCRDYR_CSIRDY) != 0) {
		VERBOSE("%s: failed\n", __func__);
	}
}

static void stm32mp1_hsi_set(bool enable)
{
	stm32mp1_hs_ocs_set(enable, RCC_OCENR_HSION);
	if (stm32mp1_osc_wait(enable, RCC_OCRDYR, RCC_OCRDYR_HSIRDY) != 0) {
		VERBOSE("%s: failed\n", __func__);
	}
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
		return -1;
	}

	if (hsidiv != 0U) {
		return stm32mp1_set_hsidiv(hsidiv);
	}

	return 0;
}

static bool stm32mp1_check_pll_conf(enum stm32mp1_pll_id pll_id,
				    unsigned int clksrc,
				    uint32_t *pllcfg, int plloff)
{
	const struct stm32mp1_clk_pll *pll = pll_ref(pll_id);
	uintptr_t rcc_base = stm32mp_rcc_base();
	uintptr_t pllxcr = rcc_base + pll->pllxcr;
	enum stm32mp1_plltype type = pll->plltype;
	uintptr_t clksrc_address = rcc_base + (clksrc >> 4);
	unsigned long refclk;
	uint32_t ifrge = 0U;
	uint32_t src, value, fracv = 0;
	void *fdt;

	/* Check PLL output */
	if (mmio_read_32(pllxcr) != RCC_PLLNCR_PLLON) {
		return false;
	}

	/* Check current clksrc */
	src = mmio_read_32(clksrc_address) & RCC_SELR_SRC_MASK;
	if (src != (clksrc & RCC_SELR_SRC_MASK)) {
		return false;
	}

	/* Check Div */
	src = mmio_read_32(rcc_base + pll->rckxselr) & RCC_SELR_REFCLK_SRC_MASK;

	refclk = stm32mp1_clk_get_fixed(pll->refclk[src]) /
		 (pllcfg[PLLCFG_M] + 1U);

	if ((refclk < (stm32mp1_pll[type].refclk_min * 1000000U)) ||
	    (refclk > (stm32mp1_pll[type].refclk_max * 1000000U))) {
		return false;
	}

	if ((type == PLL_800) && (refclk >= 8000000U)) {
		ifrge = 1U;
	}

	value = (pllcfg[PLLCFG_N] << RCC_PLLNCFGR1_DIVN_SHIFT) &
		RCC_PLLNCFGR1_DIVN_MASK;
	value |= (pllcfg[PLLCFG_M] << RCC_PLLNCFGR1_DIVM_SHIFT) &
		 RCC_PLLNCFGR1_DIVM_MASK;
	value |= (ifrge << RCC_PLLNCFGR1_IFRGE_SHIFT) &
		 RCC_PLLNCFGR1_IFRGE_MASK;
	if (mmio_read_32(rcc_base + pll->pllxcfgr1) != value) {
		return false;
	}

	/* Fractional configuration */
	if (fdt_get_address(&fdt) == 1) {
		fracv = fdt_read_uint32_default(fdt, plloff, "frac", 0);
	}

	value = fracv << RCC_PLLNFRACR_FRACV_SHIFT;
	value |= RCC_PLLNFRACR_FRACLE;
	if (mmio_read_32(rcc_base + pll->pllxfracr) != value) {
		return false;
	}

	/* Output config */
	value = (pllcfg[PLLCFG_P] << RCC_PLLNCFGR2_DIVP_SHIFT) &
		RCC_PLLNCFGR2_DIVP_MASK;
	value |= (pllcfg[PLLCFG_Q] << RCC_PLLNCFGR2_DIVQ_SHIFT) &
		 RCC_PLLNCFGR2_DIVQ_MASK;
	value |= (pllcfg[PLLCFG_R] << RCC_PLLNCFGR2_DIVR_SHIFT) &
		 RCC_PLLNCFGR2_DIVR_MASK;
	if (mmio_read_32(rcc_base + pll->pllxcfgr2) != value) {
		return false;
	}

	return true;
}

static void stm32mp1_pll_start(enum stm32mp1_pll_id pll_id)
{
	const struct stm32mp1_clk_pll *pll = pll_ref(pll_id);
	uintptr_t pllxcr = stm32mp_rcc_base() + pll->pllxcr;

	/* Preserve RCC_PLLNCR_SSCG_CTRL value */
	mmio_clrsetbits_32(pllxcr,
			   RCC_PLLNCR_DIVPEN | RCC_PLLNCR_DIVQEN |
			   RCC_PLLNCR_DIVREN,
			   RCC_PLLNCR_PLLON);
}

static int stm32mp1_pll_output(enum stm32mp1_pll_id pll_id, uint32_t output)
{
	const struct stm32mp1_clk_pll *pll = pll_ref(pll_id);
	uintptr_t pllxcr = stm32mp_rcc_base() + pll->pllxcr;
	uint64_t timeout = timeout_init_us(PLLRDY_TIMEOUT);

	/* Wait PLL lock */
	while ((mmio_read_32(pllxcr) & RCC_PLLNCR_PLLRDY) == 0U) {
		if (timeout_elapsed(timeout)) {
			ERROR("PLL%u start failed @ 0x%lx: 0x%x\n",
			      pll_id, pllxcr, mmio_read_32(pllxcr));
			return -ETIMEDOUT;
		}
	}

	/* Start the requested output */
	mmio_setbits_32(pllxcr, output << RCC_PLLNCR_DIVEN_SHIFT);

	return 0;
}

static int stm32mp1_pll_stop(enum stm32mp1_pll_id pll_id)
{
	const struct stm32mp1_clk_pll *pll = pll_ref(pll_id);
	uintptr_t pllxcr = stm32mp_rcc_base() + pll->pllxcr;
	uint64_t timeout;

	/* Stop all output */
	mmio_clrbits_32(pllxcr, RCC_PLLNCR_DIVPEN | RCC_PLLNCR_DIVQEN |
			RCC_PLLNCR_DIVREN);

	/* Stop PLL */
	mmio_clrbits_32(pllxcr, RCC_PLLNCR_PLLON);

	timeout = timeout_init_us(PLLRDY_TIMEOUT);
	/* Wait PLL stopped */
	while ((mmio_read_32(pllxcr) & RCC_PLLNCR_PLLRDY) != 0U) {
		if (timeout_elapsed(timeout)) {
			ERROR("PLL%u stop failed @ 0x%lx: 0x%x\n",
			      pll_id, pllxcr, mmio_read_32(pllxcr));
			return -ETIMEDOUT;
		}
	}

	return 0;
}

static void stm32mp1_pll_config_output(enum stm32mp1_pll_id pll_id,
				       uint32_t *pllcfg)
{
	const struct stm32mp1_clk_pll *pll = pll_ref(pll_id);
	uintptr_t rcc_base = stm32mp_rcc_base();
	uint32_t value;

	value = (pllcfg[PLLCFG_P] << RCC_PLLNCFGR2_DIVP_SHIFT) &
		RCC_PLLNCFGR2_DIVP_MASK;
	value |= (pllcfg[PLLCFG_Q] << RCC_PLLNCFGR2_DIVQ_SHIFT) &
		 RCC_PLLNCFGR2_DIVQ_MASK;
	value |= (pllcfg[PLLCFG_R] << RCC_PLLNCFGR2_DIVR_SHIFT) &
		 RCC_PLLNCFGR2_DIVR_MASK;
	mmio_write_32(rcc_base + pll->pllxcfgr2, value);
}

static int stm32mp1_pll_config(enum stm32mp1_pll_id pll_id,
			       uint32_t *pllcfg, uint32_t fracv)
{
	const struct stm32mp1_clk_pll *pll = pll_ref(pll_id);
	uintptr_t rcc_base = stm32mp_rcc_base();
	enum stm32mp1_plltype type = pll->plltype;
	unsigned long refclk;
	uint32_t ifrge = 0;
	uint32_t src, value;

	src = mmio_read_32(rcc_base + pll->rckxselr) &
		RCC_SELR_REFCLK_SRC_MASK;

	refclk = stm32mp1_clk_get_fixed(pll->refclk[src]) /
		 (pllcfg[PLLCFG_M] + 1U);

	if ((refclk < (stm32mp1_pll[type].refclk_min * 1000000U)) ||
	    (refclk > (stm32mp1_pll[type].refclk_max * 1000000U))) {
		return -EINVAL;
	}

	if ((type == PLL_800) && (refclk >= 8000000U)) {
		ifrge = 1U;
	}

	value = (pllcfg[PLLCFG_N] << RCC_PLLNCFGR1_DIVN_SHIFT) &
		RCC_PLLNCFGR1_DIVN_MASK;
	value |= (pllcfg[PLLCFG_M] << RCC_PLLNCFGR1_DIVM_SHIFT) &
		 RCC_PLLNCFGR1_DIVM_MASK;
	value |= (ifrge << RCC_PLLNCFGR1_IFRGE_SHIFT) &
		 RCC_PLLNCFGR1_IFRGE_MASK;
	mmio_write_32(rcc_base + pll->pllxcfgr1, value);

	/* Fractional configuration */
	value = 0;
	mmio_write_32(rcc_base + pll->pllxfracr, value);

	value = fracv << RCC_PLLNFRACR_FRACV_SHIFT;
	mmio_write_32(rcc_base + pll->pllxfracr, value);

	value |= RCC_PLLNFRACR_FRACLE;
	mmio_write_32(rcc_base + pll->pllxfracr, value);

	stm32mp1_pll_config_output(pll_id, pllcfg);

	return 0;
}

static void stm32mp1_pll_csg(enum stm32mp1_pll_id pll_id, uint32_t *csg)
{
	const struct stm32mp1_clk_pll *pll = pll_ref(pll_id);
	uint32_t pllxcsg = 0;

	pllxcsg |= (csg[PLLCSG_MOD_PER] << RCC_PLLNCSGR_MOD_PER_SHIFT) &
		    RCC_PLLNCSGR_MOD_PER_MASK;

	pllxcsg |= (csg[PLLCSG_INC_STEP] << RCC_PLLNCSGR_INC_STEP_SHIFT) &
		    RCC_PLLNCSGR_INC_STEP_MASK;

	pllxcsg |= (csg[PLLCSG_SSCG_MODE] << RCC_PLLNCSGR_SSCG_MODE_SHIFT) &
		    RCC_PLLNCSGR_SSCG_MODE_MASK;

	mmio_write_32(stm32mp_rcc_base() + pll->pllxcsgr, pllxcsg);

	mmio_setbits_32(stm32mp_rcc_base() + pll->pllxcr,
			RCC_PLLNCR_SSCG_CTRL);
}

static int stm32mp1_set_clksrc(unsigned int clksrc)
{
	uintptr_t clksrc_address = stm32mp_rcc_base() + (clksrc >> 4);
	uint64_t timeout;

	mmio_clrsetbits_32(clksrc_address, RCC_SELR_SRC_MASK,
			   clksrc & RCC_SELR_SRC_MASK);

	timeout = timeout_init_us(CLKSRC_TIMEOUT);
	while ((mmio_read_32(clksrc_address) & RCC_SELR_SRCRDY) == 0U) {
		if (timeout_elapsed(timeout)) {
			ERROR("CLKSRC %x start failed @ 0x%lx: 0x%x\n", clksrc,
			      clksrc_address, mmio_read_32(clksrc_address));
			return -ETIMEDOUT;
		}
	}

	return 0;
}

static int stm32mp1_set_clkdiv(unsigned int clkdiv, uintptr_t address)
{
	uint64_t timeout;

	mmio_clrsetbits_32(address, RCC_DIVR_DIV_MASK,
			   clkdiv & RCC_DIVR_DIV_MASK);

	timeout = timeout_init_us(CLKDIV_TIMEOUT);
	while ((mmio_read_32(address) & RCC_DIVR_DIVRDY) == 0U) {
		if (timeout_elapsed(timeout)) {
			ERROR("CLKDIV %x start failed @ 0x%lx: 0x%x\n",
			      clkdiv, address, mmio_read_32(address));
			return -ETIMEDOUT;
		}
	}

	return 0;
}

static void stm32mp1_mco_csg(uint32_t clksrc, uint32_t clkdiv)
{
	uintptr_t clksrc_address = stm32mp_rcc_base() + (clksrc >> 4);

	/*
	 * Binding clksrc :
	 *      bit15-4 offset
	 *      bit3:   disable
	 *      bit2-0: MCOSEL[2:0]
	 */
	if ((clksrc & 0x8U) != 0U) {
		mmio_clrbits_32(clksrc_address, RCC_MCOCFG_MCOON);
	} else {
		mmio_clrsetbits_32(clksrc_address,
				   RCC_MCOCFG_MCOSRC_MASK,
				   clksrc & RCC_MCOCFG_MCOSRC_MASK);
		mmio_clrsetbits_32(clksrc_address,
				   RCC_MCOCFG_MCODIV_MASK,
				   clkdiv << RCC_MCOCFG_MCODIV_SHIFT);
		mmio_setbits_32(clksrc_address, RCC_MCOCFG_MCOON);
	}
}

static void stm32mp1_set_rtcsrc(unsigned int clksrc, bool lse_css)
{
	uintptr_t address = stm32mp_rcc_base() + RCC_BDCR;

	if (((mmio_read_32(address) & RCC_BDCR_RTCCKEN) == 0U) ||
	    (clksrc != (uint32_t)CLK_RTC_DISABLED)) {
		mmio_clrsetbits_32(address,
				   RCC_BDCR_RTCSRC_MASK,
				   (clksrc & RCC_SELR_SRC_MASK) << RCC_BDCR_RTCSRC_SHIFT);

		mmio_setbits_32(address, RCC_BDCR_RTCCKEN);
	}

	if (lse_css) {
		mmio_setbits_32(address, RCC_BDCR_LSECSSON);
	}
}

static void stm32mp1_pkcs_config(uint32_t pkcs)
{
	uintptr_t address = stm32mp_rcc_base() + ((pkcs >> 4) & 0xFFFU);
	uint32_t value = pkcs & 0xFU;
	uint32_t mask = 0xFU;

	if ((pkcs & BIT(31)) != 0U) {
		mask <<= 4;
		value <<= 4;
	}

	mmio_clrsetbits_32(address, mask, value);
}

static int clk_get_pll_settings_from_dt(int plloff, unsigned int *pllcfg,
					uint32_t *fracv, uint32_t *csg,
					bool *csg_set)
{
	void *fdt;
	int ret;

	if (fdt_get_address(&fdt) == 0) {
		return -FDT_ERR_NOTFOUND;
	}

	ret = fdt_read_uint32_array(fdt, plloff, "cfg", (uint32_t)PLLCFG_NB,
				    pllcfg);
	if (ret < 0) {
		return -FDT_ERR_NOTFOUND;
	}

	*fracv = fdt_read_uint32_default(fdt, plloff, "frac", 0);

	ret = fdt_read_uint32_array(fdt, plloff, "csg", (uint32_t)PLLCSG_NB,
				    csg);

	*csg_set = (ret == 0);

	if (ret == -FDT_ERR_NOTFOUND) {
		ret = 0;
	}

	return ret;
}

int stm32mp1_clk_init(void)
{
	uintptr_t rcc_base = stm32mp_rcc_base();
	uint32_t pllfracv[_PLL_NB];
	uint32_t pllcsg[_PLL_NB][PLLCSG_NB];
	unsigned int clksrc[CLKSRC_NB];
	unsigned int clkdiv[CLKDIV_NB];
	unsigned int pllcfg[_PLL_NB][PLLCFG_NB];
	int plloff[_PLL_NB];
	int ret, len;
	enum stm32mp1_pll_id i;
	bool pllcsg_set[_PLL_NB];
	bool pllcfg_valid[_PLL_NB];
	bool lse_css = false;
	bool pll3_preserve = false;
	bool pll4_preserve = false;
	bool pll4_bootrom = false;
	const fdt32_t *pkcs_cell;
	void *fdt;
	int stgen_p = stm32mp1_clk_get_parent(STGEN_K);
	int usbphy_p = stm32mp1_clk_get_parent(USBPHY_K);

	if (fdt_get_address(&fdt) == 0) {
		return -FDT_ERR_NOTFOUND;
	}

	ret = fdt_rcc_read_uint32_array("st,clksrc", (uint32_t)CLKSRC_NB,
					clksrc);
	if (ret < 0) {
		return -FDT_ERR_NOTFOUND;
	}

	ret = fdt_rcc_read_uint32_array("st,clkdiv", (uint32_t)CLKDIV_NB,
					clkdiv);
	if (ret < 0) {
		return -FDT_ERR_NOTFOUND;
	}

	for (i = (enum stm32mp1_pll_id)0; i < _PLL_NB; i++) {
		char name[12];

		snprintf(name, sizeof(name), "st,pll@%u", i);
		plloff[i] = fdt_rcc_subnode_offset(name);

		pllcfg_valid[i] = fdt_check_node(plloff[i]);
		if (!pllcfg_valid[i]) {
			continue;
		}

		ret = clk_get_pll_settings_from_dt(plloff[i], pllcfg[i],
						   &pllfracv[i], pllcsg[i],
						   &pllcsg_set[i]);
		if (ret != 0) {
			return ret;
		}
	}

	stm32mp1_mco_csg(clksrc[CLKSRC_MCO1], clkdiv[CLKDIV_MCO1]);
	stm32mp1_mco_csg(clksrc[CLKSRC_MCO2], clkdiv[CLKDIV_MCO2]);

	/*
	 * Switch ON oscillator found in device-tree.
	 * Note: HSI already ON after BootROM stage.
	 */
	if (stm32mp1_osc[_LSI] != 0U) {
		stm32mp1_lsi_set(true);
	}
	if (stm32mp1_osc[_LSE] != 0U) {
		const char *name = stm32mp_osc_node_label[_LSE];
		bool bypass, digbyp;
		uint32_t lsedrv;

		bypass = fdt_clk_read_bool(name, "st,bypass");
		digbyp = fdt_clk_read_bool(name, "st,digbypass");
		lse_css = fdt_clk_read_bool(name, "st,css");
		lsedrv = fdt_clk_read_uint32_default(name, "st,drive",
						     LSEDRV_MEDIUM_HIGH);
		stm32mp1_lse_enable(bypass, digbyp, lsedrv);
	}
	if (stm32mp1_osc[_HSE] != 0U) {
		const char *name = stm32mp_osc_node_label[_HSE];
		bool bypass, digbyp, css;

		bypass = fdt_clk_read_bool(name, "st,bypass");
		digbyp = fdt_clk_read_bool(name, "st,digbypass");
		css = fdt_clk_read_bool(name, "st,css");
		stm32mp1_hse_enable(bypass, digbyp, css);
	}
	/*
	 * CSI is mandatory for automatic I/O compensation (SYSCFG_CMPCR)
	 * => switch on CSI even if node is not present in device tree
	 */
	stm32mp1_csi_set(true);

	/* Come back to HSI */
	ret = stm32mp1_set_clksrc(CLK_MPU_HSI);
	if (ret != 0) {
		return ret;
	}
	ret = stm32mp1_set_clksrc(CLK_AXI_HSI);
	if (ret != 0) {
		return ret;
	}
	ret = stm32mp1_set_clksrc(CLK_MCU_HSI);
	if (ret != 0) {
		return ret;
	}

	if ((mmio_read_32(rcc_base + RCC_MP_RSTSCLRR) &
	     RCC_MP_RSTSCLRR_MPUP0RSTF) != 0) {
		if (pllcfg_valid[_PLL3]) {
			pll3_preserve =
				stm32mp1_check_pll_conf(_PLL3,
							clksrc[CLKSRC_PLL3],
							pllcfg[_PLL3],
							plloff[_PLL3]);
		}

		if (pllcfg_valid[_PLL4]) {
			pll4_preserve =
				stm32mp1_check_pll_conf(_PLL4,
							clksrc[CLKSRC_PLL4],
							pllcfg[_PLL4],
							plloff[_PLL4]);
		}
	}
	/* Don't initialize PLL4, when used by BOOTROM */
	if ((stm32mp_get_boot_itf_selected() ==
	     BOOT_API_CTX_BOOT_INTERFACE_SEL_SERIAL_USB) &&
	    ((stgen_p == (int)_PLL4_R) || (usbphy_p == (int)_PLL4_R))) {
		pll4_bootrom = true;
		pll4_preserve = true;
	}

	for (i = (enum stm32mp1_pll_id)0; i < _PLL_NB; i++) {
		if (((i == _PLL3) && pll3_preserve) ||
		    ((i == _PLL4) && pll4_preserve)) {
			continue;
		}

		ret = stm32mp1_pll_stop(i);
		if (ret != 0) {
			return ret;
		}
	}

	/* Configure HSIDIV */
	if (stm32mp1_osc[_HSI] != 0U) {
		ret = stm32mp1_hsidiv(stm32mp1_osc[_HSI]);
		if (ret != 0) {
			return ret;
		}

		stm32mp_stgen_config(stm32mp_clk_get_rate(STGEN_K));
	}

	/* Select DIV */
	/* No ready bit when MPUSRC != CLK_MPU_PLL1P_DIV, MPUDIV is disabled */
	mmio_write_32(rcc_base + RCC_MPCKDIVR,
		      clkdiv[CLKDIV_MPU] & RCC_DIVR_DIV_MASK);
	ret = stm32mp1_set_clkdiv(clkdiv[CLKDIV_AXI], rcc_base + RCC_AXIDIVR);
	if (ret != 0) {
		return ret;
	}
	ret = stm32mp1_set_clkdiv(clkdiv[CLKDIV_APB4], rcc_base + RCC_APB4DIVR);
	if (ret != 0) {
		return ret;
	}
	ret = stm32mp1_set_clkdiv(clkdiv[CLKDIV_APB5], rcc_base + RCC_APB5DIVR);
	if (ret != 0) {
		return ret;
	}
	ret = stm32mp1_set_clkdiv(clkdiv[CLKDIV_MCU], rcc_base + RCC_MCUDIVR);
	if (ret != 0) {
		return ret;
	}
	ret = stm32mp1_set_clkdiv(clkdiv[CLKDIV_APB1], rcc_base + RCC_APB1DIVR);
	if (ret != 0) {
		return ret;
	}
	ret = stm32mp1_set_clkdiv(clkdiv[CLKDIV_APB2], rcc_base + RCC_APB2DIVR);
	if (ret != 0) {
		return ret;
	}
	ret = stm32mp1_set_clkdiv(clkdiv[CLKDIV_APB3], rcc_base + RCC_APB3DIVR);
	if (ret != 0) {
		return ret;
	}

	/* No ready bit for RTC */
	mmio_write_32(rcc_base + RCC_RTCDIVR,
		      clkdiv[CLKDIV_RTC] & RCC_DIVR_DIV_MASK);

	/* Configure PLLs source */
	ret = stm32mp1_set_clksrc(clksrc[CLKSRC_PLL12]);
	if (ret != 0) {
		return ret;
	}

	if (!pll3_preserve) {
		ret = stm32mp1_set_clksrc(clksrc[CLKSRC_PLL3]);
		if (ret != 0) {
			return ret;
		}
	}

	if (!pll4_preserve) {
		ret = stm32mp1_set_clksrc(clksrc[CLKSRC_PLL4]);
		if (ret != 0) {
			return ret;
		}
	}

	/* Configure and start PLLs */
	for (i = (enum stm32mp1_pll_id)0; i < _PLL_NB; i++) {
		if (((i == _PLL3) && pll3_preserve) ||
		    ((i == _PLL4) && pll4_preserve && !pll4_bootrom)) {
			continue;
		}

		if (!pllcfg_valid[i]) {
			continue;
		}

		if ((i == _PLL4) && pll4_bootrom) {
			/* Set output divider if not done by the Bootrom */
			stm32mp1_pll_config_output(i, pllcfg[i]);
			continue;
		}

		ret = stm32mp1_pll_config(i, pllcfg[i], pllfracv[i]);
		if (ret != 0) {
			return ret;
		}

		if (pllcsg_set[i]) {
			stm32mp1_pll_csg(i, pllcsg[i]);
		}

		stm32mp1_pll_start(i);
	}
	/* Wait and start PLLs output when ready */
	for (i = (enum stm32mp1_pll_id)0; i < _PLL_NB; i++) {
		if (!pllcfg_valid[i]) {
			continue;
		}

		ret = stm32mp1_pll_output(i, pllcfg[i][PLLCFG_O]);
		if (ret != 0) {
			return ret;
		}
	}
	/* Wait LSE ready before to use it */
	if (stm32mp1_osc[_LSE] != 0U) {
		stm32mp1_lse_wait();
	}

	/* Configure with expected clock source */
	ret = stm32mp1_set_clksrc(clksrc[CLKSRC_MPU]);
	if (ret != 0) {
		return ret;
	}
	ret = stm32mp1_set_clksrc(clksrc[CLKSRC_AXI]);
	if (ret != 0) {
		return ret;
	}
	ret = stm32mp1_set_clksrc(clksrc[CLKSRC_MCU]);
	if (ret != 0) {
		return ret;
	}
	stm32mp1_set_rtcsrc(clksrc[CLKSRC_RTC], lse_css);

	/* Configure PKCK */
	pkcs_cell = fdt_rcc_read_prop("st,pkcs", &len);
	if (pkcs_cell != NULL) {
		bool ckper_disabled = false;
		uint32_t j;
		uint32_t usbreg_bootrom = 0U;

		if (pll4_bootrom) {
			usbreg_bootrom = mmio_read_32(rcc_base + RCC_USBCKSELR);
		}

		for (j = 0; j < ((uint32_t)len / sizeof(uint32_t)); j++) {
			uint32_t pkcs = fdt32_to_cpu(pkcs_cell[j]);

			if (pkcs == (uint32_t)CLK_CKPER_DISABLED) {
				ckper_disabled = true;
				continue;
			}
			stm32mp1_pkcs_config(pkcs);
		}

		/*
		 * CKPER is source for some peripheral clocks
		 * (FMC-NAND / QPSI-NOR) and switching source is allowed
		 * only if previous clock is still ON
		 * => deactivated CKPER only after switching clock
		 */
		if (ckper_disabled) {
			stm32mp1_pkcs_config(CLK_CKPER_DISABLED);
		}

		if (pll4_bootrom) {
			uint32_t usbreg_value, usbreg_mask;
			const struct stm32mp1_clk_sel *sel;

			sel = clk_sel_ref(_USBPHY_SEL);
			usbreg_mask = (uint32_t)sel->msk << sel->src;
			sel = clk_sel_ref(_USBO_SEL);
			usbreg_mask |= (uint32_t)sel->msk << sel->src;

			usbreg_value = mmio_read_32(rcc_base + RCC_USBCKSELR) &
				       usbreg_mask;
			usbreg_bootrom &= usbreg_mask;
			if (usbreg_bootrom != usbreg_value) {
				VERBOSE("forbidden new USB clk path\n");
				VERBOSE("vs bootrom on USB boot\n");
				return -FDT_ERR_BADVALUE;
			}
		}
	}

	/* Switch OFF HSI if not found in device-tree */
	if (stm32mp1_osc[_HSI] == 0U) {
		stm32mp1_hsi_set(false);
	}

	stm32mp_stgen_config(stm32mp_clk_get_rate(STGEN_K));

	/* Software Self-Refresh mode (SSR) during DDR initilialization */
	mmio_clrsetbits_32(rcc_base + RCC_DDRITFCR,
			   RCC_DDRITFCR_DDRCKMOD_MASK,
			   RCC_DDRITFCR_DDRCKMOD_SSR <<
			   RCC_DDRITFCR_DDRCKMOD_SHIFT);

	return 0;
}

static void stm32mp1_osc_clk_init(const char *name,
				  enum stm32mp_osc_id index)
{
	uint32_t frequency;

	if (fdt_osc_read_freq(name, &frequency) == 0) {
		stm32mp1_osc[index] = frequency;
	}
}

static void stm32mp1_osc_init(void)
{
	enum stm32mp_osc_id i;

	for (i = (enum stm32mp_osc_id)0 ; i < NB_OSC; i++) {
		stm32mp1_osc_clk_init(stm32mp_osc_node_label[i], i);
	}
}

#ifdef STM32MP_SHARED_RESOURCES
/*
 * Get the parent ID of the target parent clock, for tagging as secure
 * shared clock dependencies.
 */
static int get_parent_id_parent(unsigned int parent_id)
{
	enum stm32mp1_parent_sel s = _UNKNOWN_SEL;
	enum stm32mp1_pll_id pll_id;
	uint32_t p_sel;
	uintptr_t rcc_base = stm32mp_rcc_base();

	switch (parent_id) {
	case _ACLK:
	case _PCLK4:
	case _PCLK5:
		s = _AXIS_SEL;
		break;
	case _PLL1_P:
	case _PLL1_Q:
	case _PLL1_R:
		pll_id = _PLL1;
		break;
	case _PLL2_P:
	case _PLL2_Q:
	case _PLL2_R:
		pll_id = _PLL2;
		break;
	case _PLL3_P:
	case _PLL3_Q:
	case _PLL3_R:
		pll_id = _PLL3;
		break;
	case _PLL4_P:
	case _PLL4_Q:
	case _PLL4_R:
		pll_id = _PLL4;
		break;
	case _PCLK1:
	case _PCLK2:
	case _HCLK2:
	case _HCLK6:
	case _CK_PER:
	case _CK_MPU:
	case _CK_MCU:
	case _USB_PHY_48:
		/* We do not expect to access these */
		panic();
		break;
	default:
		/* Other parents have no parent */
		return -1;
	}

	if (s != _UNKNOWN_SEL) {
		const struct stm32mp1_clk_sel *sel = clk_sel_ref(s);

		p_sel = (mmio_read_32(rcc_base + sel->offset) >> sel->src) &
			sel->msk;

		if (p_sel < sel->nb_parent) {
			return (int)sel->parent[p_sel];
		}
	} else {
		const struct stm32mp1_clk_pll *pll = pll_ref(pll_id);

		p_sel = mmio_read_32(rcc_base + pll->rckxselr) &
			RCC_SELR_REFCLK_SRC_MASK;

		if (pll->refclk[p_sel] != _UNKNOWN_OSC_ID) {
			return (int)pll->refclk[p_sel];
		}
	}

	VERBOSE("No parent selected for %s\n",
		stm32mp1_clk_parent_name[parent_id]);

	return -1;
}

static void secure_parent_clocks(unsigned long parent_id)
{
	int grandparent_id;

	switch (parent_id) {
	case _PLL3_P:
	case _PLL3_Q:
	case _PLL3_R:
		stm32mp_register_secure_periph(STM32MP1_SHRES_PLL3);
		break;

	/* These clocks are always secure when RCC is secure */
	case _ACLK:
	case _HCLK2:
	case _HCLK6:
	case _PCLK4:
	case _PCLK5:
	case _PLL1_P:
	case _PLL1_Q:
	case _PLL1_R:
	case _PLL2_P:
	case _PLL2_Q:
	case _PLL2_R:
	case _HSI:
	case _HSI_KER:
	case _LSI:
	case _CSI:
	case _CSI_KER:
	case _HSE:
	case _HSE_KER:
	case _HSE_KER_DIV2:
	case _HSE_RTC:
	case _LSE:
		break;

	default:
		VERBOSE("Cannot secure parent clock %s\n",
			stm32mp1_clk_parent_name[parent_id]);
		panic();
	}

	grandparent_id = get_parent_id_parent(parent_id);
	if (grandparent_id >= 0) {
		secure_parent_clocks(grandparent_id);
	}
}

void stm32mp1_register_clock_parents_secure(unsigned long clock_id)
{
	int parent_id;

	if (!stm32mp1_rcc_is_secure()) {
		return;
	}

	switch (clock_id) {
	case PLL1:
	case PLL2:
		/* PLL1/PLL2 are always secure: nothing to do */
		break;
	case PLL3:
		stm32mp_register_secure_periph(STM32MP1_SHRES_PLL3);
		break;
	case PLL4:
		ERROR("PLL4 cannot be secured\n");
		panic();
		break;
	default:
		/* Others are expected gateable clock */
		parent_id = stm32mp1_clk_get_parent(clock_id);
		if (parent_id < 0) {
			INFO("No parent found for clock %lu\n", clock_id);
		} else {
			secure_parent_clocks(parent_id);
		}
		break;
	}
}
#endif /* STM32MP_SHARED_RESOURCES */

void stm32mp1_clk_mcuss_protect(bool enable)
{
	uintptr_t rcc_base = stm32mp_rcc_base();

	if (enable) {
		mmio_setbits_32(rcc_base + RCC_TZCR, RCC_TZCR_MCKPROT);
	} else {
		mmio_clrbits_32(rcc_base + RCC_TZCR, RCC_TZCR_MCKPROT);
	}
}

static void sync_earlyboot_clocks_state(void)
{
	unsigned int idx;
	const unsigned long secure_enable[] = {
		AXIDCG,
		BSEC,
		DDRC1, DDRC1LP,
		DDRC2, DDRC2LP,
		DDRCAPB, DDRPHYCAPB, DDRPHYCAPBLP,
		DDRPHYC, DDRPHYCLP,
		RTCAPB,
		TZC1, TZC2,
		TZPC,
		STGEN_K,
	};

	for (idx = 0U; idx < ARRAY_SIZE(secure_enable); idx++) {
		stm32mp_clk_enable(secure_enable[idx]);
	}
}

static const struct clk_ops stm32mp_clk_ops = {
	.enable		= stm32mp_clk_enable,
	.disable	= stm32mp_clk_disable,
	.is_enabled	= stm32mp_clk_is_enabled,
	.get_rate	= stm32mp_clk_get_rate,
	.get_parent	= stm32mp1_clk_get_parent,
};

int stm32mp1_clk_probe(void)
{
#if defined(IMAGE_BL32)
	if (!fdt_get_rcc_secure_state()) {
		mmio_write_32(stm32mp_rcc_base() + RCC_TZCR, 0U);
	}
#endif

	stm32mp1_osc_init();

	sync_earlyboot_clocks_state();

	clk_register(&stm32mp_clk_ops);

	return 0;
}
