/*
 * Copyright (C) 2018, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
 */

#include <arch.h>
#include <arch_helpers.h>
#include <assert.h>
#include <debug.h>
#include <delay_timer.h>
#include <dt-bindings/clock/stm32mp1-clks.h>
#include <dt-bindings/clock/stm32mp1-clksrc.h>
#include <errno.h>
#include <generic_delay_timer.h>
#include <libfdt.h>
#include <mmio.h>
#include <platform.h>
#include <stdint.h>
#include <stdio.h>
#include <stm32mp1_clk.h>
#include <stm32mp1_clkfunc.h>
#include <stm32mp1_dt.h>
#include <stm32mp1_private.h>
#include <stm32mp1_rcc.h>
#include <utils_def.h>

#define MAX_HSI_HZ	64000000

#define TIMEOUT_200MS	(plat_get_syscnt_freq2() / 5U)
#define TIMEOUT_1S	plat_get_syscnt_freq2()

#define PLLRDY_TIMEOUT	TIMEOUT_200MS
#define CLKSRC_TIMEOUT	TIMEOUT_200MS
#define CLKDIV_TIMEOUT	TIMEOUT_200MS
#define HSIDIV_TIMEOUT	TIMEOUT_200MS
#define OSCRDY_TIMEOUT	TIMEOUT_1S

enum stm32mp1_parent_id {
/* Oscillators are defined in enum stm32mp_osc_id */

/* Other parent source */
	_HSI_KER = NB_OSC,
	_HSE_KER,
	_HSE_KER_DIV2,
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
	_PARENT_NB,
	_UNKNOWN_ID = 0xff,
};

enum stm32mp1_parent_sel {
	_I2C46_SEL,
	_UART6_SEL,
	_UART24_SEL,
	_UART35_SEL,
	_UART78_SEL,
	_SDMMC12_SEL,
	_SDMMC3_SEL,
	_QSPI_SEL,
	_FMC_SEL,
	_USBPHY_SEL,
	_USBO_SEL,
	_STGEN_SEL,
	_PARENT_SEL_NB,
	_UNKNOWN_SEL = 0xff,
};

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
	uint8_t divn_max;
};

struct stm32mp1_clk_gate {
	uint16_t offset;
	uint8_t bit;
	uint8_t index;
	uint8_t set_clr;
	enum stm32mp1_parent_sel sel;
	enum stm32mp1_parent_id fixed;
	bool secure;
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

struct stm32mp1_clk_data {
	const struct stm32mp1_clk_gate *gate;
	const struct stm32mp1_clk_sel *sel;
	const struct stm32mp1_clk_pll *pll;
	const int nb_gate;
};

struct stm32mp1_clk_priv {
	uint32_t base;
	const struct stm32mp1_clk_data *data;
	unsigned long osc[NB_OSC];
	uint32_t pkcs_usb_value;
};

#define STM32MP1_CLK(off, b, idx, s)			\
	{						\
		.offset = (off),			\
		.bit = (b),				\
		.index = (idx),				\
		.set_clr = 0,				\
		.sel = (s),				\
		.fixed = _UNKNOWN_ID,			\
		.secure = 0,				\
	}

#define STM32MP1_CLK_F(off, b, idx, f)			\
	{						\
		.offset = (off),			\
		.bit = (b),				\
		.index = (idx),				\
		.set_clr = 0,				\
		.sel = _UNKNOWN_SEL,			\
		.fixed = (f),				\
		.secure = 0,				\
	}

#define STM32MP1_CLK_SET_CLR(off, b, idx, s)		\
	{						\
		.offset = (off),			\
		.bit = (b),				\
		.index = (idx),				\
		.set_clr = 1,				\
		.sel = (s),				\
		.fixed = _UNKNOWN_ID,			\
		.secure = 0,				\
	}

#define STM32MP1_CLK_SET_CLR_F(off, b, idx, f)		\
	{						\
		.offset = (off),			\
		.bit = (b),				\
		.index = (idx),				\
		.set_clr = 1,				\
		.sel = _UNKNOWN_SEL,			\
		.fixed = (f),				\
		.secure = 0,				\
	}

#define STM32MP1_CLK_SEC_SET_CLR(off, b, idx, s)	\
	{						\
		.offset = (off),			\
		.bit = (b),				\
		.index = (idx),				\
		.set_clr = 1,				\
		.sel = (s),				\
		.fixed = _UNKNOWN_ID,			\
		.secure = 1,				\
	}

#define STM32MP1_CLK_PARENT(idx, off, s, m, p)		\
	[(idx)] = {					\
		.offset = (off),			\
		.src = (s),				\
		.msk = (m),				\
		.parent = (p),				\
		.nb_parent = ARRAY_SIZE((p))		\
	}

#define STM32MP1_CLK_PLL(idx, type, off1, off2, off3,	\
			 off4, off5, off6,		\
			 p1, p2, p3, p4)		\
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

static const uint8_t stm32mp1_clks[][2] = {
	{CK_PER, _CK_PER},
	{CK_MPU, _CK_MPU},
	{CK_AXI, _ACLK},
	{CK_HSE, _HSE},
	{CK_CSI, _CSI},
	{CK_LSI, _LSI},
	{CK_LSE, _LSE},
	{CK_HSI, _HSI},
	{CK_HSE_DIV2, _HSE_KER_DIV2},
};

static const struct stm32mp1_clk_gate stm32mp1_clk_gate[] = {
	STM32MP1_CLK(RCC_DDRITFCR, 0, DDRC1, _UNKNOWN_SEL),
	STM32MP1_CLK(RCC_DDRITFCR, 1, DDRC1LP, _UNKNOWN_SEL),
	STM32MP1_CLK(RCC_DDRITFCR, 2, DDRC2, _UNKNOWN_SEL),
	STM32MP1_CLK(RCC_DDRITFCR, 3, DDRC2LP, _UNKNOWN_SEL),
	STM32MP1_CLK_F(RCC_DDRITFCR, 4, DDRPHYC, _PLL2_R),
	STM32MP1_CLK(RCC_DDRITFCR, 5, DDRPHYCLP, _UNKNOWN_SEL),
	STM32MP1_CLK(RCC_DDRITFCR, 6, DDRCAPB, _UNKNOWN_SEL),
	STM32MP1_CLK(RCC_DDRITFCR, 7, DDRCAPBLP, _UNKNOWN_SEL),
	STM32MP1_CLK(RCC_DDRITFCR, 8, AXIDCG, _UNKNOWN_SEL),
	STM32MP1_CLK(RCC_DDRITFCR, 9, DDRPHYCAPB, _UNKNOWN_SEL),
	STM32MP1_CLK(RCC_DDRITFCR, 10, DDRPHYCAPBLP, _UNKNOWN_SEL),

	STM32MP1_CLK_SET_CLR(RCC_MP_APB1ENSETR, 14, USART2_K, _UART24_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_APB1ENSETR, 15, USART3_K, _UART35_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_APB1ENSETR, 16, UART4_K, _UART24_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_APB1ENSETR, 17, UART5_K, _UART35_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_APB1ENSETR, 18, UART7_K, _UART78_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_APB1ENSETR, 19, UART8_K, _UART78_SEL),

	STM32MP1_CLK_SET_CLR(RCC_MP_APB2ENSETR, 13, USART6_K, _UART6_SEL),

	STM32MP1_CLK_SET_CLR(RCC_MP_APB4ENSETR, 8, DDRPERFM, _UNKNOWN_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_APB4ENSETR, 15, IWDG2, _UNKNOWN_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_APB4ENSETR, 16, USBPHY_K, _USBPHY_SEL),

	STM32MP1_CLK_SEC_SET_CLR(RCC_MP_APB5ENSETR, 2, I2C4_K, _I2C46_SEL),
	STM32MP1_CLK_SEC_SET_CLR(RCC_MP_APB5ENSETR, 8, RTCAPB, _PCLK5),
	STM32MP1_CLK_SEC_SET_CLR(RCC_MP_APB5ENSETR, 11, TZC1, _UNKNOWN_SEL),
	STM32MP1_CLK_SEC_SET_CLR(RCC_MP_APB5ENSETR, 12, TZC2, _UNKNOWN_SEL),
	STM32MP1_CLK_SEC_SET_CLR(RCC_MP_APB5ENSETR, 20, STGEN_K, _STGEN_SEL),

	STM32MP1_CLK_SET_CLR(RCC_MP_AHB2ENSETR, 8, USBO_K, _USBO_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_AHB2ENSETR, 16, SDMMC3_K, _SDMMC3_SEL),

	STM32MP1_CLK_SET_CLR(RCC_MP_AHB4ENSETR, 0, GPIOA, _UNKNOWN_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_AHB4ENSETR, 1, GPIOB, _UNKNOWN_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_AHB4ENSETR, 2, GPIOC, _UNKNOWN_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_AHB4ENSETR, 3, GPIOD, _UNKNOWN_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_AHB4ENSETR, 4, GPIOE, _UNKNOWN_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_AHB4ENSETR, 5, GPIOF, _UNKNOWN_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_AHB4ENSETR, 6, GPIOG, _UNKNOWN_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_AHB4ENSETR, 7, GPIOH, _UNKNOWN_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_AHB4ENSETR, 8, GPIOI, _UNKNOWN_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_AHB4ENSETR, 9, GPIOJ, _UNKNOWN_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_AHB4ENSETR, 10, GPIOK, _UNKNOWN_SEL),

	STM32MP1_CLK_SEC_SET_CLR(RCC_MP_AHB5ENSETR, 0, GPIOZ, _UNKNOWN_SEL),
	STM32MP1_CLK_SEC_SET_CLR(RCC_MP_AHB5ENSETR, 5, HASH1, _UNKNOWN_SEL),
	STM32MP1_CLK_SEC_SET_CLR(RCC_MP_AHB5ENSETR, 6, RNG1_K, _CSI_KER),
	STM32MP1_CLK_SEC_SET_CLR(RCC_MP_AHB5ENSETR, 8, BKPSRAM, _UNKNOWN_SEL),

	STM32MP1_CLK_SET_CLR(RCC_MP_AHB6ENSETR, 12, FMC_K, _FMC_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_AHB6ENSETR, 14, QSPI_K, _QSPI_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_AHB6ENSETR, 16, SDMMC1_K, _SDMMC12_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_AHB6ENSETR, 17, SDMMC2_K, _SDMMC12_SEL),
	STM32MP1_CLK_SET_CLR(RCC_MP_AHB6ENSETR, 24, USBH, _UNKNOWN_SEL),

	STM32MP1_CLK(RCC_DBGCFGR, 8, CK_DBG, _UNKNOWN_SEL),
};

static const uint8_t i2c46_parents[] = {_PCLK5, _PLL3_Q, _HSI_KER, _CSI_KER};
static const uint8_t uart6_parents[] = {_PCLK2, _PLL4_Q, _HSI_KER, _CSI_KER,
					_HSE_KER};
static const uint8_t uart24_parents[] = {_PCLK1, _PLL4_Q, _HSI_KER, _CSI_KER,
					 _HSE_KER};
static const uint8_t uart35_parents[] = {_PCLK1, _PLL4_Q, _HSI_KER, _CSI_KER,
					 _HSE_KER};
static const uint8_t uart78_parents[] = {_PCLK1, _PLL4_Q, _HSI_KER, _CSI_KER,
					 _HSE_KER};
static const uint8_t sdmmc12_parents[] = {_HCLK6, _PLL3_R, _PLL4_P, _HSI_KER};
static const uint8_t sdmmc3_parents[] = {_HCLK2, _PLL3_R, _PLL4_P, _HSI_KER};
static const uint8_t qspi_parents[] = {_ACLK, _PLL3_R, _PLL4_P, _CK_PER};
static const uint8_t fmc_parents[] = {_ACLK, _PLL3_R, _PLL4_P, _CK_PER};
static const uint8_t usbphy_parents[] = {_HSE_KER, _PLL4_R, _HSE_KER_DIV2};
static const uint8_t usbo_parents[] = {_PLL4_R, _USB_PHY_48};
static const uint8_t stgen_parents[] = {_HSI_KER, _HSE_KER};

static const struct stm32mp1_clk_sel stm32mp1_clk_sel[_PARENT_SEL_NB] = {
	STM32MP1_CLK_PARENT(_I2C46_SEL, RCC_I2C46CKSELR, 0, 0x7, i2c46_parents),
	STM32MP1_CLK_PARENT(_UART6_SEL, RCC_UART6CKSELR, 0, 0x7, uart6_parents),
	STM32MP1_CLK_PARENT(_UART24_SEL, RCC_UART24CKSELR, 0, 0x7,
			    uart24_parents),
	STM32MP1_CLK_PARENT(_UART35_SEL, RCC_UART35CKSELR, 0, 0x7,
			    uart35_parents),
	STM32MP1_CLK_PARENT(_UART78_SEL, RCC_UART78CKSELR, 0, 0x7,
			    uart78_parents),
	STM32MP1_CLK_PARENT(_SDMMC12_SEL, RCC_SDMMC12CKSELR, 0, 0x7,
			    sdmmc12_parents),
	STM32MP1_CLK_PARENT(_SDMMC3_SEL, RCC_SDMMC3CKSELR, 0, 0x7,
			    sdmmc3_parents),
	STM32MP1_CLK_PARENT(_QSPI_SEL, RCC_QSPICKSELR, 0, 0xf, qspi_parents),
	STM32MP1_CLK_PARENT(_FMC_SEL, RCC_FMCCKSELR, 0, 0xf, fmc_parents),
	STM32MP1_CLK_PARENT(_USBPHY_SEL, RCC_USBCKSELR, 0, 0x3, usbphy_parents),
	STM32MP1_CLK_PARENT(_USBO_SEL, RCC_USBCKSELR, 4, 0x1, usbo_parents),
	STM32MP1_CLK_PARENT(_STGEN_SEL, RCC_STGENCKSELR, 0, 0x3, stgen_parents),
};

/* Define characteristic of PLL according type */
#define DIVN_MIN	24
static const struct stm32mp1_pll stm32mp1_pll[PLL_TYPE_NB] = {
	[PLL_800] = {
		.refclk_min = 4,
		.refclk_max = 16,
		.divn_max = 99,
	},
	[PLL_1600] = {
		.refclk_min = 8,
		.refclk_max = 16,
		.divn_max = 199,
	},
};

/* PLLNCFGR2 register divider by output */
static const uint8_t pllncfgr2[_DIV_NB] = {
	[_DIV_P] = RCC_PLLNCFGR2_DIVP_SHIFT,
	[_DIV_Q] = RCC_PLLNCFGR2_DIVQ_SHIFT,
	[_DIV_R] = RCC_PLLNCFGR2_DIVR_SHIFT
};

static const struct stm32mp1_clk_pll stm32mp1_clk_pll[_PLL_NB] = {
	STM32MP1_CLK_PLL(_PLL1, PLL_1600,
			 RCC_RCK12SELR, RCC_PLL1CFGR1, RCC_PLL1CFGR2,
			 RCC_PLL1FRACR, RCC_PLL1CR, RCC_PLL1CSGR,
			 _HSI, _HSE, _UNKNOWN_OSC_ID, _UNKNOWN_OSC_ID),
	STM32MP1_CLK_PLL(_PLL2, PLL_1600,
			 RCC_RCK12SELR, RCC_PLL2CFGR1, RCC_PLL2CFGR2,
			 RCC_PLL2FRACR, RCC_PLL2CR, RCC_PLL2CSGR,
			 _HSI, _HSE, _UNKNOWN_OSC_ID, _UNKNOWN_OSC_ID),
	STM32MP1_CLK_PLL(_PLL3, PLL_800,
			 RCC_RCK3SELR, RCC_PLL3CFGR1, RCC_PLL3CFGR2,
			 RCC_PLL3FRACR, RCC_PLL3CR, RCC_PLL3CSGR,
			 _HSI, _HSE, _CSI, _UNKNOWN_OSC_ID),
	STM32MP1_CLK_PLL(_PLL4, PLL_800,
			 RCC_RCK4SELR, RCC_PLL4CFGR1, RCC_PLL4CFGR2,
			 RCC_PLL4FRACR, RCC_PLL4CR, RCC_PLL4CSGR,
			 _HSI, _HSE, _CSI, _I2S_CKIN),
};

/* Prescaler table lookups for clock computation */

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

static const struct stm32mp1_clk_data stm32mp1_data = {
	.gate = stm32mp1_clk_gate,
	.sel = stm32mp1_clk_sel,
	.pll = stm32mp1_clk_pll,
	.nb_gate = ARRAY_SIZE(stm32mp1_clk_gate),
};

static struct stm32mp1_clk_priv stm32mp1_clk_priv_data;

static unsigned long stm32mp1_clk_get_fixed(struct stm32mp1_clk_priv *priv,
					    enum stm32mp_osc_id idx)
{
	if (idx >= NB_OSC) {
		return 0;
	}

	return priv->osc[idx];
}

static int stm32mp1_clk_get_id(struct stm32mp1_clk_priv *priv, unsigned long id)
{
	const struct stm32mp1_clk_gate *gate = priv->data->gate;
	int i;
	int nb_clks = priv->data->nb_gate;

	for (i = 0; i < nb_clks; i++) {
		if (gate[i].index == id) {
			return i;
		}
	}

	ERROR("%s: clk id %d not found\n", __func__, (uint32_t)id);

	return -EINVAL;
}

static enum stm32mp1_parent_sel
stm32mp1_clk_get_sel(struct stm32mp1_clk_priv *priv, int i)
{
	const struct stm32mp1_clk_gate *gate = priv->data->gate;

	return gate[i].sel;
}

static enum stm32mp1_parent_id
stm32mp1_clk_get_fixed_parent(struct stm32mp1_clk_priv *priv, int i)
{
	const struct stm32mp1_clk_gate *gate = priv->data->gate;

	return gate[i].fixed;
}

static int stm32mp1_clk_get_parent(struct stm32mp1_clk_priv *priv,
				   unsigned long id)
{
	const struct stm32mp1_clk_sel *sel = priv->data->sel;
	uint32_t j, p_sel;
	int i;
	enum stm32mp1_parent_id p;
	enum stm32mp1_parent_sel s;

	for (j = 0; j < ARRAY_SIZE(stm32mp1_clks); j++) {
		if (stm32mp1_clks[j][0] == id) {
			return (int)stm32mp1_clks[j][1];
		}
	}

	i = stm32mp1_clk_get_id(priv, id);
	if (i < 0) {
		return i;
	}

	p = stm32mp1_clk_get_fixed_parent(priv, i);
	if (p < _PARENT_NB) {
		return (int)p;
	}

	s = stm32mp1_clk_get_sel(priv, i);
	if (s >= _PARENT_SEL_NB) {
		return -EINVAL;
	}

	p_sel = (mmio_read_32(priv->base + sel[s].offset) >> sel[s].src) &
		sel[s].msk;

	if (p_sel < sel[s].nb_parent) {
		return (int)sel[s].parent[p_sel];
	}

	ERROR("%s: no parents defined for clk id %ld\n", __func__, id);

	return -EINVAL;
}

static unsigned long stm32mp1_pll_get_fref_ck(struct stm32mp1_clk_priv *priv,
					      enum stm32mp1_pll_id pll_id)
{
	const struct stm32mp1_clk_pll *pll = priv->data->pll;
	uint32_t selr, src;
	unsigned long refclk;

	selr = mmio_read_32(priv->base + pll[pll_id].rckxselr);
	src = selr & RCC_SELR_REFCLK_SRC_MASK;

	refclk = stm32mp1_clk_get_fixed(priv, pll[pll_id].refclk[src]);

	return refclk;
}

/*
 * pll_get_fvco() : return the VCO or (VCO / 2) frequency for the requested PLL
 * - PLL1 & PLL2 => return VCO / 2 with Fpll_y_ck = FVCO / 2 * (DIVy + 1)
 * - PLL3 & PLL4 => return VCO     with Fpll_y_ck = FVCO / (DIVy + 1)
 * => in all cases Fpll_y_ck = pll_get_fvco() / (DIVy + 1)
 */
static unsigned long stm32mp1_pll_get_fvco(struct stm32mp1_clk_priv *priv,
					   enum stm32mp1_pll_id pll_id)
{
	const struct stm32mp1_clk_pll *pll = priv->data->pll;
	unsigned long refclk, fvco;
	uint32_t cfgr1, fracr, divm, divn;

	cfgr1 = mmio_read_32(priv->base + pll[pll_id].pllxcfgr1);
	fracr = mmio_read_32(priv->base + pll[pll_id].pllxfracr);

	divm = (cfgr1 & (RCC_PLLNCFGR1_DIVM_MASK)) >> RCC_PLLNCFGR1_DIVM_SHIFT;
	divn = cfgr1 & RCC_PLLNCFGR1_DIVN_MASK;

	refclk = stm32mp1_pll_get_fref_ck(priv, pll_id);

	/*
	 * With FRACV :
	 *   Fvco = Fck_ref * ((DIVN + 1) + FRACV / 2^13) / (DIVM + 1)
	 * Without FRACV
	 *   Fvco = Fck_ref * ((DIVN + 1) / (DIVM + 1)
	 */
	if ((fracr & RCC_PLLNFRACR_FRACLE) != 0U) {
		uint32_t fracv = (fracr & RCC_PLLNFRACR_FRACV_MASK)
			    >> RCC_PLLNFRACR_FRACV_SHIFT;
		unsigned long long numerator, denominator;

		numerator = ((unsigned long long)divn + 1U) << 13;
		numerator = (refclk * numerator) + fracv;
		denominator = ((unsigned long long)divm + 1U)  << 13;
		fvco = (unsigned long)(numerator / denominator);
	} else {
		fvco = (unsigned long)(refclk * (divn + 1U) / (divm + 1U));
	}

	return fvco;
}

static unsigned long stm32mp1_read_pll_freq(struct stm32mp1_clk_priv *priv,
					    enum stm32mp1_pll_id pll_id,
					    enum stm32mp1_div_id div_id)
{
	const struct stm32mp1_clk_pll *pll = priv->data->pll;
	unsigned long dfout;
	uint32_t cfgr2, divy;

	if (div_id >= _DIV_NB) {
		return 0;
	}

	cfgr2 = mmio_read_32(priv->base + pll[pll_id].pllxcfgr2);
	divy = (cfgr2 >> pllncfgr2[div_id]) & RCC_PLLNCFGR2_DIVX_MASK;

	dfout = stm32mp1_pll_get_fvco(priv, pll_id) / (divy + 1U);

	return dfout;
}

static unsigned long stm32mp1_clk_get(struct stm32mp1_clk_priv *priv, int p)
{
	uint32_t reg, clkdiv;
	unsigned long clock = 0;

	switch (p) {
	case _CK_MPU:
	/* MPU sub system */
		reg = mmio_read_32(priv->base + RCC_MPCKSELR);
		switch (reg & RCC_SELR_SRC_MASK) {
		case RCC_MPCKSELR_HSI:
			clock = stm32mp1_clk_get_fixed(priv, _HSI);
			break;
		case RCC_MPCKSELR_HSE:
			clock = stm32mp1_clk_get_fixed(priv, _HSE);
			break;
		case RCC_MPCKSELR_PLL:
			clock = stm32mp1_read_pll_freq(priv, _PLL1, _DIV_P);
			break;
		case RCC_MPCKSELR_PLL_MPUDIV:
			clock = stm32mp1_read_pll_freq(priv, _PLL1, _DIV_P);

			reg = mmio_read_32(priv->base + RCC_MPCKDIVR);
			clkdiv = reg & RCC_MPUDIV_MASK;
			if (clkdiv != 0U) {
				clock /= stm32mp1_mpu_div[clkdiv];
			}

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
		reg = mmio_read_32(priv->base + RCC_ASSCKSELR);
		switch (reg & RCC_SELR_SRC_MASK) {
		case RCC_ASSCKSELR_HSI:
			clock = stm32mp1_clk_get_fixed(priv, _HSI);
			break;
		case RCC_ASSCKSELR_HSE:
			clock = stm32mp1_clk_get_fixed(priv, _HSE);
			break;
		case RCC_ASSCKSELR_PLL:
			clock = stm32mp1_read_pll_freq(priv, _PLL2, _DIV_P);
			break;
		default:
			break;
		}

		/* System clock divider */
		reg = mmio_read_32(priv->base + RCC_AXIDIVR);
		clock /= stm32mp1_axi_div[reg & RCC_AXIDIV_MASK];

		switch (p) {
		case _PCLK4:
			reg = mmio_read_32(priv->base + RCC_APB4DIVR);
			clock >>= stm32mp1_apbx_div[reg & RCC_APBXDIV_MASK];
			break;
		case _PCLK5:
			reg = mmio_read_32(priv->base + RCC_APB5DIVR);
			clock >>= stm32mp1_apbx_div[reg & RCC_APBXDIV_MASK];
			break;
		default:
			break;
		}
		break;
	case _CK_PER:
		reg = mmio_read_32(priv->base + RCC_CPERCKSELR);
		switch (reg & RCC_SELR_SRC_MASK) {
		case RCC_CPERCKSELR_HSI:
			clock = stm32mp1_clk_get_fixed(priv, _HSI);
			break;
		case RCC_CPERCKSELR_HSE:
			clock = stm32mp1_clk_get_fixed(priv, _HSE);
			break;
		case RCC_CPERCKSELR_CSI:
			clock = stm32mp1_clk_get_fixed(priv, _CSI);
			break;
		default:
			break;
		}
		break;
	case _HSI:
	case _HSI_KER:
		clock = stm32mp1_clk_get_fixed(priv, _HSI);
		break;
	case _CSI:
	case _CSI_KER:
		clock = stm32mp1_clk_get_fixed(priv, _CSI);
		break;
	case _HSE:
	case _HSE_KER:
		clock = stm32mp1_clk_get_fixed(priv, _HSE);
		break;
	case _HSE_KER_DIV2:
		clock = stm32mp1_clk_get_fixed(priv, _HSE) >> 1;
		break;
	case _LSI:
		clock = stm32mp1_clk_get_fixed(priv, _LSI);
		break;
	case _LSE:
		clock = stm32mp1_clk_get_fixed(priv, _LSE);
		break;
	/* PLL */
	case _PLL1_P:
		clock = stm32mp1_read_pll_freq(priv, _PLL1, _DIV_P);
		break;
	case _PLL1_Q:
		clock = stm32mp1_read_pll_freq(priv, _PLL1, _DIV_Q);
		break;
	case _PLL1_R:
		clock = stm32mp1_read_pll_freq(priv, _PLL1, _DIV_R);
		break;
	case _PLL2_P:
		clock = stm32mp1_read_pll_freq(priv, _PLL2, _DIV_P);
		break;
	case _PLL2_Q:
		clock = stm32mp1_read_pll_freq(priv, _PLL2, _DIV_Q);
		break;
	case _PLL2_R:
		clock = stm32mp1_read_pll_freq(priv, _PLL2, _DIV_R);
		break;
	case _PLL3_P:
		clock = stm32mp1_read_pll_freq(priv, _PLL3, _DIV_P);
		break;
	case _PLL3_Q:
		clock = stm32mp1_read_pll_freq(priv, _PLL3, _DIV_Q);
		break;
	case _PLL3_R:
		clock = stm32mp1_read_pll_freq(priv, _PLL3, _DIV_R);
		break;
	case _PLL4_P:
		clock = stm32mp1_read_pll_freq(priv, _PLL4, _DIV_P);
		break;
	case _PLL4_Q:
		clock = stm32mp1_read_pll_freq(priv, _PLL4, _DIV_Q);
		break;
	case _PLL4_R:
		clock = stm32mp1_read_pll_freq(priv, _PLL4, _DIV_R);
		break;
	/* Other */
	case _USB_PHY_48:
		clock = stm32mp1_clk_get_fixed(priv, _USB_PHY_48);
		break;
	default:
		break;
	}

	return clock;
}

bool stm32mp1_clk_is_enabled(unsigned long id)
{
	struct stm32mp1_clk_priv *priv = &stm32mp1_clk_priv_data;
	const struct stm32mp1_clk_gate *gate = priv->data->gate;
	int i = stm32mp1_clk_get_id(priv, id);

	if (i < 0) {
		return false;
	}

	return ((mmio_read_32(priv->base + gate[i].offset) &
		 BIT(gate[i].bit)) != 0U);
}

int stm32mp1_clk_enable(unsigned long id)
{
	struct stm32mp1_clk_priv *priv = &stm32mp1_clk_priv_data;
	const struct stm32mp1_clk_gate *gate = priv->data->gate;
	int i = stm32mp1_clk_get_id(priv, id);

	if (i < 0) {
		return i;
	}

	if (gate[i].set_clr != 0U) {
		mmio_write_32(priv->base + gate[i].offset, BIT(gate[i].bit));
	} else {
		mmio_setbits_32(priv->base + gate[i].offset, BIT(gate[i].bit));
	}

	return 0;
}

int stm32mp1_clk_disable(unsigned long id)
{
	struct stm32mp1_clk_priv *priv = &stm32mp1_clk_priv_data;
	const struct stm32mp1_clk_gate *gate = priv->data->gate;
	int i = stm32mp1_clk_get_id(priv, id);

	if (i < 0) {
		return i;
	}

	if (gate[i].set_clr != 0U) {
		mmio_write_32(priv->base + gate[i].offset
			      + RCC_MP_ENCLRR_OFFSET,
			      BIT(gate[i].bit));
	} else {
		mmio_clrbits_32(priv->base + gate[i].offset, BIT(gate[i].bit));
	}

	return 0;
}

unsigned long stm32mp1_clk_get_rate(unsigned long id)
{
	struct stm32mp1_clk_priv *priv = &stm32mp1_clk_priv_data;
	int p = stm32mp1_clk_get_parent(priv, id);
	unsigned long rate;

	if (p < 0) {
		return 0;
	}

	rate = stm32mp1_clk_get(priv, p);

	return rate;
}

static void stm32mp1_ls_osc_set(int enable, uint32_t rcc, uint32_t offset,
				uint32_t mask_on)
{
	uint32_t address = rcc + offset;

	if (enable != 0) {
		mmio_setbits_32(address, mask_on);
	} else {
		mmio_clrbits_32(address, mask_on);
	}
}

static void stm32mp1_hs_ocs_set(int enable, uint32_t rcc, uint32_t mask_on)
{
	if (enable != 0) {
		mmio_setbits_32(rcc + RCC_OCENSETR, mask_on);
	} else {
		mmio_setbits_32(rcc + RCC_OCENCLRR, mask_on);
	}
}

static int stm32mp1_osc_wait(int enable, uint32_t rcc, uint32_t offset,
			     uint32_t mask_rdy)
{
	unsigned long start;
	uint32_t mask_test;
	uint32_t address = rcc + offset;

	if (enable != 0) {
		mask_test = mask_rdy;
	} else {
		mask_test = 0;
	}

	start = get_timer(0);
	while ((mmio_read_32(address) & mask_rdy) != mask_test) {
		if (get_timer(start) > OSCRDY_TIMEOUT) {
			ERROR("OSC %x @ %x timeout for enable=%d : 0x%x\n",
			      mask_rdy, address, enable, mmio_read_32(address));
			return -ETIMEDOUT;
		}
	}

	return 0;
}

static void stm32mp1_lse_enable(uint32_t rcc, bool bypass, uint32_t lsedrv)
{
	uint32_t value;

	if (bypass) {
		mmio_setbits_32(rcc + RCC_BDCR, RCC_BDCR_LSEBYP);
	}

	/*
	 * Warning: not recommended to switch directly from "high drive"
	 * to "medium low drive", and vice-versa.
	 */
	value = (mmio_read_32(rcc + RCC_BDCR) & RCC_BDCR_LSEDRV_MASK) >>
		RCC_BDCR_LSEDRV_SHIFT;

	while (value != lsedrv) {
		if (value > lsedrv) {
			value--;
		} else {
			value++;
		}

		mmio_clrsetbits_32(rcc + RCC_BDCR,
				   RCC_BDCR_LSEDRV_MASK,
				   value << RCC_BDCR_LSEDRV_SHIFT);
	}

	stm32mp1_ls_osc_set(1, rcc, RCC_BDCR, RCC_BDCR_LSEON);
}

static void stm32mp1_lse_wait(uint32_t rcc)
{
	if (stm32mp1_osc_wait(1, rcc, RCC_BDCR, RCC_BDCR_LSERDY) != 0) {
		VERBOSE("%s: failed\n", __func__);
	}
}

static void stm32mp1_lsi_set(uint32_t rcc, int enable)
{
	stm32mp1_ls_osc_set(enable, rcc, RCC_RDLSICR, RCC_RDLSICR_LSION);
	if (stm32mp1_osc_wait(enable, rcc, RCC_RDLSICR, RCC_RDLSICR_LSIRDY) !=
	    0) {
		VERBOSE("%s: failed\n", __func__);
	}
}

static void stm32mp1_hse_enable(uint32_t rcc, bool bypass, bool css)
{
	if (bypass) {
		mmio_setbits_32(rcc + RCC_OCENSETR, RCC_OCENR_HSEBYP);
	}

	stm32mp1_hs_ocs_set(1, rcc, RCC_OCENR_HSEON);
	if (stm32mp1_osc_wait(1, rcc, RCC_OCRDYR, RCC_OCRDYR_HSERDY) !=
	    0) {
		VERBOSE("%s: failed\n", __func__);
	}

	if (css) {
		mmio_setbits_32(rcc + RCC_OCENSETR, RCC_OCENR_HSECSSON);
	}
}

static void stm32mp1_csi_set(uint32_t rcc, int enable)
{
	stm32mp1_ls_osc_set(enable, rcc, RCC_OCENSETR, RCC_OCENR_CSION);
	if (stm32mp1_osc_wait(enable, rcc, RCC_OCRDYR, RCC_OCRDYR_CSIRDY) !=
	    0) {
		VERBOSE("%s: failed\n", __func__);
	}
}

static void stm32mp1_hsi_set(uint32_t rcc, int enable)
{
	stm32mp1_hs_ocs_set(enable, rcc, RCC_OCENR_HSION);
	if (stm32mp1_osc_wait(enable, rcc, RCC_OCRDYR, RCC_OCRDYR_HSIRDY) !=
	    0) {
		VERBOSE("%s: failed\n", __func__);
	}
}

static int stm32mp1_set_hsidiv(uint32_t rcc, uint8_t hsidiv)
{
	unsigned long start;
	uint32_t address = rcc + RCC_OCRDYR;

	mmio_clrsetbits_32(rcc + RCC_HSICFGR,
			   RCC_HSICFGR_HSIDIV_MASK,
			   RCC_HSICFGR_HSIDIV_MASK & (uint32_t)hsidiv);

	start = get_timer(0);
	while ((mmio_read_32(address) & RCC_OCRDYR_HSIDIVRDY) == 0U) {
		if (get_timer(start) > HSIDIV_TIMEOUT) {
			ERROR("HSIDIV failed @ 0x%x: 0x%x\n",
			      address, mmio_read_32(address));
			return -ETIMEDOUT;
		}
	}

	return 0;
}

static int stm32mp1_hsidiv(uint32_t rcc, unsigned long hsifreq)
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
		return stm32mp1_set_hsidiv(rcc, hsidiv);
	}

	return 0;
}

static void stm32mp1_pll_start(struct stm32mp1_clk_priv *priv,
			       enum stm32mp1_pll_id pll_id)
{
	const struct stm32mp1_clk_pll *pll = priv->data->pll;

	mmio_write_32(priv->base + pll[pll_id].pllxcr, RCC_PLLNCR_PLLON);
}

static int stm32mp1_pll_output(struct stm32mp1_clk_priv *priv,
			       enum stm32mp1_pll_id pll_id, uint32_t output)
{
	const struct stm32mp1_clk_pll *pll = priv->data->pll;
	uint32_t pllxcr = priv->base + pll[pll_id].pllxcr;
	unsigned long start;

	start = get_timer(0);
	/* Wait PLL lock */
	while ((mmio_read_32(pllxcr) & RCC_PLLNCR_PLLRDY) == 0U) {
		if (get_timer(start) > PLLRDY_TIMEOUT) {
			ERROR("PLL%d start failed @ 0x%x: 0x%x\n",
			      pll_id, pllxcr, mmio_read_32(pllxcr));
			return -ETIMEDOUT;
		}
	}

	/* Start the requested output */
	mmio_setbits_32(pllxcr, output << RCC_PLLNCR_DIVEN_SHIFT);

	return 0;
}

static int stm32mp1_pll_stop(struct stm32mp1_clk_priv *priv,
			     enum stm32mp1_pll_id pll_id)
{
	const struct stm32mp1_clk_pll *pll = priv->data->pll;
	uint32_t pllxcr = priv->base + pll[pll_id].pllxcr;
	unsigned long start;

	/* Stop all output */
	mmio_clrbits_32(pllxcr, RCC_PLLNCR_DIVPEN | RCC_PLLNCR_DIVQEN |
			RCC_PLLNCR_DIVREN);

	/* Stop PLL */
	mmio_clrbits_32(pllxcr, RCC_PLLNCR_PLLON);

	start = get_timer(0);
	/* Wait PLL stopped */
	while ((mmio_read_32(pllxcr) & RCC_PLLNCR_PLLRDY) != 0U) {
		if (get_timer(start) > PLLRDY_TIMEOUT) {
			ERROR("PLL%d stop failed @ 0x%x: 0x%x\n",
			      pll_id, pllxcr, mmio_read_32(pllxcr));
			return -ETIMEDOUT;
		}
	}

	return 0;
}

static void stm32mp1_pll_config_output(struct stm32mp1_clk_priv *priv,
				       enum stm32mp1_pll_id pll_id,
				       uint32_t *pllcfg)
{
	const struct stm32mp1_clk_pll *pll = priv->data->pll;
	uint32_t rcc = priv->base;
	uint32_t value;

	value = (pllcfg[PLLCFG_P] << RCC_PLLNCFGR2_DIVP_SHIFT) &
		RCC_PLLNCFGR2_DIVP_MASK;
	value |= (pllcfg[PLLCFG_Q] << RCC_PLLNCFGR2_DIVQ_SHIFT) &
		 RCC_PLLNCFGR2_DIVQ_MASK;
	value |= (pllcfg[PLLCFG_R] << RCC_PLLNCFGR2_DIVR_SHIFT) &
		 RCC_PLLNCFGR2_DIVR_MASK;
	mmio_write_32(rcc + pll[pll_id].pllxcfgr2, value);
}

static int stm32mp1_pll_config(struct stm32mp1_clk_priv *priv,
			       enum stm32mp1_pll_id pll_id,
			       uint32_t *pllcfg, uint32_t fracv)
{
	const struct stm32mp1_clk_pll *pll = priv->data->pll;
	uint32_t rcc = priv->base;
	enum stm32mp1_plltype type = pll[pll_id].plltype;
	unsigned long refclk;
	uint32_t ifrge = 0;
	uint32_t src, value;

	src = mmio_read_32(priv->base + pll[pll_id].rckxselr) &
		RCC_SELR_REFCLK_SRC_MASK;

	refclk = stm32mp1_clk_get_fixed(priv, pll[pll_id].refclk[src]) /
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
	mmio_write_32(rcc + pll[pll_id].pllxcfgr1, value);

	/* Fractional configuration */
	value = 0;
	mmio_write_32(rcc + pll[pll_id].pllxfracr, value);

	value = fracv << RCC_PLLNFRACR_FRACV_SHIFT;
	mmio_write_32(rcc + pll[pll_id].pllxfracr, value);

	value |= RCC_PLLNFRACR_FRACLE;
	mmio_write_32(rcc + pll[pll_id].pllxfracr, value);

	stm32mp1_pll_config_output(priv, pll_id, pllcfg);

	return 0;
}

static void stm32mp1_pll_csg(struct stm32mp1_clk_priv *priv,
			     enum stm32mp1_pll_id pll_id,
			     uint32_t *csg)
{
	const struct stm32mp1_clk_pll *pll = priv->data->pll;
	uint32_t pllxcsg = 0;

	pllxcsg |= (csg[PLLCSG_MOD_PER] << RCC_PLLNCSGR_MOD_PER_SHIFT) &
		    RCC_PLLNCSGR_MOD_PER_MASK;

	pllxcsg |= (csg[PLLCSG_INC_STEP] << RCC_PLLNCSGR_INC_STEP_SHIFT) &
		    RCC_PLLNCSGR_INC_STEP_MASK;

	pllxcsg |= (csg[PLLCSG_SSCG_MODE] << RCC_PLLNCSGR_SSCG_MODE_SHIFT) &
		    RCC_PLLNCSGR_SSCG_MODE_MASK;

	mmio_write_32(priv->base + pll[pll_id].pllxcsgr, pllxcsg);
}

static int stm32mp1_set_clksrc(struct stm32mp1_clk_priv *priv,
			       unsigned int clksrc)
{
	uint32_t address = priv->base + (clksrc >> 4);
	unsigned long start;

	mmio_clrsetbits_32(address, RCC_SELR_SRC_MASK,
			   clksrc & RCC_SELR_SRC_MASK);

	start = get_timer(0);
	while ((mmio_read_32(address) & RCC_SELR_SRCRDY) == 0U) {
		if (get_timer(start) > CLKSRC_TIMEOUT) {
			ERROR("CLKSRC %x start failed @ 0x%x: 0x%x\n",
			      clksrc, address, mmio_read_32(address));
			return -ETIMEDOUT;
		}
	}

	return 0;
}

static int stm32mp1_set_clkdiv(unsigned int clkdiv, uint32_t address)
{
	unsigned long start;

	mmio_clrsetbits_32(address, RCC_DIVR_DIV_MASK,
			   clkdiv & RCC_DIVR_DIV_MASK);

	start = get_timer(0);
	while ((mmio_read_32(address) & RCC_DIVR_DIVRDY) == 0U) {
		if (get_timer(start) > CLKDIV_TIMEOUT) {
			ERROR("CLKDIV %x start failed @ 0x%x: 0x%x\n",
			      clkdiv, address, mmio_read_32(address));
			return -ETIMEDOUT;
		}
	}

	return 0;
}

static void stm32mp1_mco_csg(struct stm32mp1_clk_priv *priv,
			     uint32_t clksrc, uint32_t clkdiv)
{
	uint32_t address = priv->base + (clksrc >> 4);

	/*
	 * Binding clksrc :
	 *      bit15-4 offset
	 *      bit3:   disable
	 *      bit2-0: MCOSEL[2:0]
	 */
	if ((clksrc & 0x8U) != 0U) {
		mmio_clrbits_32(address, RCC_MCOCFG_MCOON);
	} else {
		mmio_clrsetbits_32(address,
				   RCC_MCOCFG_MCOSRC_MASK,
				   clksrc & RCC_MCOCFG_MCOSRC_MASK);
		mmio_clrsetbits_32(address,
				   RCC_MCOCFG_MCODIV_MASK,
				   clkdiv << RCC_MCOCFG_MCODIV_SHIFT);
		mmio_setbits_32(address, RCC_MCOCFG_MCOON);
	}
}

static void stm32mp1_set_rtcsrc(struct stm32mp1_clk_priv *priv,
				unsigned int clksrc, bool lse_css)
{
	uint32_t address = priv->base + RCC_BDCR;

	if (((mmio_read_32(address) & RCC_BDCR_RTCCKEN) == 0U) ||
	    (clksrc != (uint32_t)CLK_RTC_DISABLED)) {
		mmio_clrsetbits_32(address,
				   RCC_BDCR_RTCSRC_MASK,
				   clksrc << RCC_BDCR_RTCSRC_SHIFT);

		mmio_setbits_32(address, RCC_BDCR_RTCCKEN);
	}

	if (lse_css) {
		mmio_setbits_32(address, RCC_BDCR_LSECSSON);
	}
}

#define CNTCVL_OFF	0x008
#define CNTCVU_OFF	0x00C

static void stm32mp1_stgen_config(struct stm32mp1_clk_priv *priv)
{
	uintptr_t stgen;
	int p;
	uint32_t cntfid0;
	unsigned long rate;

	stgen = fdt_get_stgen_base();

	cntfid0 = mmio_read_32(stgen + CNTFID_OFF);
	p = stm32mp1_clk_get_parent(priv, STGEN_K);
	rate = stm32mp1_clk_get(priv, p);

	if (cntfid0 != rate) {
		unsigned long long counter;

		mmio_clrbits_32(stgen + CNTCR_OFF, CNTCR_EN);
		counter = (unsigned long long)
			mmio_read_32(stgen + CNTCVL_OFF);
		counter |= ((unsigned long long)
			    (mmio_read_32(stgen + CNTCVU_OFF))) << 32;
		counter = (counter * rate / cntfid0);
		mmio_write_32(stgen + CNTCVL_OFF, (uint32_t)counter);
		mmio_write_32(stgen + CNTCVU_OFF, (uint32_t)(counter >> 32));
		mmio_write_32(stgen + CNTFID_OFF, rate);
		mmio_setbits_32(stgen + CNTCR_OFF, CNTCR_EN);

		write_cntfrq((u_register_t)rate);

		/* Need to update timer with new frequency */
		generic_delay_timer_init();
	}
}

void stm32mp1_stgen_increment(unsigned long long offset_in_ms)
{
	uintptr_t stgen;
	unsigned long long cnt;

	stgen = fdt_get_stgen_base();

	cnt = ((unsigned long long)mmio_read_32(stgen + CNTCVU_OFF) << 32) |
		mmio_read_32(stgen + CNTCVL_OFF);

	cnt += (offset_in_ms * mmio_read_32(stgen + CNTFID_OFF)) / 1000U;

	mmio_clrbits_32(stgen + CNTCR_OFF, CNTCR_EN);
	mmio_write_32(stgen + CNTCVL_OFF, (uint32_t)cnt);
	mmio_write_32(stgen + CNTCVU_OFF, (uint32_t)(cnt >> 32));
	mmio_setbits_32(stgen + CNTCR_OFF, CNTCR_EN);
}

static void stm32mp1_pkcs_config(struct stm32mp1_clk_priv *priv, uint32_t pkcs)
{
	uint32_t address = priv->base + ((pkcs >> 4) & 0xFFFU);
	uint32_t value = pkcs & 0xFU;
	uint32_t mask = 0xFU;

	if ((pkcs & BIT(31)) != 0U) {
		mask <<= 4;
		value <<= 4;
	}

	mmio_clrsetbits_32(address, mask, value);
}

int stm32mp1_clk_init(void)
{
	struct stm32mp1_clk_priv *priv = &stm32mp1_clk_priv_data;
	uint32_t rcc = priv->base;
	unsigned int clksrc[CLKSRC_NB];
	unsigned int clkdiv[CLKDIV_NB];
	unsigned int pllcfg[_PLL_NB][PLLCFG_NB];
	int plloff[_PLL_NB];
	int ret, len;
	enum stm32mp1_pll_id i;
	bool lse_css = false;
	const uint32_t *pkcs_cell;

	/* Check status field to disable security */
	if (!fdt_get_rcc_secure_status()) {
		mmio_write_32(rcc + RCC_TZCR, 0);
	}

	ret = fdt_rcc_read_uint32_array("st,clksrc", clksrc,
					(uint32_t)CLKSRC_NB);
	if (ret < 0) {
		return -FDT_ERR_NOTFOUND;
	}

	ret = fdt_rcc_read_uint32_array("st,clkdiv", clkdiv,
					(uint32_t)CLKDIV_NB);
	if (ret < 0) {
		return -FDT_ERR_NOTFOUND;
	}

	for (i = (enum stm32mp1_pll_id)0; i < _PLL_NB; i++) {
		char name[12];

		snprintf(name, sizeof(name), "st,pll@%d", i);
		plloff[i] = fdt_rcc_subnode_offset(name);

		if (!fdt_check_node(plloff[i])) {
			continue;
		}

		ret = fdt_read_uint32_array(plloff[i], "cfg",
					    pllcfg[i], (int)PLLCFG_NB);
		if (ret < 0) {
			return -FDT_ERR_NOTFOUND;
		}
	}

	stm32mp1_mco_csg(priv, clksrc[CLKSRC_MCO1], clkdiv[CLKDIV_MCO1]);
	stm32mp1_mco_csg(priv, clksrc[CLKSRC_MCO2], clkdiv[CLKDIV_MCO2]);

	/*
	 * Switch ON oscillator found in device-tree.
	 * Note: HSI already ON after BootROM stage.
	 */
	if (priv->osc[_LSI] != 0U) {
		stm32mp1_lsi_set(rcc, 1);
	}
	if (priv->osc[_LSE] != 0U) {
		bool bypass;
		uint32_t lsedrv;

		bypass = fdt_osc_read_bool(_LSE, "st,bypass");
		lse_css = fdt_osc_read_bool(_LSE, "st,css");
		lsedrv = fdt_osc_read_uint32_default(_LSE, "st,drive",
						     LSEDRV_MEDIUM_HIGH);
		stm32mp1_lse_enable(rcc, bypass, lsedrv);
	}
	if (priv->osc[_HSE] != 0U) {
		bool bypass, css;

		bypass = fdt_osc_read_bool(_LSE, "st,bypass");
		css = fdt_osc_read_bool(_LSE, "st,css");
		stm32mp1_hse_enable(rcc, bypass, css);
	}
	/*
	 * CSI is mandatory for automatic I/O compensation (SYSCFG_CMPCR)
	 * => switch on CSI even if node is not present in device tree
	 */
	stm32mp1_csi_set(rcc, 1);

	/* Come back to HSI */
	ret = stm32mp1_set_clksrc(priv, CLK_MPU_HSI);
	if (ret != 0) {
		return ret;
	}
	ret = stm32mp1_set_clksrc(priv, CLK_AXI_HSI);
	if (ret != 0) {
		return ret;
	}

	for (i = (enum stm32mp1_pll_id)0; i < _PLL_NB; i++) {
		if (i == _PLL4)
			continue;
		ret = stm32mp1_pll_stop(priv, i);
		if (ret != 0) {
			return ret;
		}
	}

	/* Configure HSIDIV */
	if (priv->osc[_HSI] != 0U) {
		ret = stm32mp1_hsidiv(rcc, priv->osc[_HSI]);
		if (ret != 0) {
			return ret;
		}
		stm32mp1_stgen_config(priv);
	}

	/* Select DIV */
	/* No ready bit when MPUSRC != CLK_MPU_PLL1P_DIV, MPUDIV is disabled */
	mmio_write_32(rcc + RCC_MPCKDIVR,
		      clkdiv[CLKDIV_MPU] & RCC_DIVR_DIV_MASK);
	ret = stm32mp1_set_clkdiv(clkdiv[CLKDIV_AXI], rcc + RCC_AXIDIVR);
	if (ret != 0) {
		return ret;
	}
	ret = stm32mp1_set_clkdiv(clkdiv[CLKDIV_APB4], rcc + RCC_APB4DIVR);
	if (ret != 0) {
		return ret;
	}
	ret = stm32mp1_set_clkdiv(clkdiv[CLKDIV_APB5], rcc + RCC_APB5DIVR);
	if (ret != 0) {
		return ret;
	}
	ret = stm32mp1_set_clkdiv(clkdiv[CLKDIV_APB1], rcc + RCC_APB1DIVR);
	if (ret != 0) {
		return ret;
	}
	ret = stm32mp1_set_clkdiv(clkdiv[CLKDIV_APB2], rcc + RCC_APB2DIVR);
	if (ret != 0) {
		return ret;
	}
	ret = stm32mp1_set_clkdiv(clkdiv[CLKDIV_APB3], rcc + RCC_APB3DIVR);
	if (ret != 0) {
		return ret;
	}

	/* No ready bit for RTC */
	mmio_write_32(rcc + RCC_RTCDIVR,
		      clkdiv[CLKDIV_RTC] & RCC_DIVR_DIV_MASK);

	/* Configure PLLs source */
	ret = stm32mp1_set_clksrc(priv, clksrc[CLKSRC_PLL12]);
	if (ret != 0) {
		return ret;
	}
	ret = stm32mp1_set_clksrc(priv, clksrc[CLKSRC_PLL3]);
	if (ret != 0) {
		return ret;
	}

	ret = stm32mp1_set_clksrc(priv, clksrc[CLKSRC_PLL4]);
	if (ret != 0) {
		return ret;
	}

	/* Configure and start PLLs */
	for (i = (enum stm32mp1_pll_id)0; i < _PLL_NB; i++) {
		uint32_t fracv;
		uint32_t csg[PLLCSG_NB];

		if (!fdt_check_node(plloff[i])) {
			continue;
		}

		fracv = fdt_read_uint32_default(plloff[i], "frac", 0);

		ret = stm32mp1_pll_config(priv, i, pllcfg[i], fracv);
		if (ret != 0) {
			return ret;
		}
		ret = fdt_read_uint32_array(plloff[i], "csg", csg,
					    (uint32_t)PLLCSG_NB);
		if (ret == 0) {
			stm32mp1_pll_csg(priv, i, csg);
		} else if (ret != -FDT_ERR_NOTFOUND) {
			return ret;
		}

		stm32mp1_pll_start(priv, i);
	}
	/* Wait and start PLLs ouptut when ready */
	for (i = (enum stm32mp1_pll_id)0; i < _PLL_NB; i++) {
		if (!fdt_check_node(plloff[i])) {
			continue;
		}

		ret = stm32mp1_pll_output(priv, i, pllcfg[i][PLLCFG_O]);
		if (ret != 0) {
			return ret;
		}
	}
	/* Wait LSE ready before to use it */
	if (priv->osc[_LSE] != 0U) {
		stm32mp1_lse_wait(rcc);
	}

	/* Configure with expected clock source */
	ret = stm32mp1_set_clksrc(priv, clksrc[CLKSRC_MPU]);
	if (ret != 0) {
		return ret;
	}
	ret = stm32mp1_set_clksrc(priv, clksrc[CLKSRC_AXI]);
	if (ret != 0) {
		return ret;
	}
	stm32mp1_set_rtcsrc(priv, clksrc[CLKSRC_RTC], lse_css);

	/* Configure PKCK */
	pkcs_cell = fdt_rcc_read_prop("st,pkcs", &len);
	if (pkcs_cell != NULL) {
		bool ckper_disabled = false;
		uint32_t j;

		priv->pkcs_usb_value = 0;

		for (j = 0; j < ((uint32_t)len / sizeof(uint32_t)); j++) {
			uint32_t pkcs = (uint32_t)fdt32_to_cpu(pkcs_cell[j]);

			if (pkcs == (uint32_t)CLK_CKPER_DISABLED) {
				ckper_disabled = true;
				continue;
			}
			stm32mp1_pkcs_config(priv, pkcs);
		}

		/*
		 * CKPER is source for some peripheral clocks
		 * (FMC-NAND / QPSI-NOR) and switching source is allowed
		 * only if previous clock is still ON
		 * => deactivated CKPER only after switching clock
		 */
		if (ckper_disabled) {
			stm32mp1_pkcs_config(priv, CLK_CKPER_DISABLED);
		}
	}

	/* Switch OFF HSI if not found in device-tree */
	if (priv->osc[_HSI] == 0U) {
		stm32mp1_hsi_set(rcc, 0);
	}
	stm32mp1_stgen_config(priv);

	/* Software Self-Refresh mode (SSR) during DDR initilialization */
	mmio_clrsetbits_32(priv->base + RCC_DDRITFCR,
			   RCC_DDRITFCR_DDRCKMOD_MASK,
			   RCC_DDRITFCR_DDRCKMOD_SSR <<
			   RCC_DDRITFCR_DDRCKMOD_SHIFT);

	return 0;
}

static void stm32mp1_osc_clk_init(const char *name,
				  struct stm32mp1_clk_priv *priv,
				  enum stm32mp_osc_id index)
{
	uint32_t frequency;

	priv->osc[index] = 0;

	if (fdt_osc_read_freq(name, &frequency) != 0) {
		ERROR("%s frequency request failed\n", name);
		panic();
	} else {
		priv->osc[index] = frequency;
	}
}

static void stm32mp1_osc_init(void)
{
	struct stm32mp1_clk_priv *priv = &stm32mp1_clk_priv_data;
	enum stm32mp_osc_id i;

	for (i = (enum stm32mp_osc_id)0 ; i < NB_OSC; i++) {
		stm32mp1_osc_clk_init(stm32mp_osc_node_label[i], priv, i);
	}
}

int stm32mp1_clk_probe(void)
{
	struct stm32mp1_clk_priv *priv = &stm32mp1_clk_priv_data;

	priv->base = fdt_rcc_read_addr();
	if (priv->base == 0U) {
		return -EINVAL;
	}

	priv->data = &stm32mp1_data;

	if ((priv->data->gate == NULL) || (priv->data->sel == NULL) ||
	    (priv->data->pll == NULL)) {
		return -EINVAL;
	}

	stm32mp1_osc_init();

	return 0;
}
