/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <clk.h>
#include <clk_mux.h>
#include <clk_dev.h>
#include <clk_div.h>
#include <clk_pll.h>
#include <types/array_size.h>
#include <types/devgrps.h>
#include <control.h>
#include <clk_ids.h>
#include <devices.h>
#include <clocks.h>
#include <clk_soc_hfosc0.h>
#include <clk_soc_lfosc0.h>
#include <clk_fixed.h>
#include <clk_pll_16fft.h>
#include <clk_div.h>
#include <clk_pllctrl.h>


enum {
	AM62LX_FREQ_VALUE_BOARD_0_AUDIO_EXT_REFCLK0_OUT,
	AM62LX_FREQ_VALUE_BOARD_0_AUDIO_EXT_REFCLK1_OUT,
	AM62LX_FREQ_VALUE_BOARD_0_CP_GEMAC_CPTS0_RFT_CLK_OUT,
	AM62LX_FREQ_VALUE_BOARD_0_EXT_REFCLK1_OUT,
	AM62LX_FREQ_VALUE_BOARD_0_GPMC0_CLKLB_OUT,
	AM62LX_FREQ_VALUE_BOARD_0_I2C0_SCL_OUT,
	AM62LX_FREQ_VALUE_BOARD_0_I2C1_SCL_OUT,
	AM62LX_FREQ_VALUE_BOARD_0_I2C2_SCL_OUT,
	AM62LX_FREQ_VALUE_BOARD_0_I2C3_SCL_OUT,
	AM62LX_FREQ_VALUE_BOARD_0_MCASP0_ACLKR_OUT,
	AM62LX_FREQ_VALUE_BOARD_0_MCASP0_ACLKX_OUT,
	AM62LX_FREQ_VALUE_BOARD_0_MCASP0_AFSR_OUT,
	AM62LX_FREQ_VALUE_BOARD_0_MCASP0_AFSX_OUT,
	AM62LX_FREQ_VALUE_BOARD_0_MCASP1_ACLKR_OUT,
	AM62LX_FREQ_VALUE_BOARD_0_MCASP1_ACLKX_OUT,
	AM62LX_FREQ_VALUE_BOARD_0_MCASP1_AFSR_OUT,
	AM62LX_FREQ_VALUE_BOARD_0_MCASP1_AFSX_OUT,
	AM62LX_FREQ_VALUE_BOARD_0_MCASP2_ACLKR_OUT,
	AM62LX_FREQ_VALUE_BOARD_0_MCASP2_ACLKX_OUT,
	AM62LX_FREQ_VALUE_BOARD_0_MCASP2_AFSR_OUT,
	AM62LX_FREQ_VALUE_BOARD_0_MCASP2_AFSX_OUT,
	AM62LX_FREQ_VALUE_BOARD_0_MMC0_CLKLB_OUT,
	AM62LX_FREQ_VALUE_BOARD_0_MMC0_CLK_OUT,
	AM62LX_FREQ_VALUE_BOARD_0_MMC1_CLKLB_OUT,
	AM62LX_FREQ_VALUE_BOARD_0_MMC1_CLK_OUT,
	AM62LX_FREQ_VALUE_BOARD_0_MMC2_CLKLB_OUT,
	AM62LX_FREQ_VALUE_BOARD_0_MMC2_CLK_OUT,
	AM62LX_FREQ_VALUE_BOARD_0_OSPI0_DQS_OUT,
	AM62LX_FREQ_VALUE_BOARD_0_OSPI0_LBCLKO_OUT,
	AM62LX_FREQ_VALUE_BOARD_0_RMII1_REF_CLK_OUT,
	AM62LX_FREQ_VALUE_BOARD_0_RMII2_REF_CLK_OUT,
	AM62LX_FREQ_VALUE_BOARD_0_TCK_OUT,
	AM62LX_FREQ_VALUE_BOARD_0_VOUT0_EXTPCLKIN_OUT,
	AM62LX_FREQ_VALUE_BOARD_0_WKUP_EXT_REFCLK0_OUT,
	AM62LX_FREQ_VALUE_BOARD_0_WKUP_I2C0_SCL_OUT,
	AM62LX_FREQ_VALUE_CPSW_3GUSS_AM62L_MAIN_0_CPTS_GENF0,
	AM62LX_FREQ_VALUE_CPSW_3GUSS_AM62L_MAIN_0_CPTS_GENF1,
	AM62LX_FREQ_VALUE_CPSW_3GUSS_AM62L_MAIN_0_MDIO_MDCLK_O,
	AM62LX_FREQ_VALUE_DEBUGSS_K3_WRAP_CV0_MAIN_0_CSTPIU_TRACECLK,
	AM62LX_FREQ_VALUE_DMTIMER_DMC1MS_MAIN_0_TIMER_PWM,
	AM62LX_FREQ_VALUE_DMTIMER_DMC1MS_MAIN_1_TIMER_PWM,
	AM62LX_FREQ_VALUE_DMTIMER_DMC1MS_MAIN_2_TIMER_PWM,
	AM62LX_FREQ_VALUE_DMTIMER_DMC1MS_MAIN_3_TIMER_PWM,
	AM62LX_FREQ_VALUE_DMTIMER_DMC1MS_WKUP_0_TIMER_PWM,
	AM62LX_FREQ_VALUE_EMMCSD4SS_MAIN_0_EMMCSDSS_IO_CLK_O,
	AM62LX_FREQ_VALUE_EMMCSD4SS_MAIN_1_EMMCSDSS_IO_CLK_O,
	AM62LX_FREQ_VALUE_EMMCSD8SS_MAIN_0_EMMCSDSS_IO_CLK_O,
	AM62LX_FREQ_VALUE_FSS_UL_128_MAIN_0_OSPI0_OCLK_CLK,
	AM62LX_FREQ_VALUE_GPMC_MAIN_0_PO_GPMC_DEV_CLK,
	AM62LX_FREQ_VALUE_K3_DSS_NANO_MAIN_0_DPI_0_OUT_CLK,
	AM62LX_FREQ_VALUE_MCASP_MAIN_0_MCASP_ACLKR_POUT,
	AM62LX_FREQ_VALUE_MCASP_MAIN_0_MCASP_ACLKX_POUT,
	AM62LX_FREQ_VALUE_MCASP_MAIN_0_MCASP_AFSR_POUT,
	AM62LX_FREQ_VALUE_MCASP_MAIN_0_MCASP_AFSX_POUT,
	AM62LX_FREQ_VALUE_MCASP_MAIN_0_MCASP_AHCLKR_POUT,
	AM62LX_FREQ_VALUE_MCASP_MAIN_0_MCASP_AHCLKX_POUT,
	AM62LX_FREQ_VALUE_MCASP_MAIN_1_MCASP_ACLKR_POUT,
	AM62LX_FREQ_VALUE_MCASP_MAIN_1_MCASP_ACLKX_POUT,
	AM62LX_FREQ_VALUE_MCASP_MAIN_1_MCASP_AFSR_POUT,
	AM62LX_FREQ_VALUE_MCASP_MAIN_1_MCASP_AFSX_POUT,
	AM62LX_FREQ_VALUE_MCASP_MAIN_1_MCASP_AHCLKR_POUT,
	AM62LX_FREQ_VALUE_MCASP_MAIN_1_MCASP_AHCLKX_POUT,
	AM62LX_FREQ_VALUE_MCASP_MAIN_2_MCASP_ACLKR_POUT,
	AM62LX_FREQ_VALUE_MCASP_MAIN_2_MCASP_ACLKX_POUT,
	AM62LX_FREQ_VALUE_MCASP_MAIN_2_MCASP_AFSR_POUT,
	AM62LX_FREQ_VALUE_MCASP_MAIN_2_MCASP_AFSX_POUT,
	AM62LX_FREQ_VALUE_MCASP_MAIN_2_MCASP_AHCLKR_POUT,
	AM62LX_FREQ_VALUE_MCASP_MAIN_2_MCASP_AHCLKX_POUT,
	AM62LX_FREQ_VALUE_MSHSI2C_MAIN_0_PORSCL,
	AM62LX_FREQ_VALUE_MSHSI2C_MAIN_1_PORSCL,
	AM62LX_FREQ_VALUE_MSHSI2C_MAIN_2_PORSCL,
	AM62LX_FREQ_VALUE_MSHSI2C_MAIN_3_PORSCL,
	AM62LX_FREQ_VALUE_MSHSI2C_WKUP_0_PORSCL,
	AM62LX_FREQ_VALUE_PLLFRACF2_SSMOD_16FFT_MAIN_0,
	AM62LX_FREQ_VALUE_PLLFRACF2_SSMOD_16FFT_MAIN_17,
	AM62LX_FREQ_VALUE_PLLFRACF2_SSMOD_16FFT_MAIN_8,
	AM62LX_FREQ_VALUE_PLLFRACF2_SSMOD_16FFT_WKUP_0,
	AM62LX_FREQ_VALUE_RTCSS_WKUP_0_OSC_32K_CLK,
	AM62LX_FREQ_VALUE_SAM62L_A53_256KB_WRAP_MAIN_0_ARM_COREPACK_0_A53_DIVH_CLK4_OBSCLK_OUT_CLK,
	AM62LX_FREQ_VALUE_SAM62L_A53_256KB_WRAP_MAIN_0_CLKDIV_0_DIVH_CLK4_CLK_CLK,
	AM62LX_FREQ_VALUE_SAM62L_A53_256KB_WRAP_MAIN_0_CLKDIV_0_DIVP_CLK1_CLK_CLK,
	AM62LX_FREQ_VALUE_SPI_MAIN_0_IO_CLKSPIO_CLK,
	AM62LX_FREQ_VALUE_SPI_MAIN_1_IO_CLKSPIO_CLK,
	AM62LX_FREQ_VALUE_SPI_MAIN_2_IO_CLKSPIO_CLK,
	AM62LX_FREQ_VALUE_SPI_MAIN_3_IO_CLKSPIO_CLK,
	AM62LX_FREQ_VALUE_WIZ16B8M4CDT3_MAIN_0_IP1_PPI_M_RXCLKESC_CLK,
	AM62LX_FREQ_VALUE_WIZ16B8M4CDT3_MAIN_0_IP1_PPI_TXBYTECLKHS_CL_CLK,
	AM62LX_FREQ_VALUE_COUNT,
};

enum {
	AM62LX_FREQ_RANGE_ANY,
	AM62LX_FREQ_RANGE_GLUELOGIC_RCOSC_CLKOUT,
	AM62LX_FREQ_RANGE_GLUELOGIC_RCOSC_CLK_1P0V_97P65K,
	AM62LX_FREQ_RANGE_VCO_PLLFRACF2_SSMOD_16FFT_MAIN_0,
	AM62LX_FREQ_RANGE_VCO_IN_PLLFRACF2_SSMOD_16FFT_MAIN_0,
	AM62LX_FREQ_RANGE_ID_MAX,
};

enum {
	PLL_ENTRY_WKUP_24MHZ_1200MHZ,
	PLL_ENTRY_WKUP_25MHZ_1200MHZ,
	PLL_ENTRY_WKUP_26MHZ_1200MHZ_F24BIT,
	PLL_ENTRY_MAIN_24MHZ_1000MHZ_F24BIT,
	PLL_ENTRY_MAIN_25MHZ_1000MHZ,
	PLL_ENTRY_MAIN_26MHZ_1000MHZ_F24BIT,
	PLL_ENTRY_ARM0_24MHZ_2500MHZ_F24BIT,
	PLL_ENTRY_ARM0_25MHZ_2500MHZ,
	PLL_ENTRY_ARM0_26MHZ_2500MHZ_F24BIT,
	PLL_ENTRY_DSS_24MHZ_1800MHZ,
	PLL_ENTRY_DSS_25MHZ_1800MHZ,
	PLL_ENTRY_DSS_26MHZ_1800MHZ_F24BIT,
};

enum {
	NO_DEFAULT_FREQ,
	FREQ_PLLFRACF2_SSMOD_16FFT_MAIN_0_DEFAULT,
	FREQ_PLLFRACF2_SSMOD_16FFT_MAIN_17_DEFAULT,
	FREQ_PLLFRACF2_SSMOD_16FFT_MAIN_8_DEFAULT,
	FREQ_PLLFRACF2_SSMOD_16FFT_WKUP_0_DEFAULT,
};

uint32_t soc_clock_values[AM62LX_FREQ_VALUE_COUNT];

const struct clk_range soc_clock_ranges[AM62LX_FREQ_RANGE_ID_MAX] = {
	CLK_RANGE(AM62LX_FREQ_RANGE_ANY, 0, ULONG_MAX),
	CLK_RANGE(AM62LX_FREQ_RANGE_GLUELOGIC_RCOSC_CLKOUT, 12500000, 12500000),
	CLK_RANGE(AM62LX_FREQ_RANGE_GLUELOGIC_RCOSC_CLK_1P0V_97P65K, 97656, 97656),
	CLK_RANGE(AM62LX_FREQ_RANGE_VCO_IN_PLLFRACF2_SSMOD_16FFT_MAIN_0, 5000000, 1200000000),
	CLK_RANGE(AM62LX_FREQ_RANGE_VCO_PLLFRACF2_SSMOD_16FFT_MAIN_0, 800000000, 3200000000),
};

const struct clk_default soc_clock_freq_defaults[5] = {
	CLK_DEFAULT(FREQ_PLLFRACF2_SSMOD_16FFT_MAIN_0_DEFAULT,
		    1000000000UL, 1000000000UL, 1000000000UL),
	CLK_DEFAULT(FREQ_PLLFRACF2_SSMOD_16FFT_MAIN_17_DEFAULT,
		    1800000000UL, 1800000000UL, 1800000000UL),
	CLK_DEFAULT(FREQ_PLLFRACF2_SSMOD_16FFT_MAIN_8_DEFAULT,
		    2500000000UL, 2500000000UL, 2500000000UL),
	CLK_DEFAULT(FREQ_PLLFRACF2_SSMOD_16FFT_WKUP_0_DEFAULT,
		    1200000000UL, 1200000000UL, 1200000000UL),
};

const struct pll_table_entry soc_pll_table[12] = {
	[PLL_ENTRY_WKUP_24MHZ_1200MHZ] = {
		.freq_min_hz = 1200000000U,
		.freq_max_hz = 1200000000U,
		.plld = 1U,
		.pllm = 100U,
		.pllfm = 0U,
		.clkod = 2U,
	},
	[PLL_ENTRY_WKUP_25MHZ_1200MHZ] = {
		.freq_min_hz = 1200000000U,
		.freq_max_hz = 1200000000U,
		.plld = 1U,
		.pllm = 96U,
		.pllfm = 0U,
		.clkod = 2U,
	},
	[PLL_ENTRY_WKUP_26MHZ_1200MHZ_F24BIT] = {
		.freq_min_hz = 1200000000U,
		.freq_max_hz = 1200000001U,
		.plld = 1U,
		.pllm = 92U,
		.pllfm = 5162221U,
		.clkod = 2U,
	},
	[PLL_ENTRY_MAIN_24MHZ_1000MHZ_F24BIT] = {
		.freq_min_hz = 1000000000U,
		.freq_max_hz = 1000000000U,
		.plld = 1U,
		.pllm = 83U,
		.pllfm = 5592406U,
		.clkod = 2U,
	},
	[PLL_ENTRY_MAIN_25MHZ_1000MHZ] = {
		.freq_min_hz = 1000000000U,
		.freq_max_hz = 1000000000U,
		.plld = 1U,
		.pllm = 80U,
		.pllfm = 0U,
		.clkod = 2U,
	},
	[PLL_ENTRY_MAIN_26MHZ_1000MHZ_F24BIT] = {
		.freq_min_hz = 1000000000U,
		.freq_max_hz = 1000000000U,
		.plld = 1U,
		.pllm = 76U,
		.pllfm = 15486661U,
		.clkod = 2U,
	},
	[PLL_ENTRY_ARM0_24MHZ_2500MHZ_F24BIT] = {
		.freq_min_hz = 2500000000U,
		.freq_max_hz = 2500000000U,
		.plld = 1U,
		.pllm = 104U,
		.pllfm = 2796203U,
		.clkod = 1U,
	},
	[PLL_ENTRY_ARM0_25MHZ_2500MHZ] = {
		.freq_min_hz = 2500000000U,
		.freq_max_hz = 2500000000U,
		.plld = 1U,
		.pllm = 100U,
		.pllfm = 0U,
		.clkod = 1U,
	},
	[PLL_ENTRY_ARM0_26MHZ_2500MHZ_F24BIT] = {
		.freq_min_hz = 2500000000U,
		.freq_max_hz = 2500000001U,
		.plld = 1U,
		.pllm = 96U,
		.pllfm = 2581111U,
		.clkod = 1U,
	},
	[PLL_ENTRY_DSS_24MHZ_1800MHZ] = {
		.freq_min_hz = 1800000000U,
		.freq_max_hz = 1800000000U,
		.plld = 1U,
		.pllm = 75U,
		.pllfm = 0U,
		.clkod = 1U,
	},
	[PLL_ENTRY_DSS_25MHZ_1800MHZ] = {
		.freq_min_hz = 1800000000U,
		.freq_max_hz = 1800000000U,
		.plld = 1U,
		.pllm = 72U,
		.pllfm = 0U,
		.clkod = 1U,
	},
	[PLL_ENTRY_DSS_26MHZ_1800MHZ_F24BIT] = {
		.freq_min_hz = 1800000000U,
		.freq_max_hz = 1800000001U,
		.plld = 1U,
		.pllm = 69U,
		.pllfm = 3871666U,
		.clkod = 1U,
	},
};

static const uint8_t pllfracf2_ssmod_16fft_main_0_entries[4] = {
	PLL_ENTRY_MAIN_24MHZ_1000MHZ_F24BIT,
	PLL_ENTRY_MAIN_25MHZ_1000MHZ,
	PLL_ENTRY_MAIN_26MHZ_1000MHZ_F24BIT,
	PLL_TABLE_LAST,
};
static const uint8_t pllfracf2_ssmod_16fft_main_17_entries[4] = {
	PLL_ENTRY_DSS_24MHZ_1800MHZ,
	PLL_ENTRY_DSS_25MHZ_1800MHZ,
	PLL_ENTRY_DSS_26MHZ_1800MHZ_F24BIT,
	PLL_TABLE_LAST,
};
static const uint8_t pllfracf2_ssmod_16fft_main_8_entries[4] = {
	PLL_ENTRY_ARM0_24MHZ_2500MHZ_F24BIT,
	PLL_ENTRY_ARM0_25MHZ_2500MHZ,
	PLL_ENTRY_ARM0_26MHZ_2500MHZ_F24BIT,
	PLL_TABLE_LAST,
};
static const uint8_t pllfracf2_ssmod_16fft_wkup_0_entries[4] = {
	PLL_ENTRY_WKUP_24MHZ_1200MHZ,
	PLL_ENTRY_WKUP_25MHZ_1200MHZ,
	PLL_ENTRY_WKUP_26MHZ_1200MHZ_F24BIT,
	PLL_TABLE_LAST,
};

static const struct clk_parent clk_ADC0_CLKSEL_parents[4] = {
	{
		CLK_AM62LX_GLUELOGIC_HFOSC0_CLK,
		1,
	},
	{
		CLK_AM62LX_SAM62_PLL_CTRL_WRAP_WKUP_0_CHIP_DIV1_CLK_CLK,
		12,
	},
	{
		CLK_AM62LX_POSTDIV4_16FF_WKUP_0_HSDIVOUT8_CLK,
		1,
	},
	{
		CLK_AM62LX_BOARD_0_EXT_REFCLK1_OUT,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_ADC0_CLKSEL_out0 = {
	.data_mux = {
		.parents = clk_ADC0_CLKSEL_parents,
		.n = ARRAY_SIZE(clk_ADC0_CLKSEL_parents),
	},
	.reg = 0x09100000 + 430080,
	.bit = 0,
};
static const struct clk_parent clk_AUDIO_REFCLKn_out0_parents[8] = {
	{
		CLK_AM62LX_MCASP_MAIN_0_MCASP_AHCLKR_POUT,
		1,
	},
	{
		CLK_AM62LX_MCASP_MAIN_1_MCASP_AHCLKR_POUT,
		1,
	},
	{
		CLK_AM62LX_MCASP_MAIN_2_MCASP_AHCLKR_POUT,
		1,
	},
	{
		CLK_AM62LX_MCASP_MAIN_0_MCASP_AHCLKX_POUT,
		1,
	},
	{
		CLK_AM62LX_MCASP_MAIN_1_MCASP_AHCLKX_POUT,
		1,
	},
	{
		CLK_AM62LX_MCASP_MAIN_2_MCASP_AHCLKX_POUT,
		1,
	},
	{
		CLK_AM62LX_POSTDIV4_16FF_MAIN_0_HSDIVOUT8_CLK,
		1,
	},
	{
		CLK_AM62LX_HSDIV4_16FFT_WKUP_0_HSDIVOUT1_CLK,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_AUDIO_REFCLKn_out0 = {
	.data_mux = {
		.parents = clk_AUDIO_REFCLKn_out0_parents,
		.n = ARRAY_SIZE(clk_AUDIO_REFCLKn_out0_parents),
	},
	.reg = 0x09100000 + 294912,
	.bit = 0,
};
static const struct clk_data_mux_reg clk_data_AUDIO_REFCLKn_out1 = {
	.data_mux = {
		.parents = clk_AUDIO_REFCLKn_out0_parents,
		.n = ARRAY_SIZE(clk_AUDIO_REFCLKn_out0_parents),
	},
	.reg = 0x09100000 + 294928,
	.bit = 0,
};
static const struct clk_parent clk_CLKOUT0_CTRL_parents[2] = {
	{
		CLK_AM62LX_POSTDIV4_16FF_MAIN_0_HSDIVOUT6_CLK,
		5,
	},
	{
		CLK_AM62LX_POSTDIV4_16FF_MAIN_0_HSDIVOUT6_CLK,
		10,
	},
};
static const struct clk_data_mux_reg clk_data_CLKOUT0_CTRL_out0 = {
	.data_mux = {
		.parents = clk_CLKOUT0_CTRL_parents,
		.n = ARRAY_SIZE(clk_CLKOUT0_CTRL_parents),
	},
	.reg = 0x09100000 + 40960,
	.bit = 0,
};
static const struct clk_parent clk_CLK_32K_RC_SEL_parents[4] = {
	{
		CLK_AM62LX_GLUELOGIC_RCOSC_CLK_1P0V_97P65K,
		3,
	},
	{
		CLK_AM62LX_HSDIV0_16FFT_WKUP_32KHZ_GEN_0_HSDIVOUT0_CLK,
		8,
	},
	{
		CLK_AM62LX_GLUELOGIC_RCOSC_CLK_1P0V_97P65K,
		3,
	},
	{
		CLK_AM62LX_RTCSS_WKUP_0_OSC_32K_CLK,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_CLK_32K_RC_SEL_out0 = {
	.data_mux = {
		.parents = clk_CLK_32K_RC_SEL_parents,
		.n = ARRAY_SIZE(clk_CLK_32K_RC_SEL_parents),
	},
	.reg = 0x43020000 + 256,
	.bit = 0,
};
static const struct clk_parent clk_MAIN_CP_GEMAC_CPTS_CLK_SEL_parents[8] = {
	{
		CLK_AM62LX_POSTDIV4_16FF_WKUP_0_HSDIVOUT7_CLK,
		1,
	},
	{
		CLK_AM62LX_POSTDIV4_16FF_MAIN_0_HSDIVOUT6_CLK,
		1,
	},
	{
		CLK_AM62LX_BOARD_0_CP_GEMAC_CPTS0_RFT_CLK_OUT,
		1,
	},
	{
		CLK_AM62LX_CLK_32K_RC_SEL_OUT0,
		1,
	},
	{
		CLK_AM62LX_BOARD_0_WKUP_EXT_REFCLK0_OUT,
		1,
	},
	{
		CLK_AM62LX_BOARD_0_EXT_REFCLK1_OUT,
		1,
	},
	{
		CLK_AM62LX_SAM62_PLL_CTRL_WRAP_WKUP_0_CHIP_DIV1_CLK_CLK,
		1,
	},
	{
		CLK_AM62LX_HSDIV4_16FFT_MAIN_0_HSDIVOUT0_CLK,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_MAIN_CP_GEMAC_CPTS_CLK_SEL_out0 = {
	.data_mux = {
		.parents = clk_MAIN_CP_GEMAC_CPTS_CLK_SEL_parents,
		.n = ARRAY_SIZE(clk_MAIN_CP_GEMAC_CPTS_CLK_SEL_parents),
	},
	.reg = 0x09100000 + 253952,
	.bit = 0,
};
static const struct clk_parent clk_MAIN_DPHYTX_REFCLK_parents[2] = {
	{
		CLK_AM62LX_GLUELOGIC_HFOSC0_CLK,
		1,
	},
	{
		CLK_AM62LX_POSTDIV4_16FF_MAIN_0_HSDIVOUT8_CLK,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_MAIN_DPHYTX_REFCLK_out0 = {
	.data_mux = {
		.parents = clk_MAIN_DPHYTX_REFCLK_parents,
		.n = ARRAY_SIZE(clk_MAIN_DPHYTX_REFCLK_parents),
	},
	.reg = 0x09100000 + 405504,
	.bit = 0,
};
static const struct clk_parent clk_MAIN_DSS_DPI0_parents[2] = {
	{
		CLK_AM62LX_HSDIV0_16FFT_MAIN_17_HSDIVOUT0_CLK,
		1,
	},
	{
		CLK_AM62LX_BOARD_0_VOUT0_EXTPCLKIN_OUT,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_MAIN_DSS_DPI0_out0 = {
	.data_mux = {
		.parents = clk_MAIN_DSS_DPI0_parents,
		.n = ARRAY_SIZE(clk_MAIN_DSS_DPI0_parents),
	},
	.reg = 0x09100000 + 372736,
	.bit = 0,
};
static const struct clk_parent clk_MAIN_EMMCSD0_IO_CLKLB_SEL_parents[2] = {
	{
		CLK_AM62LX_BOARD_0_MMC0_CLKLB_OUT,
		1,
	},
	{
		CLK_AM62LX_BOARD_0_MMC0_CLK_OUT,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_MAIN_EMMCSD0_IO_CLKLB_SEL_out0 = {
	.data_mux = {
		.parents = clk_MAIN_EMMCSD0_IO_CLKLB_SEL_parents,
		.n = ARRAY_SIZE(clk_MAIN_EMMCSD0_IO_CLKLB_SEL_parents),
	},
	.reg = 0x09180000 + 45056,
	.bit = 0,
};
static const struct clk_parent clk_MAIN_EMMCSD0_REFCLK_SEL_parents[2] = {
	{
		CLK_AM62LX_POSTDIV4_16FF_MAIN_0_HSDIVOUT5_CLK,
		1,
	},
	{
		CLK_AM62LX_POSTDIV4_16FF_WKUP_0_HSDIVOUT9_CLK,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_MAIN_EMMCSD0_REFCLK_SEL_out0 = {
	.data_mux = {
		.parents = clk_MAIN_EMMCSD0_REFCLK_SEL_parents,
		.n = ARRAY_SIZE(clk_MAIN_EMMCSD0_REFCLK_SEL_parents),
	},
	.reg = 0x09100000 + 45056,
	.bit = 0,
};
static const struct clk_parent clk_MAIN_EMMCSD1_IO_CLKLB_SEL_parents[2] = {
	{
		CLK_AM62LX_BOARD_0_MMC1_CLKLB_OUT,
		1,
	},
	{
		CLK_AM62LX_BOARD_0_MMC1_CLK_OUT,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_MAIN_EMMCSD1_IO_CLKLB_SEL_out0 = {
	.data_mux = {
		.parents = clk_MAIN_EMMCSD1_IO_CLKLB_SEL_parents,
		.n = ARRAY_SIZE(clk_MAIN_EMMCSD1_IO_CLKLB_SEL_parents),
	},
	.reg = 0x09180000 + 49152,
	.bit = 0,
};
static const struct clk_data_mux_reg clk_data_MAIN_EMMCSD1_REFCLK_SEL_out0 = {
	.data_mux = {
		.parents = clk_MAIN_EMMCSD0_REFCLK_SEL_parents,
		.n = ARRAY_SIZE(clk_MAIN_EMMCSD0_REFCLK_SEL_parents),
	},
	.reg = 0x09100000 + 49152,
	.bit = 0,
};
static const struct clk_parent clk_MAIN_EMMCSD2_IO_CLKLB_SEL_parents[2] = {
	{
		CLK_AM62LX_BOARD_0_MMC2_CLKLB_OUT,
		1,
	},
	{
		CLK_AM62LX_BOARD_0_MMC2_CLK_OUT,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_MAIN_EMMCSD2_IO_CLKLB_SEL_out0 = {
	.data_mux = {
		.parents = clk_MAIN_EMMCSD2_IO_CLKLB_SEL_parents,
		.n = ARRAY_SIZE(clk_MAIN_EMMCSD2_IO_CLKLB_SEL_parents),
	},
	.reg = 0x09180000 + 53248,
	.bit = 0,
};
static const struct clk_data_mux_reg clk_data_MAIN_EMMCSD2_REFCLK_SEL_out0 = {
	.data_mux = {
		.parents = clk_MAIN_EMMCSD0_REFCLK_SEL_parents,
		.n = ARRAY_SIZE(clk_MAIN_EMMCSD0_REFCLK_SEL_parents),
	},
	.reg = 0x09100000 + 53248,
	.bit = 0,
};
static const struct clk_parent clk_MAIN_GPMC_FCLK_SEL_parents[2] = {
	{
		CLK_AM62LX_HSDIV4_16FFT_MAIN_0_HSDIVOUT3_CLK,
		1,
	},
	{
		CLK_AM62LX_HSDIV4_16FFT_WKUP_0_HSDIVOUT3_CLK,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_MAIN_GPMC_FCLK_SEL_out0 = {
	.data_mux = {
		.parents = clk_MAIN_GPMC_FCLK_SEL_parents,
		.n = ARRAY_SIZE(clk_MAIN_GPMC_FCLK_SEL_parents),
	},
	.reg = 0x09100000 + 81920,
	.bit = 0,
};
static const struct clk_parent clk_MAIN_MCANn_CLK_SEL_out0_parents[4] = {
	{
		CLK_AM62LX_HSDIV4_16FFT_MAIN_0_HSDIVOUT4_CLK,
		1,
	},
	{
		CLK_AM62LX_BOARD_0_WKUP_EXT_REFCLK0_OUT,
		1,
	},
	{
		CLK_AM62LX_BOARD_0_EXT_REFCLK1_OUT,
		1,
	},
	{
		CLK_AM62LX_GLUELOGIC_HFOSC0_CLK,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_MAIN_MCANn_CLK_SEL_out0 = {
	.data_mux = {
		.parents = clk_MAIN_MCANn_CLK_SEL_out0_parents,
		.n = ARRAY_SIZE(clk_MAIN_MCANn_CLK_SEL_out0_parents),
	},
	.reg = 0x09100000 + 262144,
	.bit = 0,
};
static const struct clk_data_mux_reg clk_data_MAIN_MCANn_CLK_SEL_out1 = {
	.data_mux = {
		.parents = clk_MAIN_MCANn_CLK_SEL_out0_parents,
		.n = ARRAY_SIZE(clk_MAIN_MCANn_CLK_SEL_out0_parents),
	},
	.reg = 0x09100000 + 266240,
	.bit = 0,
};
static const struct clk_data_mux_reg clk_data_MAIN_MCANn_CLK_SEL_out2 = {
	.data_mux = {
		.parents = clk_MAIN_MCANn_CLK_SEL_out0_parents,
		.n = ARRAY_SIZE(clk_MAIN_MCANn_CLK_SEL_out0_parents),
	},
	.reg = 0x09100000 + 270336,
	.bit = 0,
};
static const struct clk_parent clk_MAIN_OBSCLK0_MUX_SEL_parents[16] = {
	{
		CLK_AM62LX_HSDIV4_16FFT_MAIN_0_HSDIVOUT0_CLK,
		1,
	},
	{
		0,
		1,
	},
	{
		0,
		1,
	},
	{
		CLK_AM62LX_SAM62L_A53_256KB_WRAP_MAIN_0_ARM_COREPACK_0_A53_DIVH_CLK4_OBSCLK_OUT_CLK,
		1,
	},
	{
		CLK_AM62LX_HSDIV4_16FFT_MAIN_0_HSDIVOUT2_CLK,
		1,
	},
	{
		CLK_AM62LX_GLUELOGIC_RCOSC_CLKOUT,
		1,
	},
	{
		CLK_AM62LX_HSDIV0_16FFT_WKUP_32KHZ_GEN_0_HSDIVOUT0_CLK,
		8,
	},
	{
		CLK_AM62LX_HSDIV4_16FFT_WKUP_0_HSDIVOUT0_CLK,
		1,
	},
	{
		CLK_AM62LX_GLUELOGIC_HFOSC0_CLK,
		1,
	},
	{
		CLK_AM62LX_CPSW_3GUSS_AM62L_MAIN_0_CPTS_GENF0,
		1,
	},
	{
		CLK_AM62LX_CPSW_3GUSS_AM62L_MAIN_0_CPTS_GENF1,
		1,
	},
	{
		CLK_AM62LX_HSDIV0_16FFT_MAIN_17_HSDIVOUT0_CLK,
		1,
	},
	{
		CLK_AM62LX_GLUELOGIC_RCOSC_CLK_1P0V_97P65K,
		3,
	},
	{
		0,
		1,
	},
	{
		0,
		1,
	},
	{
		0,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_MAIN_OBSCLK0_MUX_SEL_out0 = {
	.data_mux = {
		.parents = clk_MAIN_OBSCLK0_MUX_SEL_parents,
		.n = ARRAY_SIZE(clk_MAIN_OBSCLK0_MUX_SEL_parents),
	},
	.reg = 0x09100000 + 36864,
	.bit = 0,
};
static const struct clk_data_div_reg_go clk_data_MAIN_OBSCLK_DIV_out0 = {
	.data_div = {
		.n = 256,
	},
	.reg = 0x09100000 + 36880,
	.bit = 8,
	.go = 16,
};
static const struct clk_parent clk_MAIN_OBSCLK_OUTMUX_SEL_parents[2] = {
	{
		CLK_AM62LX_MAIN_OBSCLK_DIV_OUT0,
		1,
	},
	{
		CLK_AM62LX_GLUELOGIC_HFOSC0_CLK,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_MAIN_OBSCLK_OUTMUX_SEL_out0 = {
	.data_mux = {
		.parents = clk_MAIN_OBSCLK_OUTMUX_SEL_parents,
		.n = ARRAY_SIZE(clk_MAIN_OBSCLK_OUTMUX_SEL_parents),
	},
	.reg = 0x09100000 + 36864,
	.bit = 24,
};
static const struct clk_parent clk_MAIN_OSPI_LOOPBACK_CLK_SEL_parents[2] = {
	{
		CLK_AM62LX_BOARD_0_OSPI0_DQS_OUT,
		1,
	},
	{
		CLK_AM62LX_BOARD_0_OSPI0_LBCLKO_OUT,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_MAIN_OSPI_LOOPBACK_CLK_SEL_out0 = {
	.data_mux = {
		.parents = clk_MAIN_OSPI_LOOPBACK_CLK_SEL_parents,
		.n = ARRAY_SIZE(clk_MAIN_OSPI_LOOPBACK_CLK_SEL_parents),
	},
	.reg = 0x09180000 + 69632,
	.bit = 4,
};
static const struct clk_parent clk_MAIN_OSPI_REF_CLK_SEL_parents[2] = {
	{
		CLK_AM62LX_POSTDIV4_16FF_MAIN_0_HSDIVOUT7_CLK,
		1,
	},
	{
		CLK_AM62LX_HSDIV4_16FFT_WKUP_0_HSDIVOUT4_CLK,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_MAIN_OSPI_REF_CLK_SEL_out0 = {
	.data_mux = {
		.parents = clk_MAIN_OSPI_REF_CLK_SEL_parents,
		.n = ARRAY_SIZE(clk_MAIN_OSPI_REF_CLK_SEL_parents),
	},
	.reg = 0x09100000 + 69632,
	.bit = 0,
};
static const struct clk_parent clk_MAIN_TIMER1_CASCADE_parents[2] = {
	{
		CLK_AM62LX_MAIN_TIMERCLKN_SEL_OUT1,
		1,
	},
	{
		CLK_AM62LX_DMTIMER_DMC1MS_MAIN_0_TIMER_PWM,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_MAIN_TIMER1_CASCADE_out0 = {
	.data_mux = {
		.parents = clk_MAIN_TIMER1_CASCADE_parents,
		.n = ARRAY_SIZE(clk_MAIN_TIMER1_CASCADE_parents),
	},
	.reg = 0x09180000 + 90112,
	.bit = 8,
};
static const struct clk_parent clk_MAIN_TIMER3_CASCADE_parents[2] = {
	{
		CLK_AM62LX_MAIN_TIMERCLKN_SEL_OUT3,
		1,
	},
	{
		CLK_AM62LX_DMTIMER_DMC1MS_MAIN_2_TIMER_PWM,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_MAIN_TIMER3_CASCADE_out0 = {
	.data_mux = {
		.parents = clk_MAIN_TIMER3_CASCADE_parents,
		.n = ARRAY_SIZE(clk_MAIN_TIMER3_CASCADE_parents),
	},
	.reg = 0x09180000 + 98304,
	.bit = 8,
};
static const struct clk_parent clk_MAIN_TIMERCLKn_SEL_out0_parents[16] = {
	{
		CLK_AM62LX_GLUELOGIC_HFOSC0_CLK,
		1,
	},
	{
		CLK_AM62LX_CLK_32K_RC_SEL_OUT0,
		1,
	},
	{
		CLK_AM62LX_POSTDIV4_16FF_MAIN_0_HSDIVOUT6_CLK,
		1,
	},
	{
		CLK_AM62LX_GLUELOGIC_RCOSC_CLKOUT,
		1,
	},
	{
		CLK_AM62LX_BOARD_0_WKUP_EXT_REFCLK0_OUT,
		1,
	},
	{
		CLK_AM62LX_BOARD_0_EXT_REFCLK1_OUT,
		1,
	},
	{
		0,
		1,
	},
	{
		CLK_AM62LX_BOARD_0_CP_GEMAC_CPTS0_RFT_CLK_OUT,
		1,
	},
	{
		CLK_AM62LX_HSDIV4_16FFT_WKUP_0_HSDIVOUT1_CLK,
		1,
	},
	{
		CLK_AM62LX_POSTDIV4_16FF_WKUP_0_HSDIVOUT7_CLK,
		1,
	},
	{
		CLK_AM62LX_CPSW_3GUSS_AM62L_MAIN_0_CPTS_GENF0,
		1,
	},
	{
		CLK_AM62LX_CPSW_3GUSS_AM62L_MAIN_0_CPTS_GENF1,
		1,
	},
	{
		0,
		1,
	},
	{
		0,
		1,
	},
	{
		0,
		1,
	},
	{
		0,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_MAIN_TIMERCLKn_SEL_out0 = {
	.data_mux = {
		.parents = clk_MAIN_TIMERCLKn_SEL_out0_parents,
		.n = ARRAY_SIZE(clk_MAIN_TIMERCLKn_SEL_out0_parents),
	},
	.reg = 0x09100000 + 86016,
	.bit = 0,
};
static const struct clk_data_mux_reg clk_data_MAIN_TIMERCLKn_SEL_out1 = {
	.data_mux = {
		.parents = clk_MAIN_TIMERCLKn_SEL_out0_parents,
		.n = ARRAY_SIZE(clk_MAIN_TIMERCLKn_SEL_out0_parents),
	},
	.reg = 0x09100000 + 90112,
	.bit = 0,
};
static const struct clk_data_mux_reg clk_data_MAIN_TIMERCLKn_SEL_out2 = {
	.data_mux = {
		.parents = clk_MAIN_TIMERCLKn_SEL_out0_parents,
		.n = ARRAY_SIZE(clk_MAIN_TIMERCLKn_SEL_out0_parents),
	},
	.reg = 0x09100000 + 94208,
	.bit = 0,
};
static const struct clk_data_mux_reg clk_data_MAIN_TIMERCLKn_SEL_out3 = {
	.data_mux = {
		.parents = clk_MAIN_TIMERCLKn_SEL_out0_parents,
		.n = ARRAY_SIZE(clk_MAIN_TIMERCLKn_SEL_out0_parents),
	},
	.reg = 0x09100000 + 98304,
	.bit = 0,
};
static const struct clk_data_div_reg_go clk_data_MAIN_USART_CLKDIV_out0 = {
	.data_div = {
		.n = 4,
	},
	.reg = 0x09100000 + 188416,
	.bit = 0,
	.go = 16,
};
static const struct clk_data_div_reg_go clk_data_MAIN_USART_CLKDIV_out1 = {
	.data_div = {
		.n = 4,
	},
	.reg = 0x09100000 + 192512,
	.bit = 0,
	.go = 16,
};
static const struct clk_data_div_reg_go clk_data_MAIN_USART_CLKDIV_out2 = {
	.data_div = {
		.n = 4,
	},
	.reg = 0x09100000 + 196608,
	.bit = 0,
	.go = 16,
};
static const struct clk_data_div_reg_go clk_data_MAIN_USART_CLKDIV_out3 = {
	.data_div = {
		.n = 4,
	},
	.reg = 0x09100000 + 200704,
	.bit = 0,
	.go = 16,
};
static const struct clk_data_div_reg_go clk_data_MAIN_USART_CLKDIV_out4 = {
	.data_div = {
		.n = 4,
	},
	.reg = 0x09100000 + 204800,
	.bit = 0,
	.go = 16,
};
static const struct clk_data_div_reg_go clk_data_MAIN_USART_CLKDIV_out5 = {
	.data_div = {
		.n = 4,
	},
	.reg = 0x09100000 + 208896,
	.bit = 0,
	.go = 16,
};
static const struct clk_data_div_reg_go clk_data_MAIN_USART_CLKDIV_out6 = {
	.data_div = {
		.n = 4,
	},
	.reg = 0x09100000 + 212992,
	.bit = 0,
	.go = 16,
};
static const struct clk_parent clk_MAIN_USB0_REFCLK_SEL_parents[2] = {
	{
		CLK_AM62LX_GLUELOGIC_HFOSC0_CLK,
		1,
	},
	{
		CLK_AM62LX_POSTDIV4_16FF_MAIN_0_HSDIVOUT8_CLK,
		4,
	},
};
static const struct clk_data_mux_reg clk_data_MAIN_USB0_REFCLK_SEL_out0 = {
	.data_mux = {
		.parents = clk_MAIN_USB0_REFCLK_SEL_parents,
		.n = ARRAY_SIZE(clk_MAIN_USB0_REFCLK_SEL_parents),
	},
	.reg = 0x43020000 + 20480,
	.bit = 0,
};
static const struct clk_data_mux_reg clk_data_MAIN_USB1_REFCLK_SEL_out0 = {
	.data_mux = {
		.parents = clk_MAIN_USB0_REFCLK_SEL_parents,
		.n = ARRAY_SIZE(clk_MAIN_USB0_REFCLK_SEL_parents),
	},
	.reg = 0x43020000 + 20484,
	.bit = 0,
};
static const struct clk_parent clk_MAIN_WWDTCLKn_SEL_out0_parents[4] = {
	{
		CLK_AM62LX_GLUELOGIC_HFOSC0_CLK,
		1,
	},
	{
		CLK_AM62LX_CLK_32K_RC_SEL_OUT0,
		1,
	},
	{
		CLK_AM62LX_GLUELOGIC_RCOSC_CLKOUT,
		1,
	},
	{
		CLK_AM62LX_GLUELOGIC_RCOSC_CLK_1P0V_97P65K,
		3,
	},
};
static const struct clk_data_mux_reg clk_data_MAIN_WWDTCLKn_SEL_out0 = {
	.data_mux = {
		.parents = clk_MAIN_WWDTCLKn_SEL_out0_parents,
		.n = ARRAY_SIZE(clk_MAIN_WWDTCLKn_SEL_out0_parents),
	},
	.reg = 0x09100000 + 118784,
	.bit = 0,
};
static const struct clk_data_mux_reg clk_data_MAIN_WWDTCLKn_SEL_out1 = {
	.data_mux = {
		.parents = clk_MAIN_WWDTCLKn_SEL_out0_parents,
		.n = ARRAY_SIZE(clk_MAIN_WWDTCLKn_SEL_out0_parents),
	},
	.reg = 0x09100000 + 122880,
	.bit = 0,
};
static const struct clk_parent clk_MCASPn_AHCLKSEL_AHCLKR_out0_parents[4] = {
	{
		CLK_AM62LX_BOARD_0_EXT_REFCLK1_OUT,
		1,
	},
	{
		CLK_AM62LX_GLUELOGIC_HFOSC0_CLK,
		1,
	},
	{
		CLK_AM62LX_BOARD_0_AUDIO_EXT_REFCLK0_OUT,
		1,
	},
	{
		CLK_AM62LX_BOARD_0_AUDIO_EXT_REFCLK1_OUT,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_MCASPn_AHCLKSEL_AHCLKR_out0 = {
	.data_mux = {
		.parents = clk_MCASPn_AHCLKSEL_AHCLKR_out0_parents,
		.n = ARRAY_SIZE(clk_MCASPn_AHCLKSEL_AHCLKR_out0_parents),
	},
	.reg = 0x09100000 + 299264,
	.bit = 0,
};
static const struct clk_data_mux_reg clk_data_MCASPn_AHCLKSEL_AHCLKR_out1 = {
	.data_mux = {
		.parents = clk_MCASPn_AHCLKSEL_AHCLKR_out0_parents,
		.n = ARRAY_SIZE(clk_MCASPn_AHCLKSEL_AHCLKR_out0_parents),
	},
	.reg = 0x09100000 + 303360,
	.bit = 0,
};
static const struct clk_data_mux_reg clk_data_MCASPn_AHCLKSEL_AHCLKR_out2 = {
	.data_mux = {
		.parents = clk_MCASPn_AHCLKSEL_AHCLKR_out0_parents,
		.n = ARRAY_SIZE(clk_MCASPn_AHCLKSEL_AHCLKR_out0_parents),
	},
	.reg = 0x09100000 + 307456,
	.bit = 0,
};
static const struct clk_data_mux_reg clk_data_MCASPn_AHCLKSEL_AHCLKX_out0 = {
	.data_mux = {
		.parents = clk_MCASPn_AHCLKSEL_AHCLKR_out0_parents,
		.n = ARRAY_SIZE(clk_MCASPn_AHCLKSEL_AHCLKR_out0_parents),
	},
	.reg = 0x09100000 + 299264,
	.bit = 8,
};
static const struct clk_data_mux_reg clk_data_MCASPn_AHCLKSEL_AHCLKX_out1 = {
	.data_mux = {
		.parents = clk_MCASPn_AHCLKSEL_AHCLKR_out0_parents,
		.n = ARRAY_SIZE(clk_MCASPn_AHCLKSEL_AHCLKR_out0_parents),
	},
	.reg = 0x09100000 + 303360,
	.bit = 8,
};
static const struct clk_data_mux_reg clk_data_MCASPn_AHCLKSEL_AHCLKX_out2 = {
	.data_mux = {
		.parents = clk_MCASPn_AHCLKSEL_AHCLKR_out0_parents,
		.n = ARRAY_SIZE(clk_MCASPn_AHCLKSEL_AHCLKR_out0_parents),
	},
	.reg = 0x09100000 + 307456,
	.bit = 8,
};
static const struct clk_parent clk_MCASPn_CLKSEL_AUXCLK_out0_parents[2] = {
	{
		CLK_AM62LX_POSTDIV4_16FF_MAIN_0_HSDIVOUT8_CLK,
		1,
	},
	{
		CLK_AM62LX_HSDIV4_16FFT_WKUP_0_HSDIVOUT1_CLK,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_MCASPn_CLKSEL_AUXCLK_out0 = {
	.data_mux = {
		.parents = clk_MCASPn_CLKSEL_AUXCLK_out0_parents,
		.n = ARRAY_SIZE(clk_MCASPn_CLKSEL_AUXCLK_out0_parents),
	},
	.reg = 0x09100000 + 299008,
	.bit = 0,
};
static const struct clk_data_mux_reg clk_data_MCASPn_CLKSEL_AUXCLK_out1 = {
	.data_mux = {
		.parents = clk_MCASPn_CLKSEL_AUXCLK_out0_parents,
		.n = ARRAY_SIZE(clk_MCASPn_CLKSEL_AUXCLK_out0_parents),
	},
	.reg = 0x09100000 + 303104,
	.bit = 0,
};
static const struct clk_data_mux_reg clk_data_MCASPn_CLKSEL_AUXCLK_out2 = {
	.data_mux = {
		.parents = clk_MCASPn_CLKSEL_AUXCLK_out0_parents,
		.n = ARRAY_SIZE(clk_MCASPn_CLKSEL_AUXCLK_out0_parents),
	},
	.reg = 0x09100000 + 307200,
	.bit = 0,
};
static const struct clk_parent clk_WKUP_CLKOUT_SEL_parents[8] = {
	{
		0,
		1,
	},
	{
		CLK_AM62LX_RTCSS_WKUP_0_OSC_32K_CLK,
		1,
	},
	{
		CLK_AM62LX_POSTDIV4_16FF_WKUP_0_HSDIVOUT7_CLK,
		1,
	},
	{
		CLK_AM62LX_HSDIV0_16FFT_WKUP_0_HSDIVOUT0_CLK,
		2,
	},
	{
		CLK_AM62LX_POSTDIV4_16FF_WKUP_0_HSDIVOUT8_CLK,
		1,
	},
	{
		CLK_AM62LX_CLK_32K_RC_SEL_OUT0,
		1,
	},
	{
		CLK_AM62LX_GLUELOGIC_RCOSC_CLKOUT,
		1,
	},
	{
		CLK_AM62LX_GLUELOGIC_HFOSC0_CLK,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_WKUP_CLKOUT_SEL_out0 = {
	.data_mux = {
		.parents = clk_WKUP_CLKOUT_SEL_parents,
		.n = ARRAY_SIZE(clk_WKUP_CLKOUT_SEL_parents),
	},
	.reg = 0x43020000 + 12288,
	.bit = 0,
};
static const struct clk_parent clk_WKUP_CLKOUT_SEL_IO_parents[2] = {
	{
		CLK_AM62LX_WKUP_CLKOUT_SEL_OUT0,
		1,
	},
	{
		CLK_AM62LX_GLUELOGIC_HFOSC0_CLK,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_WKUP_CLKOUT_SEL_IO_out0 = {
	.data_mux = {
		.parents = clk_WKUP_CLKOUT_SEL_IO_parents,
		.n = ARRAY_SIZE(clk_WKUP_CLKOUT_SEL_IO_parents),
	},
	.reg = 0x43020000 + 12292,
	.bit = 0,
};
static const struct clk_parent clk_WKUP_GPIO0_CLKSEL_parents[4] = {
	{
		CLK_AM62LX_SAM62_PLL_CTRL_WRAP_WKUP_0_CHIP_DIV1_CLK_CLK,
		4,
	},
	{
		CLK_AM62LX_RTCSS_WKUP_0_OSC_32K_CLK,
		1,
	},
	{
		CLK_AM62LX_GLUELOGIC_RCOSC_CLK_1P0V_97P65K,
		3,
	},
	{
		CLK_AM62LX_GLUELOGIC_RCOSC_CLKOUT,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_WKUP_GPIO0_CLKSEL_out0 = {
	.data_mux = {
		.parents = clk_WKUP_GPIO0_CLKSEL_parents,
		.n = ARRAY_SIZE(clk_WKUP_GPIO0_CLKSEL_parents),
	},
	.reg = 0x43020000 + 32768,
	.bit = 0,
};
static const struct clk_parent clk_WKUP_GTCCLK_SEL_parents[8] = {
	{
		CLK_AM62LX_POSTDIV4_16FF_WKUP_0_HSDIVOUT7_CLK,
		1,
	},
	{
		CLK_AM62LX_POSTDIV4_16FF_MAIN_0_HSDIVOUT6_CLK,
		1,
	},
	{
		CLK_AM62LX_BOARD_0_CP_GEMAC_CPTS0_RFT_CLK_OUT,
		1,
	},
	{
		0,
		1,
	},
	{
		CLK_AM62LX_BOARD_0_WKUP_EXT_REFCLK0_OUT,
		1,
	},
	{
		CLK_AM62LX_BOARD_0_EXT_REFCLK1_OUT,
		1,
	},
	{
		CLK_AM62LX_SAM62_PLL_CTRL_WRAP_WKUP_0_CHIP_DIV1_CLK_CLK,
		2,
	},
	{
		0,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_WKUP_GTCCLK_SEL_out0 = {
	.data_mux = {
		.parents = clk_WKUP_GTCCLK_SEL_parents,
		.n = ARRAY_SIZE(clk_WKUP_GTCCLK_SEL_parents),
	},
	.reg = 0x43020000 + 16384,
	.bit = 0,
};
static const struct clk_parent clk_WKUP_GTC_OUTMUX_SEL_parents[2] = {
	{
		CLK_AM62LX_WKUP_GTCCLK_SEL_OUT0,
		1,
	},
	{
		CLK_AM62LX_CLK_32K_RC_SEL_OUT0,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_WKUP_GTC_OUTMUX_SEL_out0 = {
	.data_mux = {
		.parents = clk_WKUP_GTC_OUTMUX_SEL_parents,
		.n = ARRAY_SIZE(clk_WKUP_GTC_OUTMUX_SEL_parents),
	},
	.reg = 0x43020000 + 4096,
	.bit = 0,
};
static const struct clk_parent clk_WKUP_OBSCLK_MUX_SEL_parents[16] = {
	{
		CLK_AM62LX_GLUELOGIC_RCOSC_CLKOUT,
		1,
	},
	{
		0,
		1,
	},
	{
		CLK_AM62LX_HSDIV4_16FFT_WKUP_0_HSDIVOUT0_CLK,
		1,
	},
	{
		CLK_AM62LX_HSDIV4_16FFT_WKUP_0_HSDIVOUT4_CLK,
		1,
	},
	{
		CLK_AM62LX_HSDIV4_16FFT_WKUP_0_HSDIVOUT0_CLK,
		1,
	},
	{
		CLK_AM62LX_GLUELOGIC_RCOSC_CLK_1P0V_97P65K,
		3,
	},
	{
		CLK_AM62LX_GLUELOGIC_HFOSC0_CLK,
		1,
	},
	{
		CLK_AM62LX_HSDIV0_16FFT_WKUP_32KHZ_GEN_0_HSDIVOUT0_CLK,
		8,
	},
	{
		CLK_AM62LX_HSDIV0_16FFT_WKUP_0_HSDIVOUT0_CLK,
		1,
	},
	{
		CLK_AM62LX_CLK_32K_RC_SEL_OUT0,
		1,
	},
	{
		CLK_AM62LX_POSTDIV4_16FF_WKUP_0_HSDIVOUT8_CLK,
		1,
	},
	{
		0,
		1,
	},
	{
		0,
		1,
	},
	{
		0,
		1,
	},
	{
		0,
		1,
	},
	{
		0,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_WKUP_OBSCLK_MUX_SEL_out0 = {
	.data_mux = {
		.parents = clk_WKUP_OBSCLK_MUX_SEL_parents,
		.n = ARRAY_SIZE(clk_WKUP_OBSCLK_MUX_SEL_parents),
	},
	.reg = 0x43020000 + 512,
	.bit = 0,
};
static const struct clk_parent clk_WKUP_OBSCLK_OUTMUX_SEL_parents[2] = {
	{
		CLK_AM62LX_WKUP_OBSCLK_MUX_SEL_OUT0,
		1,
	},
	{
		CLK_AM62LX_GLUELOGIC_HFOSC0_CLK,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_WKUP_OBSCLK_OUTMUX_SEL_out0 = {
	.data_mux = {
		.parents = clk_WKUP_OBSCLK_OUTMUX_SEL_parents,
		.n = ARRAY_SIZE(clk_WKUP_OBSCLK_OUTMUX_SEL_parents),
	},
	.reg = 0x43020000 + 516,
	.bit = 0,
};
static const struct clk_parent clk_WKUP_TIMER1_CASCADE_parents[2] = {
	{
		CLK_AM62LX_WKUP_TIMERCLKN_SEL_OUT1,
		1,
	},
	{
		CLK_AM62LX_DMTIMER_DMC1MS_WKUP_0_TIMER_PWM,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_WKUP_TIMER1_CASCADE_out0 = {
	.data_mux = {
		.parents = clk_WKUP_TIMER1_CASCADE_parents,
		.n = ARRAY_SIZE(clk_WKUP_TIMER1_CASCADE_parents),
	},
	.reg = 0x43030000 + 12292,
	.bit = 8,
};
static const struct clk_parent clk_WKUP_TIMERCLKn_SEL_out0_parents[8] = {
	{
		CLK_AM62LX_GLUELOGIC_HFOSC0_CLK,
		1,
	},
	{
		CLK_AM62LX_SAM62_PLL_CTRL_WRAP_WKUP_0_CHIP_DIV1_CLK_CLK,
		2,
	},
	{
		CLK_AM62LX_GLUELOGIC_RCOSC_CLKOUT,
		1,
	},
	{
		CLK_AM62LX_POSTDIV4_16FF_WKUP_0_HSDIVOUT7_CLK,
		1,
	},
	{
		CLK_AM62LX_BOARD_0_WKUP_EXT_REFCLK0_OUT,
		1,
	},
	{
		CLK_AM62LX_CLK_32K_RC_SEL_OUT0,
		1,
	},
	{
		CLK_AM62LX_CPSW_3GUSS_AM62L_MAIN_0_CPTS_GENF0,
		1,
	},
	{
		CLK_AM62LX_GLUELOGIC_RCOSC_CLK_1P0V_97P65K,
		3,
	},
};
static const struct clk_data_mux_reg clk_data_WKUP_TIMERCLKn_SEL_out0 = {
	.data_mux = {
		.parents = clk_WKUP_TIMERCLKn_SEL_out0_parents,
		.n = ARRAY_SIZE(clk_WKUP_TIMERCLKn_SEL_out0_parents),
	},
	.reg = 0x43020000 + 8192,
	.bit = 0,
};
static const struct clk_data_mux_reg clk_data_WKUP_TIMERCLKn_SEL_out1 = {
	.data_mux = {
		.parents = clk_WKUP_TIMERCLKn_SEL_out0_parents,
		.n = ARRAY_SIZE(clk_WKUP_TIMERCLKn_SEL_out0_parents),
	},
	.reg = 0x43020000 + 8196,
	.bit = 0,
};
static const struct clk_data_from_dev clk_data_board_0_AUDIO_EXT_REFCLK0_out = {
	.dev = AM62LX_DEV_BOARD0,
	.clk_idx = AM62LX_DEV_BOARD0_AUDIO_EXT_REFCLK0_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_AUDIO_EXT_REFCLK1_out = {
	.dev = AM62LX_DEV_BOARD0,
	.clk_idx = AM62LX_DEV_BOARD0_AUDIO_EXT_REFCLK1_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_CP_GEMAC_CPTS0_RFT_CLK_out = {
	.dev = AM62LX_DEV_BOARD0,
	.clk_idx = AM62LX_DEV_BOARD0_CP_GEMAC_CPTS0_RFT_CLK_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_EXT_REFCLK1_out = {
	.dev = AM62LX_DEV_BOARD0,
	.clk_idx = AM62LX_DEV_BOARD0_EXT_REFCLK1_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_GPMC0_CLKLB_out = {
	.dev = AM62LX_DEV_BOARD0,
	.clk_idx = AM62LX_DEV_BOARD0_GPMC0_CLKLB_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_I2C0_SCL_out = {
	.dev = AM62LX_DEV_BOARD0,
	.clk_idx = AM62LX_DEV_BOARD0_I2C0_SCL_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_I2C1_SCL_out = {
	.dev = AM62LX_DEV_BOARD0,
	.clk_idx = AM62LX_DEV_BOARD0_I2C1_SCL_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_I2C2_SCL_out = {
	.dev = AM62LX_DEV_BOARD0,
	.clk_idx = AM62LX_DEV_BOARD0_I2C2_SCL_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_I2C3_SCL_out = {
	.dev = AM62LX_DEV_BOARD0,
	.clk_idx = AM62LX_DEV_BOARD0_I2C3_SCL_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_MCASP0_ACLKR_out = {
	.dev = AM62LX_DEV_BOARD0,
	.clk_idx = AM62LX_DEV_BOARD0_MCASP0_ACLKR_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_MCASP0_ACLKX_out = {
	.dev = AM62LX_DEV_BOARD0,
	.clk_idx = AM62LX_DEV_BOARD0_MCASP0_ACLKX_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_MCASP0_AFSR_out = {
	.dev = AM62LX_DEV_BOARD0,
	.clk_idx = AM62LX_DEV_BOARD0_MCASP0_AFSR_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_MCASP0_AFSX_out = {
	.dev = AM62LX_DEV_BOARD0,
	.clk_idx = AM62LX_DEV_BOARD0_MCASP0_AFSX_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_MCASP1_ACLKR_out = {
	.dev = AM62LX_DEV_BOARD0,
	.clk_idx = AM62LX_DEV_BOARD0_MCASP1_ACLKR_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_MCASP1_ACLKX_out = {
	.dev = AM62LX_DEV_BOARD0,
	.clk_idx = AM62LX_DEV_BOARD0_MCASP1_ACLKX_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_MCASP1_AFSR_out = {
	.dev = AM62LX_DEV_BOARD0,
	.clk_idx = AM62LX_DEV_BOARD0_MCASP1_AFSR_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_MCASP1_AFSX_out = {
	.dev = AM62LX_DEV_BOARD0,
	.clk_idx = AM62LX_DEV_BOARD0_MCASP1_AFSX_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_MCASP2_ACLKR_out = {
	.dev = AM62LX_DEV_BOARD0,
	.clk_idx = AM62LX_DEV_BOARD0_MCASP2_ACLKR_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_MCASP2_ACLKX_out = {
	.dev = AM62LX_DEV_BOARD0,
	.clk_idx = AM62LX_DEV_BOARD0_MCASP2_ACLKX_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_MCASP2_AFSR_out = {
	.dev = AM62LX_DEV_BOARD0,
	.clk_idx = AM62LX_DEV_BOARD0_MCASP2_AFSR_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_MCASP2_AFSX_out = {
	.dev = AM62LX_DEV_BOARD0,
	.clk_idx = AM62LX_DEV_BOARD0_MCASP2_AFSX_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_MMC0_CLKLB_out = {
	.dev = AM62LX_DEV_BOARD0,
	.clk_idx = AM62LX_DEV_BOARD0_MMC0_CLKLB_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_MMC0_CLK_out = {
	.dev = AM62LX_DEV_BOARD0,
	.clk_idx = AM62LX_DEV_BOARD0_MMC0_CLK_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_MMC1_CLKLB_out = {
	.dev = AM62LX_DEV_BOARD0,
	.clk_idx = AM62LX_DEV_BOARD0_MMC1_CLKLB_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_MMC1_CLK_out = {
	.dev = AM62LX_DEV_BOARD0,
	.clk_idx = AM62LX_DEV_BOARD0_MMC1_CLK_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_MMC2_CLKLB_out = {
	.dev = AM62LX_DEV_BOARD0,
	.clk_idx = AM62LX_DEV_BOARD0_MMC2_CLKLB_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_MMC2_CLK_out = {
	.dev = AM62LX_DEV_BOARD0,
	.clk_idx = AM62LX_DEV_BOARD0_MMC2_CLK_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_OSPI0_DQS_out = {
	.dev = AM62LX_DEV_BOARD0,
	.clk_idx = AM62LX_DEV_BOARD0_OSPI0_DQS_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_OSPI0_LBCLKO_out = {
	.dev = AM62LX_DEV_BOARD0,
	.clk_idx = AM62LX_DEV_BOARD0_OSPI0_LBCLKO_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_RMII1_REF_CLK_out = {
	.dev = AM62LX_DEV_BOARD0,
	.clk_idx = AM62LX_DEV_BOARD0_RMII1_REF_CLK_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_RMII2_REF_CLK_out = {
	.dev = AM62LX_DEV_BOARD0,
	.clk_idx = AM62LX_DEV_BOARD0_RMII2_REF_CLK_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_TCK_out = {
	.dev = AM62LX_DEV_BOARD0,
	.clk_idx = AM62LX_DEV_BOARD0_TCK_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_VOUT0_EXTPCLKIN_out = {
	.dev = AM62LX_DEV_BOARD0,
	.clk_idx = AM62LX_DEV_BOARD0_VOUT0_EXTPCLKIN_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_WKUP_EXT_REFCLK0_out = {
	.dev = AM62LX_DEV_BOARD0,
	.clk_idx = AM62LX_DEV_BOARD0_WKUP_EXT_REFCLK0_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_WKUP_I2C0_SCL_out = {
	.dev = AM62LX_DEV_BOARD0,
	.clk_idx = AM62LX_DEV_BOARD0_WKUP_I2C0_SCL_OUT,
};
static const struct clk_data_from_dev clk_data_cpsw_3guss_am62l_main_0_cpts_genf0 = {
	.dev = AM62LX_DEV_CPSW0,
	.clk_idx = AM62LX_DEV_CPSW0_CPTS_GENF0,
};
static const struct clk_data_from_dev clk_data_cpsw_3guss_am62l_main_0_cpts_genf1 = {
	.dev = AM62LX_DEV_CPSW0,
	.clk_idx = AM62LX_DEV_CPSW0_CPTS_GENF1,
};
static const struct clk_data_from_dev clk_data_cpsw_3guss_am62l_main_0_mdio_mdclk_o = {
	.dev = AM62LX_DEV_CPSW0,
	.clk_idx = AM62LX_DEV_CPSW0_MDIO_MDCLK_O,
};
static const struct clk_data_from_dev clk_data_debugss_k3_wrap_cv0_main_0_cstpiu_traceclk = {
	.dev = AM62LX_DEV_DEBUGSS_WRAP0,
	.clk_idx = AM62LX_DEV_DEBUGSS_WRAP0_CSTPIU_TRACECLK,
};
static const struct clk_data_from_dev clk_data_dmtimer_dmc1ms_main_0_timer_pwm = {
	.dev = AM62LX_DEV_TIMER0,
	.clk_idx = AM62LX_DEV_TIMER0_TIMER_PWM,
};
static const struct clk_data_from_dev clk_data_dmtimer_dmc1ms_main_1_timer_pwm = {
	.dev = AM62LX_DEV_TIMER1,
	.clk_idx = AM62LX_DEV_TIMER1_TIMER_PWM,
};
static const struct clk_data_from_dev clk_data_dmtimer_dmc1ms_main_2_timer_pwm = {
	.dev = AM62LX_DEV_TIMER2,
	.clk_idx = AM62LX_DEV_TIMER2_TIMER_PWM,
};
static const struct clk_data_from_dev clk_data_dmtimer_dmc1ms_main_3_timer_pwm = {
	.dev = AM62LX_DEV_TIMER3,
	.clk_idx = AM62LX_DEV_TIMER3_TIMER_PWM,
};
static const struct clk_data_from_dev clk_data_dmtimer_dmc1ms_wkup_0_timer_pwm = {
	.dev = AM62LX_DEV_WKUP_TIMER0,
	.clk_idx = AM62LX_DEV_WKUP_TIMER0_TIMER_PWM,
};
static const struct clk_data_from_dev clk_data_emmcsd4ss_main_0_emmcsdss_io_clk_o = {
	.dev = AM62LX_DEV_MMCSD1,
	.clk_idx = AM62LX_DEV_MMCSD1_EMMCSDSS_IO_CLK_O,
};
static const struct clk_data_from_dev clk_data_emmcsd4ss_main_1_emmcsdss_io_clk_o = {
	.dev = AM62LX_DEV_MMCSD2,
	.clk_idx = AM62LX_DEV_MMCSD2_EMMCSDSS_IO_CLK_O,
};
static const struct clk_data_from_dev clk_data_emmcsd8ss_main_0_emmcsdss_io_clk_o = {
	.dev = AM62LX_DEV_MMCSD0,
	.clk_idx = AM62LX_DEV_MMCSD0_EMMCSDSS_IO_CLK_O,
};
static const struct clk_data_from_dev clk_data_fss_ul_128_main_0_ospi0_oclk_clk = {
	.dev = AM62LX_DEV_FSS0,
	.clk_idx = AM62LX_DEV_FSS0_OSPI0_OCLK_CLK,
};
static const struct clk_data_from_dev clk_data_gpmc_main_0_po_gpmc_dev_clk = {
	.dev = AM62LX_DEV_GPMC0,
	.clk_idx = AM62LX_DEV_GPMC0_PO_GPMC_DEV_CLK,
};
static const struct clk_data_div_reg clk_data_hsdiv0_16fft_main_17_hsdiv0 = {
	.data_div = {
		.n = 128,
		.default_div = 9,
	},
	.reg = 0x04060000UL + (0x1000UL * 17UL) + 0x80UL + (0x4UL * 0UL),
	.bit = 0,
};
static const struct clk_data_div_reg clk_data_hsdiv0_16fft_main_8_hsdiv0 = {
	.data_div = {
		.n = 128,
		.default_div = 2,
	},
	.reg = 0x04060000UL + (0x1000UL * 8UL) + 0x80UL + (0x4UL * 0UL),
	.bit = 0,
};
static const struct clk_data_div_reg clk_data_hsdiv0_16fft_wkup_0_hsdiv0 = {
	.data_div = {
		.n = 128,
		.default_div = 6,
	},
	.reg = 0x04040000UL + (0x1000UL * 0UL) + 0x80UL + (0x4UL * 0UL),
	.bit = 0,
};
static const struct clk_data_div_reg clk_data_hsdiv0_16fft_wkup_32khz_gen_0_hsdiv0 = {
	.data_div = {
		.n = 128,
	},
	.reg = 0x43020000UL + 0x148UL + (0x4UL * 0UL),
	.bit = 0,
};
static const struct clk_data_div_reg clk_data_hsdiv4_16fft_main_0_hsdiv0 = {
	.data_div = {
		.n = 128,
		.default_div = 4,
	},
	.reg = 0x04060000UL + (0x1000UL * 0UL) + 0x80UL + (0x4UL * 0UL),
	.bit = 0,
};
static const struct clk_data_div_reg clk_data_hsdiv4_16fft_main_0_hsdiv2 = {
	.data_div = {
		.n = 128,
		.default_div = 5,
	},
	.reg = 0x04060000UL + (0x1000UL * 0UL) + 0x80UL + (0x4UL * 2UL),
	.bit = 0,
};
static const struct clk_data_div_reg clk_data_hsdiv4_16fft_main_0_hsdiv3 = {
	.data_div = {
		.n = 128,
		.default_div = 15,
	},
	.reg = 0x04060000UL + (0x1000UL * 0UL) + 0x80UL + (0x4UL * 3UL),
	.bit = 0,
};
static const struct clk_data_div_reg clk_data_hsdiv4_16fft_main_0_hsdiv4 = {
	.data_div = {
		.n = 128,
		.default_div = 25,
	},
	.reg = 0x04060000UL + (0x1000UL * 0UL) + 0x80UL + (0x4UL * 4UL),
	.bit = 0,
};
static const struct clk_data_div_reg clk_data_hsdiv4_16fft_wkup_0_hsdiv0 = {
	.data_div = {
		.n = 128,
		.default_div = 6,
	},
	.reg = 0x04040000UL + (0x1000UL * 0UL) + 0x80UL + (0x4UL * 0UL),
	.bit = 0,
};
static const struct clk_data_div_reg clk_data_hsdiv4_16fft_wkup_0_hsdiv1 = {
	.data_div = {
		.n = 128,
		.default_div = 25,
	},
	.reg = 0x04040000UL + (0x1000UL * 0UL) + 0x80UL + (0x4UL * 1UL),
	.bit = 0,
};
static const struct clk_data_div_reg clk_data_hsdiv4_16fft_wkup_0_hsdiv2 = {
	.data_div = {
		.n = 128,
		.default_div = 25,
	},
	.reg = 0x04040000UL + (0x1000UL * 0UL) + 0x80UL + (0x4UL * 2UL),
	.bit = 0,
};
static const struct clk_data_div_reg clk_data_hsdiv4_16fft_wkup_0_hsdiv3 = {
	.data_div = {
		.n = 128,
		.default_div = 40,
	},
	.reg = 0x04040000UL + (0x1000UL * 0UL) + 0x80UL + (0x4UL * 3UL),
	.bit = 0,
};
static const struct clk_data_div_reg clk_data_hsdiv4_16fft_wkup_0_hsdiv4 = {
	.data_div = {
		.n = 128,
		.default_div = 18,
	},
	.reg = 0x04040000UL + (0x1000UL * 0UL) + 0x80UL + (0x4UL * 4UL),
	.bit = 0,
};
static const struct clk_data_from_dev clk_data_k3_dss_nano_main_0_dpi_0_out_clk = {
	.dev = AM62LX_DEV_DSS0,
	.clk_idx = AM62LX_DEV_DSS0_DPI_0_OUT_CLK,
};
static const struct clk_data_from_dev clk_data_mcasp_main_0_mcasp_aclkr_pout = {
	.dev = AM62LX_DEV_MCASP0,
	.clk_idx = AM62LX_DEV_MCASP0_MCASP_ACLKR_POUT,
};
static const struct clk_data_from_dev clk_data_mcasp_main_0_mcasp_aclkx_pout = {
	.dev = AM62LX_DEV_MCASP0,
	.clk_idx = AM62LX_DEV_MCASP0_MCASP_ACLKX_POUT,
};
static const struct clk_data_from_dev clk_data_mcasp_main_0_mcasp_afsr_pout = {
	.dev = AM62LX_DEV_MCASP0,
	.clk_idx = AM62LX_DEV_MCASP0_MCASP_AFSR_POUT,
};
static const struct clk_data_from_dev clk_data_mcasp_main_0_mcasp_afsx_pout = {
	.dev = AM62LX_DEV_MCASP0,
	.clk_idx = AM62LX_DEV_MCASP0_MCASP_AFSX_POUT,
};
static const struct clk_data_from_dev clk_data_mcasp_main_0_mcasp_ahclkr_pout = {
	.dev = AM62LX_DEV_MCASP0,
	.clk_idx = AM62LX_DEV_MCASP0_MCASP_AHCLKR_POUT,
};
static const struct clk_data_from_dev clk_data_mcasp_main_0_mcasp_ahclkx_pout = {
	.dev = AM62LX_DEV_MCASP0,
	.clk_idx = AM62LX_DEV_MCASP0_MCASP_AHCLKX_POUT,
};
static const struct clk_data_from_dev clk_data_mcasp_main_1_mcasp_aclkr_pout = {
	.dev = AM62LX_DEV_MCASP1,
	.clk_idx = AM62LX_DEV_MCASP1_MCASP_ACLKR_POUT,
};
static const struct clk_data_from_dev clk_data_mcasp_main_1_mcasp_aclkx_pout = {
	.dev = AM62LX_DEV_MCASP1,
	.clk_idx = AM62LX_DEV_MCASP1_MCASP_ACLKX_POUT,
};
static const struct clk_data_from_dev clk_data_mcasp_main_1_mcasp_afsr_pout = {
	.dev = AM62LX_DEV_MCASP1,
	.clk_idx = AM62LX_DEV_MCASP1_MCASP_AFSR_POUT,
};
static const struct clk_data_from_dev clk_data_mcasp_main_1_mcasp_afsx_pout = {
	.dev = AM62LX_DEV_MCASP1,
	.clk_idx = AM62LX_DEV_MCASP1_MCASP_AFSX_POUT,
};
static const struct clk_data_from_dev clk_data_mcasp_main_1_mcasp_ahclkr_pout = {
	.dev = AM62LX_DEV_MCASP1,
	.clk_idx = AM62LX_DEV_MCASP1_MCASP_AHCLKR_POUT,
};
static const struct clk_data_from_dev clk_data_mcasp_main_1_mcasp_ahclkx_pout = {
	.dev = AM62LX_DEV_MCASP1,
	.clk_idx = AM62LX_DEV_MCASP1_MCASP_AHCLKX_POUT,
};
static const struct clk_data_from_dev clk_data_mcasp_main_2_mcasp_aclkr_pout = {
	.dev = AM62LX_DEV_MCASP2,
	.clk_idx = AM62LX_DEV_MCASP2_MCASP_ACLKR_POUT,
};
static const struct clk_data_from_dev clk_data_mcasp_main_2_mcasp_aclkx_pout = {
	.dev = AM62LX_DEV_MCASP2,
	.clk_idx = AM62LX_DEV_MCASP2_MCASP_ACLKX_POUT,
};
static const struct clk_data_from_dev clk_data_mcasp_main_2_mcasp_afsr_pout = {
	.dev = AM62LX_DEV_MCASP2,
	.clk_idx = AM62LX_DEV_MCASP2_MCASP_AFSR_POUT,
};
static const struct clk_data_from_dev clk_data_mcasp_main_2_mcasp_afsx_pout = {
	.dev = AM62LX_DEV_MCASP2,
	.clk_idx = AM62LX_DEV_MCASP2_MCASP_AFSX_POUT,
};
static const struct clk_data_from_dev clk_data_mcasp_main_2_mcasp_ahclkr_pout = {
	.dev = AM62LX_DEV_MCASP2,
	.clk_idx = AM62LX_DEV_MCASP2_MCASP_AHCLKR_POUT,
};
static const struct clk_data_from_dev clk_data_mcasp_main_2_mcasp_ahclkx_pout = {
	.dev = AM62LX_DEV_MCASP2,
	.clk_idx = AM62LX_DEV_MCASP2_MCASP_AHCLKX_POUT,
};
static const struct clk_data_from_dev clk_data_mshsi2c_main_0_porscl = {
	.dev = AM62LX_DEV_I2C0,
	.clk_idx = AM62LX_DEV_I2C0_PORSCL,
};
static const struct clk_data_from_dev clk_data_mshsi2c_main_1_porscl = {
	.dev = AM62LX_DEV_I2C1,
	.clk_idx = AM62LX_DEV_I2C1_PORSCL,
};
static const struct clk_data_from_dev clk_data_mshsi2c_main_2_porscl = {
	.dev = AM62LX_DEV_I2C2,
	.clk_idx = AM62LX_DEV_I2C2_PORSCL,
};
static const struct clk_data_from_dev clk_data_mshsi2c_main_3_porscl = {
	.dev = AM62LX_DEV_I2C3,
	.clk_idx = AM62LX_DEV_I2C3_PORSCL,
};
static const struct clk_data_from_dev clk_data_mshsi2c_wkup_0_porscl = {
	.dev = AM62LX_DEV_WKUP_I2C0,
	.clk_idx = AM62LX_DEV_WKUP_I2C0_PORSCL,
};
static const struct clk_data_pll_16fft clk_data_pllfracf2_ssmod_16fft_main_0 = {
	.data_pll = {
		.vco_range_idx = AM62LX_FREQ_RANGE_VCO_PLLFRACF2_SSMOD_16FFT_MAIN_0,
		.vco_in_range_idx = AM62LX_FREQ_RANGE_VCO_IN_PLLFRACF2_SSMOD_16FFT_MAIN_0,
		.fractional_support = true,
		.devgrp = DEVGRP_00,
		.pll_entries = pllfracf2_ssmod_16fft_main_0_entries,
		.default_freq_idx = FREQ_PLLFRACF2_SSMOD_16FFT_MAIN_0_DEFAULT,
	},
	.idx = 0,
	.base = 0x04060000,
};
static const struct clk_data_div clk_data_pllfracf2_ssmod_16fft_main_0_postdiv = {
	.n = 49,
};
static const struct clk_data_pll_16fft clk_data_pllfracf2_ssmod_16fft_main_17 = {
	.data_pll = {
		.vco_range_idx = AM62LX_FREQ_RANGE_VCO_PLLFRACF2_SSMOD_16FFT_MAIN_0,
		.vco_in_range_idx = AM62LX_FREQ_RANGE_VCO_IN_PLLFRACF2_SSMOD_16FFT_MAIN_0,
		.fractional_support = true,
		.devgrp = DEVGRP_00,
		.pll_entries = pllfracf2_ssmod_16fft_main_17_entries,
		.default_freq_idx = FREQ_PLLFRACF2_SSMOD_16FFT_MAIN_17_DEFAULT,
	},
	.idx = 17,
	.base = 0x04060000,
};
static const struct clk_data_pll_16fft clk_data_pllfracf2_ssmod_16fft_main_8 = {
	.data_pll = {
		.vco_range_idx = AM62LX_FREQ_RANGE_VCO_PLLFRACF2_SSMOD_16FFT_MAIN_0,
		.vco_in_range_idx = AM62LX_FREQ_RANGE_VCO_IN_PLLFRACF2_SSMOD_16FFT_MAIN_0,
		.fractional_support = true,
		.devgrp = DEVGRP_00,
		.pll_entries = pllfracf2_ssmod_16fft_main_8_entries,
		.default_freq_idx = FREQ_PLLFRACF2_SSMOD_16FFT_MAIN_8_DEFAULT,
	},
	.idx = 8,
	.base = 0x04060000,
};
static const struct clk_data_pll_16fft clk_data_pllfracf2_ssmod_16fft_wkup_0 = {
	.data_pll = {
		.vco_range_idx = AM62LX_FREQ_RANGE_VCO_PLLFRACF2_SSMOD_16FFT_MAIN_0,
		.vco_in_range_idx = AM62LX_FREQ_RANGE_VCO_IN_PLLFRACF2_SSMOD_16FFT_MAIN_0,
		.fractional_support = true,
		.devgrp = DEVGRP_00,
		.pll_entries = pllfracf2_ssmod_16fft_wkup_0_entries,
		.default_freq_idx = FREQ_PLLFRACF2_SSMOD_16FFT_WKUP_0_DEFAULT,
	},
	.idx = 0,
	.base = 0x04040000,
};
static const struct clk_data_div clk_data_pllfracf2_ssmod_16fft_wkup_0_postdiv = {
	.n = 49,
};
static const struct clk_data_div_reg clk_data_postdiv4_16ff_main_0_hsdiv5 = {
	.data_div = {
		.n = 128,
		.default_div = 5,
	},
	.reg = 0x04060000UL + (0x1000UL * 0UL) + 0x80UL + (0x4UL * 5UL),
	.bit = 0,
};
static const struct clk_data_div_reg clk_data_postdiv4_16ff_main_0_hsdiv6 = {
	.data_div = {
		.n = 128,
		.default_div = 4,
	},
	.reg = 0x04060000UL + (0x1000UL * 0UL) + 0x80UL + (0x4UL * 6UL),
	.bit = 0,
};
static const struct clk_data_div_reg clk_data_postdiv4_16ff_main_0_hsdiv7 = {
	.data_div = {
		.n = 128,
		.default_div = 5,
	},
	.reg = 0x04060000UL + (0x1000UL * 0UL) + 0x80UL + (0x4UL * 7UL),
	.bit = 0,
};
static const struct clk_data_div_reg clk_data_postdiv4_16ff_main_0_hsdiv8 = {
	.data_div = {
		.n = 128,
		.default_div = 10,
	},
	.reg = 0x04060000UL + (0x1000UL * 0UL) + 0x80UL + (0x4UL * 8UL),
	.bit = 0,
};
static const struct clk_data_div_reg clk_data_postdiv4_16ff_main_0_hsdiv9 = {
	.data_div = {
		.n = 128,
		.default_div = 3,
	},
	.reg = 0x04060000UL + (0x1000UL * 0UL) + 0x80UL + (0x4UL * 9UL),
	.bit = 0,
};
static const struct clk_data_div_reg clk_data_postdiv4_16ff_wkup_0_hsdiv6 = {
	.data_div = {
		.n = 128,
		.default_div = 24,
	},
	.reg = 0x04040000UL + (0x1000UL * 0UL) + 0x80UL + (0x4UL * 6UL),
	.bit = 0,
};
static const struct clk_data_div_reg clk_data_postdiv4_16ff_wkup_0_hsdiv7 = {
	.data_div = {
		.n = 128,
		.default_div = 6,
	},
	.reg = 0x04040000UL + (0x1000UL * 0UL) + 0x80UL + (0x4UL * 7UL),
	.bit = 0,
};
static const struct clk_data_div_reg clk_data_postdiv4_16ff_wkup_0_hsdiv8 = {
	.data_div = {
		.n = 128,
		.default_div = 40,
	},
	.reg = 0x04040000UL + (0x1000UL * 0UL) + 0x80UL + (0x4UL * 8UL),
	.bit = 0,
};
static const struct clk_data_div_reg clk_data_postdiv4_16ff_wkup_0_hsdiv9 = {
	.data_div = {
		.n = 128,
		.default_div = 6,
	},
	.reg = 0x04040000UL + (0x1000UL * 0UL) + 0x80UL + (0x4UL * 9UL),
	.bit = 0,
};
static const struct clk_data_from_dev clk_data_rtcss_wkup_0_osc_32k_clk = {
	.dev = AM62LX_DEV_WKUP_RTCSS0,
	.clk_idx = AM62LX_DEV_WKUP_RTCSS0_OSC_32K_CLK,
};
static const struct clk_parent clk_sam62_pll_ctrl_wrap_wkup_0_parents[2] = {
	{
		CLK_AM62LX_GLUELOGIC_HFOSC0_CLK,
		1,
	},
	{
		CLK_AM62LX_HSDIV4_16FFT_WKUP_0_HSDIVOUT0_CLK,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_sam62_pll_ctrl_wrap_wkup_0_sysclkout_clk = {
	.data_mux = {
		.parents = clk_sam62_pll_ctrl_wrap_wkup_0_parents,
		.n = ARRAY_SIZE(clk_sam62_pll_ctrl_wrap_wkup_0_parents),
	},
	.reg = 0x00410000,
};
static const struct clk_data_div_reg clk_data_sam62_pll_ctrl_wrap_wkup_0_chip_div1_clk_clk = {
	.data_div = {
		.n = 32,
	},
	.reg = 0x00410000 + 0x118,
	.bit = 0,
};
static const struct clk_data_div_reg clk_data_sam62_pll_ctrl_wrap_wkup_0_chip_div24_clk_clk = {
	.data_div = {
		.n = 32,
	},
	.reg = 0x00410000 + 0x11c,
	.bit = 0,
};
static const struct clk_data_from_dev
clk_data_sam62l_a53_256kb_wrap_main_0_arm_corepack_0_a53_divh_clk4_obsclk_out_clk = {
	.dev = AM62LX_DEV_COMPUTE_CLUSTER0_ARM_COREPACK_0,
	.clk_idx = AM62LX_DEV_COMPUTE_CLUSTER0_ARM_COREPACK_0_A53_DIVH_CLK4_OBSCLK_OUT_CLK,
};
static const struct clk_data_from_dev
clk_data_sam62l_a53_256kb_wrap_main_0_clkdiv_0_divh_clk4_clk_clk = {
	.dev = AM62LX_DEV_COMPUTE_CLUSTER0_CLKDIV_0,
	.clk_idx = AM62LX_DEV_COMPUTE_CLUSTER0_CLKDIV_0_DIVH_CLK4_CLK_CLK,
};
static const struct clk_data_from_dev
clk_data_sam62l_a53_256kb_wrap_main_0_clkdiv_0_divp_clk1_clk_clk = {
	.dev = AM62LX_DEV_COMPUTE_CLUSTER0_CLKDIV_0,
	.clk_idx = AM62LX_DEV_COMPUTE_CLUSTER0_CLKDIV_0_DIVP_CLK1_CLK_CLK,
};
static const struct clk_data_from_dev clk_data_spi_main_0_io_clkspio_clk = {
	.dev = AM62LX_DEV_MCSPI0,
	.clk_idx = AM62LX_DEV_MCSPI0_IO_CLKSPIO_CLK,
};
static const struct clk_data_from_dev clk_data_spi_main_1_io_clkspio_clk = {
	.dev = AM62LX_DEV_MCSPI1,
	.clk_idx = AM62LX_DEV_MCSPI1_IO_CLKSPIO_CLK,
};
static const struct clk_data_from_dev clk_data_spi_main_2_io_clkspio_clk = {
	.dev = AM62LX_DEV_MCSPI2,
	.clk_idx = AM62LX_DEV_MCSPI2_IO_CLKSPIO_CLK,
};
static const struct clk_data_from_dev clk_data_spi_main_3_io_clkspio_clk = {
	.dev = AM62LX_DEV_MCSPI3,
	.clk_idx = AM62LX_DEV_MCSPI3_IO_CLKSPIO_CLK,
};
static const struct clk_data_from_dev
clk_data_wiz16b8m4cdt3_main_0_ip1_ppi_M_RxClkEsc_clk = {
	.dev = AM62LX_DEV_DPHY_TX0,
	.clk_idx = AM62LX_DEV_DPHY_TX0_IP1_PPI_M_RXCLKESC_CLK,
};
static const struct clk_data_from_dev
clk_data_wiz16b8m4cdt3_main_0_ip1_ppi_TxByteClkHS_cl_clk = {
	.dev = AM62LX_DEV_DPHY_TX0,
	.clk_idx = AM62LX_DEV_DPHY_TX0_IP1_PPI_TXBYTECLKHS_CL_CLK,
};

const struct clk_data soc_clock_data[178] = {
	[CLK_AM62LX_GLUELOGIC_HFOSC0_CLK] = {
		.drv = &clk_drv_soc_hfosc0,
		.flags = 0,
	},
	[CLK_AM62LX_GLUELOGIC_LFOSC0_CLK] = {
		.drv = &clk_drv_soc_lfosc0,
		.flags = 0,
	},
	[CLK_AM62LX_GLUELOGIC_RCOSC_CLKOUT] = {
		.drv = &clk_drv_fixed,
		.flags = 0,
		.range_idx = AM62LX_FREQ_RANGE_GLUELOGIC_RCOSC_CLKOUT,
	},
	[CLK_AM62LX_GLUELOGIC_RCOSC_CLK_1P0V_97P65K] = {
		.drv = &clk_drv_fixed,
		.flags = 0,
		.range_idx = AM62LX_FREQ_RANGE_GLUELOGIC_RCOSC_CLK_1P0V_97P65K,
	},
	[CLK_AM62LX_BOARD_0_AUDIO_EXT_REFCLK0_OUT] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data = &clk_data_board_0_AUDIO_EXT_REFCLK0_out.data,
		.freq_idx = AM62LX_FREQ_VALUE_BOARD_0_AUDIO_EXT_REFCLK0_OUT,
	},
	[CLK_AM62LX_BOARD_0_AUDIO_EXT_REFCLK1_OUT] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data = &clk_data_board_0_AUDIO_EXT_REFCLK1_out.data,
		.freq_idx = AM62LX_FREQ_VALUE_BOARD_0_AUDIO_EXT_REFCLK1_OUT,
	},
	[CLK_AM62LX_BOARD_0_CP_GEMAC_CPTS0_RFT_CLK_OUT] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data = &clk_data_board_0_CP_GEMAC_CPTS0_RFT_CLK_out.data,
		.freq_idx = AM62LX_FREQ_VALUE_BOARD_0_CP_GEMAC_CPTS0_RFT_CLK_OUT,
	},
	[CLK_AM62LX_BOARD_0_EXT_REFCLK1_OUT] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data = &clk_data_board_0_EXT_REFCLK1_out.data,
		.freq_idx = AM62LX_FREQ_VALUE_BOARD_0_EXT_REFCLK1_OUT,
	},
	[CLK_AM62LX_BOARD_0_GPMC0_CLKLB_OUT] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data = &clk_data_board_0_GPMC0_CLKLB_out.data,
		.freq_idx = AM62LX_FREQ_VALUE_BOARD_0_GPMC0_CLKLB_OUT,
	},
	[CLK_AM62LX_BOARD_0_I2C0_SCL_OUT] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data = &clk_data_board_0_I2C0_SCL_out.data,
		.freq_idx = AM62LX_FREQ_VALUE_BOARD_0_I2C0_SCL_OUT,
	},
	[CLK_AM62LX_BOARD_0_I2C1_SCL_OUT] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data = &clk_data_board_0_I2C1_SCL_out.data,
		.freq_idx = AM62LX_FREQ_VALUE_BOARD_0_I2C1_SCL_OUT,
	},
	[CLK_AM62LX_BOARD_0_I2C2_SCL_OUT] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data = &clk_data_board_0_I2C2_SCL_out.data,
		.freq_idx = AM62LX_FREQ_VALUE_BOARD_0_I2C2_SCL_OUT,
	},
	[CLK_AM62LX_BOARD_0_I2C3_SCL_OUT] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data = &clk_data_board_0_I2C3_SCL_out.data,
		.freq_idx = AM62LX_FREQ_VALUE_BOARD_0_I2C3_SCL_OUT,
	},
	[CLK_AM62LX_BOARD_0_MCASP0_ACLKR_OUT] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data = &clk_data_board_0_MCASP0_ACLKR_out.data,
		.freq_idx = AM62LX_FREQ_VALUE_BOARD_0_MCASP0_ACLKR_OUT,
	},
	[CLK_AM62LX_BOARD_0_MCASP0_ACLKX_OUT] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data = &clk_data_board_0_MCASP0_ACLKX_out.data,
		.freq_idx = AM62LX_FREQ_VALUE_BOARD_0_MCASP0_ACLKX_OUT,
	},
	[CLK_AM62LX_BOARD_0_MCASP0_AFSR_OUT] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data = &clk_data_board_0_MCASP0_AFSR_out.data,
		.freq_idx = AM62LX_FREQ_VALUE_BOARD_0_MCASP0_AFSR_OUT,
	},
	[CLK_AM62LX_BOARD_0_MCASP0_AFSX_OUT] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data = &clk_data_board_0_MCASP0_AFSX_out.data,
		.freq_idx = AM62LX_FREQ_VALUE_BOARD_0_MCASP0_AFSX_OUT,
	},
	[CLK_AM62LX_BOARD_0_MCASP1_ACLKR_OUT] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data = &clk_data_board_0_MCASP1_ACLKR_out.data,
		.freq_idx = AM62LX_FREQ_VALUE_BOARD_0_MCASP1_ACLKR_OUT,
	},
	[CLK_AM62LX_BOARD_0_MCASP1_ACLKX_OUT] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data = &clk_data_board_0_MCASP1_ACLKX_out.data,
		.freq_idx = AM62LX_FREQ_VALUE_BOARD_0_MCASP1_ACLKX_OUT,
	},
	[CLK_AM62LX_BOARD_0_MCASP1_AFSR_OUT] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data = &clk_data_board_0_MCASP1_AFSR_out.data,
		.freq_idx = AM62LX_FREQ_VALUE_BOARD_0_MCASP1_AFSR_OUT,
	},
	[CLK_AM62LX_BOARD_0_MCASP1_AFSX_OUT] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data = &clk_data_board_0_MCASP1_AFSX_out.data,
		.freq_idx = AM62LX_FREQ_VALUE_BOARD_0_MCASP1_AFSX_OUT,
	},
	[CLK_AM62LX_BOARD_0_MCASP2_ACLKR_OUT] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data = &clk_data_board_0_MCASP2_ACLKR_out.data,
		.freq_idx = AM62LX_FREQ_VALUE_BOARD_0_MCASP2_ACLKR_OUT,
	},
	[CLK_AM62LX_BOARD_0_MCASP2_ACLKX_OUT] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data = &clk_data_board_0_MCASP2_ACLKX_out.data,
		.freq_idx = AM62LX_FREQ_VALUE_BOARD_0_MCASP2_ACLKX_OUT,
	},
	[CLK_AM62LX_BOARD_0_MCASP2_AFSR_OUT] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data = &clk_data_board_0_MCASP2_AFSR_out.data,
		.freq_idx = AM62LX_FREQ_VALUE_BOARD_0_MCASP2_AFSR_OUT,
	},
	[CLK_AM62LX_BOARD_0_MCASP2_AFSX_OUT] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data = &clk_data_board_0_MCASP2_AFSX_out.data,
		.freq_idx = AM62LX_FREQ_VALUE_BOARD_0_MCASP2_AFSX_OUT,
	},
	[CLK_AM62LX_BOARD_0_MMC0_CLKLB_OUT] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data = &clk_data_board_0_MMC0_CLKLB_out.data,
		.freq_idx = AM62LX_FREQ_VALUE_BOARD_0_MMC0_CLKLB_OUT,
	},
	[CLK_AM62LX_BOARD_0_MMC0_CLK_OUT] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data = &clk_data_board_0_MMC0_CLK_out.data,
		.freq_idx = AM62LX_FREQ_VALUE_BOARD_0_MMC0_CLK_OUT,
	},
	[CLK_AM62LX_BOARD_0_MMC1_CLKLB_OUT] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data = &clk_data_board_0_MMC1_CLKLB_out.data,
		.freq_idx = AM62LX_FREQ_VALUE_BOARD_0_MMC1_CLKLB_OUT,
	},
	[CLK_AM62LX_BOARD_0_MMC1_CLK_OUT] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data = &clk_data_board_0_MMC1_CLK_out.data,
		.freq_idx = AM62LX_FREQ_VALUE_BOARD_0_MMC1_CLK_OUT,
	},
	[CLK_AM62LX_BOARD_0_MMC2_CLKLB_OUT] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data = &clk_data_board_0_MMC2_CLKLB_out.data,
		.freq_idx = AM62LX_FREQ_VALUE_BOARD_0_MMC2_CLKLB_OUT,
	},
	[CLK_AM62LX_BOARD_0_MMC2_CLK_OUT] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data = &clk_data_board_0_MMC2_CLK_out.data,
		.freq_idx = AM62LX_FREQ_VALUE_BOARD_0_MMC2_CLK_OUT,
	},
	[CLK_AM62LX_BOARD_0_OSPI0_DQS_OUT] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data = &clk_data_board_0_OSPI0_DQS_out.data,
		.freq_idx = AM62LX_FREQ_VALUE_BOARD_0_OSPI0_DQS_OUT,
	},
	[CLK_AM62LX_BOARD_0_OSPI0_LBCLKO_OUT] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data = &clk_data_board_0_OSPI0_LBCLKO_out.data,
		.freq_idx = AM62LX_FREQ_VALUE_BOARD_0_OSPI0_LBCLKO_OUT,
	},
	[CLK_AM62LX_BOARD_0_RMII1_REF_CLK_OUT] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data = &clk_data_board_0_RMII1_REF_CLK_out.data,
		.freq_idx = AM62LX_FREQ_VALUE_BOARD_0_RMII1_REF_CLK_OUT,
	},
	[CLK_AM62LX_BOARD_0_RMII2_REF_CLK_OUT] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data = &clk_data_board_0_RMII2_REF_CLK_out.data,
		.freq_idx = AM62LX_FREQ_VALUE_BOARD_0_RMII2_REF_CLK_OUT,
	},
	[CLK_AM62LX_BOARD_0_TCK_OUT] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data = &clk_data_board_0_TCK_out.data,
		.freq_idx = AM62LX_FREQ_VALUE_BOARD_0_TCK_OUT,
	},
	[CLK_AM62LX_BOARD_0_VOUT0_EXTPCLKIN_OUT] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data = &clk_data_board_0_VOUT0_EXTPCLKIN_out.data,
		.freq_idx = AM62LX_FREQ_VALUE_BOARD_0_VOUT0_EXTPCLKIN_OUT,
	},
	[CLK_AM62LX_BOARD_0_WKUP_EXT_REFCLK0_OUT] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data = &clk_data_board_0_WKUP_EXT_REFCLK0_out.data,
		.freq_idx = AM62LX_FREQ_VALUE_BOARD_0_WKUP_EXT_REFCLK0_OUT,
	},
	[CLK_AM62LX_BOARD_0_WKUP_I2C0_SCL_OUT] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data = &clk_data_board_0_WKUP_I2C0_SCL_out.data,
		.freq_idx = AM62LX_FREQ_VALUE_BOARD_0_WKUP_I2C0_SCL_OUT,
	},
	[CLK_AM62LX_CPSW_3GUSS_AM62L_MAIN_0_CPTS_GENF0] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data = &clk_data_cpsw_3guss_am62l_main_0_cpts_genf0.data,
		.freq_idx = AM62LX_FREQ_VALUE_CPSW_3GUSS_AM62L_MAIN_0_CPTS_GENF0,
	},
	[CLK_AM62LX_CPSW_3GUSS_AM62L_MAIN_0_CPTS_GENF1] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data = &clk_data_cpsw_3guss_am62l_main_0_cpts_genf1.data,
		.freq_idx = AM62LX_FREQ_VALUE_CPSW_3GUSS_AM62L_MAIN_0_CPTS_GENF1,
	},
	[CLK_AM62LX_CPSW_3GUSS_AM62L_MAIN_0_MDIO_MDCLK_O] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data = &clk_data_cpsw_3guss_am62l_main_0_mdio_mdclk_o.data,
		.freq_idx = AM62LX_FREQ_VALUE_CPSW_3GUSS_AM62L_MAIN_0_MDIO_MDCLK_O,
	},
	[CLK_AM62LX_DEBUGSS_K3_WRAP_CV0_MAIN_0_CSTPIU_TRACECLK] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data = &clk_data_debugss_k3_wrap_cv0_main_0_cstpiu_traceclk.data,
		.freq_idx = AM62LX_FREQ_VALUE_DEBUGSS_K3_WRAP_CV0_MAIN_0_CSTPIU_TRACECLK,
	},
	[CLK_AM62LX_DMTIMER_DMC1MS_MAIN_0_TIMER_PWM] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data = &clk_data_dmtimer_dmc1ms_main_0_timer_pwm.data,
		.freq_idx = AM62LX_FREQ_VALUE_DMTIMER_DMC1MS_MAIN_0_TIMER_PWM,
	},
	[CLK_AM62LX_DMTIMER_DMC1MS_MAIN_1_TIMER_PWM] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data = &clk_data_dmtimer_dmc1ms_main_1_timer_pwm.data,
		.freq_idx = AM62LX_FREQ_VALUE_DMTIMER_DMC1MS_MAIN_1_TIMER_PWM,
	},
	[CLK_AM62LX_DMTIMER_DMC1MS_MAIN_2_TIMER_PWM] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data = &clk_data_dmtimer_dmc1ms_main_2_timer_pwm.data,
		.freq_idx = AM62LX_FREQ_VALUE_DMTIMER_DMC1MS_MAIN_2_TIMER_PWM,
	},
	[CLK_AM62LX_DMTIMER_DMC1MS_MAIN_3_TIMER_PWM] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data = &clk_data_dmtimer_dmc1ms_main_3_timer_pwm.data,
		.freq_idx = AM62LX_FREQ_VALUE_DMTIMER_DMC1MS_MAIN_3_TIMER_PWM,
	},
	[CLK_AM62LX_DMTIMER_DMC1MS_WKUP_0_TIMER_PWM] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data = &clk_data_dmtimer_dmc1ms_wkup_0_timer_pwm.data,
		.freq_idx = AM62LX_FREQ_VALUE_DMTIMER_DMC1MS_WKUP_0_TIMER_PWM,
	},
	[CLK_AM62LX_EMMCSD4SS_MAIN_0_EMMCSDSS_IO_CLK_O] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data = &clk_data_emmcsd4ss_main_0_emmcsdss_io_clk_o.data,
		.freq_idx = AM62LX_FREQ_VALUE_EMMCSD4SS_MAIN_0_EMMCSDSS_IO_CLK_O,
	},
	[CLK_AM62LX_EMMCSD4SS_MAIN_1_EMMCSDSS_IO_CLK_O] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data = &clk_data_emmcsd4ss_main_1_emmcsdss_io_clk_o.data,
		.freq_idx = AM62LX_FREQ_VALUE_EMMCSD4SS_MAIN_1_EMMCSDSS_IO_CLK_O,
	},
	[CLK_AM62LX_EMMCSD8SS_MAIN_0_EMMCSDSS_IO_CLK_O] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data = &clk_data_emmcsd8ss_main_0_emmcsdss_io_clk_o.data,
		.freq_idx = AM62LX_FREQ_VALUE_EMMCSD8SS_MAIN_0_EMMCSDSS_IO_CLK_O,
	},
	[CLK_AM62LX_FSS_UL_128_MAIN_0_OSPI0_OCLK_CLK] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data = &clk_data_fss_ul_128_main_0_ospi0_oclk_clk.data,
		.freq_idx = AM62LX_FREQ_VALUE_FSS_UL_128_MAIN_0_OSPI0_OCLK_CLK,
	},
	[CLK_AM62LX_GPMC_MAIN_0_PO_GPMC_DEV_CLK] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data = &clk_data_gpmc_main_0_po_gpmc_dev_clk.data,
		.freq_idx = AM62LX_FREQ_VALUE_GPMC_MAIN_0_PO_GPMC_DEV_CLK,
	},
	[CLK_AM62LX_HSDIV0_16FFT_WKUP_32KHZ_GEN_0_HSDIVOUT0_CLK] = {
		.parent = {
			CLK_AM62LX_GLUELOGIC_HFOSC0_CLK,
			1,
		},
		.drv = &clk_drv_div_reg.drv,
		.flags = 0,
		.type = CLK_TYPE_DIV,
		.data = &clk_data_hsdiv0_16fft_wkup_32khz_gen_0_hsdiv0.data_div.data,
	},
	[CLK_AM62LX_K3_DSS_NANO_MAIN_0_DPI_0_OUT_CLK] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data = &clk_data_k3_dss_nano_main_0_dpi_0_out_clk.data,
		.freq_idx = AM62LX_FREQ_VALUE_K3_DSS_NANO_MAIN_0_DPI_0_OUT_CLK,
	},
	[CLK_AM62LX_MCASP_MAIN_0_MCASP_ACLKR_POUT] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data = &clk_data_mcasp_main_0_mcasp_aclkr_pout.data,
		.freq_idx = AM62LX_FREQ_VALUE_MCASP_MAIN_0_MCASP_ACLKR_POUT,
	},
	[CLK_AM62LX_MCASP_MAIN_0_MCASP_ACLKX_POUT] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data = &clk_data_mcasp_main_0_mcasp_aclkx_pout.data,
		.freq_idx = AM62LX_FREQ_VALUE_MCASP_MAIN_0_MCASP_ACLKX_POUT,
	},
	[CLK_AM62LX_MCASP_MAIN_0_MCASP_AFSR_POUT] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data = &clk_data_mcasp_main_0_mcasp_afsr_pout.data,
		.freq_idx = AM62LX_FREQ_VALUE_MCASP_MAIN_0_MCASP_AFSR_POUT,
	},
	[CLK_AM62LX_MCASP_MAIN_0_MCASP_AFSX_POUT] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data = &clk_data_mcasp_main_0_mcasp_afsx_pout.data,
		.freq_idx = AM62LX_FREQ_VALUE_MCASP_MAIN_0_MCASP_AFSX_POUT,
	},
	[CLK_AM62LX_MCASP_MAIN_0_MCASP_AHCLKR_POUT] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data = &clk_data_mcasp_main_0_mcasp_ahclkr_pout.data,
		.freq_idx = AM62LX_FREQ_VALUE_MCASP_MAIN_0_MCASP_AHCLKR_POUT,
	},
	[CLK_AM62LX_MCASP_MAIN_0_MCASP_AHCLKX_POUT] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data = &clk_data_mcasp_main_0_mcasp_ahclkx_pout.data,
		.freq_idx = AM62LX_FREQ_VALUE_MCASP_MAIN_0_MCASP_AHCLKX_POUT,
	},
	[CLK_AM62LX_MCASP_MAIN_1_MCASP_ACLKR_POUT] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data = &clk_data_mcasp_main_1_mcasp_aclkr_pout.data,
		.freq_idx = AM62LX_FREQ_VALUE_MCASP_MAIN_1_MCASP_ACLKR_POUT,
	},
	[CLK_AM62LX_MCASP_MAIN_1_MCASP_ACLKX_POUT] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data = &clk_data_mcasp_main_1_mcasp_aclkx_pout.data,
		.freq_idx = AM62LX_FREQ_VALUE_MCASP_MAIN_1_MCASP_ACLKX_POUT,
	},
	[CLK_AM62LX_MCASP_MAIN_1_MCASP_AFSR_POUT] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data = &clk_data_mcasp_main_1_mcasp_afsr_pout.data,
		.freq_idx = AM62LX_FREQ_VALUE_MCASP_MAIN_1_MCASP_AFSR_POUT,
	},
	[CLK_AM62LX_MCASP_MAIN_1_MCASP_AFSX_POUT] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data = &clk_data_mcasp_main_1_mcasp_afsx_pout.data,
		.freq_idx = AM62LX_FREQ_VALUE_MCASP_MAIN_1_MCASP_AFSX_POUT,
	},
	[CLK_AM62LX_MCASP_MAIN_1_MCASP_AHCLKR_POUT] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data = &clk_data_mcasp_main_1_mcasp_ahclkr_pout.data,
		.freq_idx = AM62LX_FREQ_VALUE_MCASP_MAIN_1_MCASP_AHCLKR_POUT,
	},
	[CLK_AM62LX_MCASP_MAIN_1_MCASP_AHCLKX_POUT] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data = &clk_data_mcasp_main_1_mcasp_ahclkx_pout.data,
		.freq_idx = AM62LX_FREQ_VALUE_MCASP_MAIN_1_MCASP_AHCLKX_POUT,
	},
	[CLK_AM62LX_MCASP_MAIN_2_MCASP_ACLKR_POUT] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data = &clk_data_mcasp_main_2_mcasp_aclkr_pout.data,
		.freq_idx = AM62LX_FREQ_VALUE_MCASP_MAIN_2_MCASP_ACLKR_POUT,
	},
	[CLK_AM62LX_MCASP_MAIN_2_MCASP_ACLKX_POUT] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data = &clk_data_mcasp_main_2_mcasp_aclkx_pout.data,
		.freq_idx = AM62LX_FREQ_VALUE_MCASP_MAIN_2_MCASP_ACLKX_POUT,
	},
	[CLK_AM62LX_MCASP_MAIN_2_MCASP_AFSR_POUT] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data = &clk_data_mcasp_main_2_mcasp_afsr_pout.data,
		.freq_idx = AM62LX_FREQ_VALUE_MCASP_MAIN_2_MCASP_AFSR_POUT,
	},
	[CLK_AM62LX_MCASP_MAIN_2_MCASP_AFSX_POUT] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data = &clk_data_mcasp_main_2_mcasp_afsx_pout.data,
		.freq_idx = AM62LX_FREQ_VALUE_MCASP_MAIN_2_MCASP_AFSX_POUT,
	},
	[CLK_AM62LX_MCASP_MAIN_2_MCASP_AHCLKR_POUT] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data = &clk_data_mcasp_main_2_mcasp_ahclkr_pout.data,
		.freq_idx = AM62LX_FREQ_VALUE_MCASP_MAIN_2_MCASP_AHCLKR_POUT,
	},
	[CLK_AM62LX_MCASP_MAIN_2_MCASP_AHCLKX_POUT] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data = &clk_data_mcasp_main_2_mcasp_ahclkx_pout.data,
		.freq_idx = AM62LX_FREQ_VALUE_MCASP_MAIN_2_MCASP_AHCLKX_POUT,
	},
	[CLK_AM62LX_MSHSI2C_MAIN_0_PORSCL] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data = &clk_data_mshsi2c_main_0_porscl.data,
		.freq_idx = AM62LX_FREQ_VALUE_MSHSI2C_MAIN_0_PORSCL,
	},
	[CLK_AM62LX_MSHSI2C_MAIN_1_PORSCL] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data = &clk_data_mshsi2c_main_1_porscl.data,
		.freq_idx = AM62LX_FREQ_VALUE_MSHSI2C_MAIN_1_PORSCL,
	},
	[CLK_AM62LX_MSHSI2C_MAIN_2_PORSCL] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data = &clk_data_mshsi2c_main_2_porscl.data,
		.freq_idx = AM62LX_FREQ_VALUE_MSHSI2C_MAIN_2_PORSCL,
	},
	[CLK_AM62LX_MSHSI2C_MAIN_3_PORSCL] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data = &clk_data_mshsi2c_main_3_porscl.data,
		.freq_idx = AM62LX_FREQ_VALUE_MSHSI2C_MAIN_3_PORSCL,
	},
	[CLK_AM62LX_MSHSI2C_WKUP_0_PORSCL] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data = &clk_data_mshsi2c_wkup_0_porscl.data,
		.freq_idx = AM62LX_FREQ_VALUE_MSHSI2C_WKUP_0_PORSCL,
	},
	[CLK_AM62LX_PLLFRACF2_SSMOD_16FFT_MAIN_0_FOUTVCOP_CLK] = {
		.parent = {
			CLK_AM62LX_GLUELOGIC_HFOSC0_CLK,
			1,
		},
		.drv = &clk_drv_pll_16fft,
		.freq_idx = AM62LX_FREQ_VALUE_PLLFRACF2_SSMOD_16FFT_MAIN_0,
		.data = &clk_data_pllfracf2_ssmod_16fft_main_0.data_pll.data,
		.flags = CLK_DATA_FLAG_NO_HW_REINIT,
	},
	[CLK_AM62LX_PLLFRACF2_SSMOD_16FFT_MAIN_0_FOUTPOSTDIV_CLK] = {
		.parent = {
			CLK_AM62LX_PLLFRACF2_SSMOD_16FFT_MAIN_0_FOUTVCOP_CLK,
			1,
		},
		.drv = &clk_drv_div_pll_16fft_postdiv.drv,
		.type = CLK_TYPE_DIV,
		.data = &clk_data_pllfracf2_ssmod_16fft_main_0_postdiv.data,
		.flags = 0,
	},
	[CLK_AM62LX_PLLFRACF2_SSMOD_16FFT_MAIN_17_FOUTVCOP_CLK] = {
		.parent = {
			CLK_AM62LX_GLUELOGIC_HFOSC0_CLK,
			1,
		},
		.drv = &clk_drv_pll_16fft,
		.freq_idx = AM62LX_FREQ_VALUE_PLLFRACF2_SSMOD_16FFT_MAIN_17,
		.data = &clk_data_pllfracf2_ssmod_16fft_main_17.data_pll.data,
		.flags = 0,
	},
	[CLK_AM62LX_PLLFRACF2_SSMOD_16FFT_MAIN_8_FOUTVCOP_CLK] = {
		.parent = {
			CLK_AM62LX_GLUELOGIC_HFOSC0_CLK,
			1,
		},
		.drv = &clk_drv_pll_16fft,
		.freq_idx = AM62LX_FREQ_VALUE_PLLFRACF2_SSMOD_16FFT_MAIN_8,
		.data = &clk_data_pllfracf2_ssmod_16fft_main_8.data_pll.data,
		.flags = CLK_DATA_FLAG_NO_HW_REINIT,
	},
	[CLK_AM62LX_PLLFRACF2_SSMOD_16FFT_WKUP_0_FOUTVCOP_CLK] = {
		.parent = {
			CLK_AM62LX_GLUELOGIC_HFOSC0_CLK,
			1,
		},
		.drv = &clk_drv_pll_16fft,
		.freq_idx = AM62LX_FREQ_VALUE_PLLFRACF2_SSMOD_16FFT_WKUP_0,
		.data = &clk_data_pllfracf2_ssmod_16fft_wkup_0.data_pll.data,
		.flags = CLK_DATA_FLAG_NO_HW_REINIT,
	},
	[CLK_AM62LX_PLLFRACF2_SSMOD_16FFT_WKUP_0_FOUTPOSTDIV_CLK] = {
		.parent = {
			CLK_AM62LX_PLLFRACF2_SSMOD_16FFT_WKUP_0_FOUTVCOP_CLK,
			1,
		},
		.drv = &clk_drv_div_pll_16fft_postdiv.drv,
		.type = CLK_TYPE_DIV,
		.data = &clk_data_pllfracf2_ssmod_16fft_wkup_0_postdiv.data,
		.flags = 0,
	},
	[CLK_AM62LX_POSTDIV4_16FF_MAIN_0_HSDIVOUT5_CLK] = {
		.parent = {
			CLK_AM62LX_PLLFRACF2_SSMOD_16FFT_MAIN_0_FOUTPOSTDIV_CLK,
			1,
		},
		.drv = &clk_drv_div_pll_16fft_postdiv_hsdiv.drv,
		.flags = 0,
		.type = CLK_TYPE_DIV,
		.data = &clk_data_postdiv4_16ff_main_0_hsdiv5.data_div.data,
	},
	[CLK_AM62LX_POSTDIV4_16FF_MAIN_0_HSDIVOUT6_CLK] = {
		.parent = {
			CLK_AM62LX_PLLFRACF2_SSMOD_16FFT_MAIN_0_FOUTPOSTDIV_CLK,
			1,
		},
		.drv = &clk_drv_div_pll_16fft_postdiv_hsdiv.drv,
		.flags = 0,
		.type = CLK_TYPE_DIV,
		.data = &clk_data_postdiv4_16ff_main_0_hsdiv6.data_div.data,
	},
	[CLK_AM62LX_POSTDIV4_16FF_MAIN_0_HSDIVOUT7_CLK] = {
		.parent = {
			CLK_AM62LX_PLLFRACF2_SSMOD_16FFT_MAIN_0_FOUTPOSTDIV_CLK,
			1,
		},
		.drv = &clk_drv_div_pll_16fft_postdiv_hsdiv.drv,
		.flags = 0,
		.type = CLK_TYPE_DIV,
		.data = &clk_data_postdiv4_16ff_main_0_hsdiv7.data_div.data,
	},
	[CLK_AM62LX_POSTDIV4_16FF_MAIN_0_HSDIVOUT8_CLK] = {
		.parent = {
			CLK_AM62LX_PLLFRACF2_SSMOD_16FFT_MAIN_0_FOUTPOSTDIV_CLK,
			1,
		},
		.drv = &clk_drv_div_pll_16fft_postdiv_hsdiv.drv,
		.flags = 0,
		.type = CLK_TYPE_DIV,
		.data = &clk_data_postdiv4_16ff_main_0_hsdiv8.data_div.data,
	},
	[CLK_AM62LX_POSTDIV4_16FF_MAIN_0_HSDIVOUT9_CLK] = {
		.parent = {
			CLK_AM62LX_PLLFRACF2_SSMOD_16FFT_MAIN_0_FOUTPOSTDIV_CLK,
			1,
		},
		.drv = &clk_drv_div_pll_16fft_postdiv_hsdiv.drv,
		.flags = 0,
		.type = CLK_TYPE_DIV,
		.data = &clk_data_postdiv4_16ff_main_0_hsdiv9.data_div.data,
	},
	[CLK_AM62LX_POSTDIV4_16FF_WKUP_0_HSDIVOUT6_CLK] = {
		.parent = {
			CLK_AM62LX_PLLFRACF2_SSMOD_16FFT_WKUP_0_FOUTPOSTDIV_CLK,
			1,
		},
		.drv = &clk_drv_div_pll_16fft_postdiv_hsdiv.drv,
		.flags = 0,
		.type = CLK_TYPE_DIV,
		.data = &clk_data_postdiv4_16ff_wkup_0_hsdiv6.data_div.data,
	},
	[CLK_AM62LX_POSTDIV4_16FF_WKUP_0_HSDIVOUT7_CLK] = {
		.parent = {
			CLK_AM62LX_PLLFRACF2_SSMOD_16FFT_WKUP_0_FOUTPOSTDIV_CLK,
			1,
		},
		.drv = &clk_drv_div_pll_16fft_postdiv_hsdiv.drv,
		.flags = 0,
		.type = CLK_TYPE_DIV,
		.data = &clk_data_postdiv4_16ff_wkup_0_hsdiv7.data_div.data,
	},
	[CLK_AM62LX_POSTDIV4_16FF_WKUP_0_HSDIVOUT8_CLK] = {
		.parent = {
			CLK_AM62LX_PLLFRACF2_SSMOD_16FFT_WKUP_0_FOUTPOSTDIV_CLK,
			1,
		},
		.drv = &clk_drv_div_pll_16fft_postdiv_hsdiv.drv,
		.flags = 0,
		.type = CLK_TYPE_DIV,
		.data = &clk_data_postdiv4_16ff_wkup_0_hsdiv8.data_div.data,
	},
	[CLK_AM62LX_POSTDIV4_16FF_WKUP_0_HSDIVOUT9_CLK] = {
		.parent = {
			CLK_AM62LX_PLLFRACF2_SSMOD_16FFT_WKUP_0_FOUTPOSTDIV_CLK,
			1,
		},
		.drv = &clk_drv_div_pll_16fft_postdiv_hsdiv.drv,
		.flags = 0,
		.type = CLK_TYPE_DIV,
		.data = &clk_data_postdiv4_16ff_wkup_0_hsdiv9.data_div.data,
	},
	[CLK_AM62LX_RTCSS_WKUP_0_OSC_32K_CLK] = {
		.drv = &clk_drv_soc_lfosc0,
		.flags = 0,
		.data = &clk_data_rtcss_wkup_0_osc_32k_clk.data,
		.freq_idx = AM62LX_FREQ_VALUE_RTCSS_WKUP_0_OSC_32K_CLK,
	},
	[CLK_AM62LX_SAM62L_A53_256KB_WRAP_MAIN_0_ARM_COREPACK_0_A53_DIVH_CLK4_OBSCLK_OUT_CLK] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data =
		&clk_data_sam62l_a53_256kb_wrap_main_0_arm_corepack_0_a53_divh_clk4_obsclk_out_clk.data,
		.freq_idx =
		AM62LX_FREQ_VALUE_SAM62L_A53_256KB_WRAP_MAIN_0_ARM_COREPACK_0_A53_DIVH_CLK4_OBSCLK_OUT_CLK,
	},
	[CLK_AM62LX_SAM62L_A53_256KB_WRAP_MAIN_0_CLKDIV_0_DIVH_CLK4_CLK_CLK] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data = &clk_data_sam62l_a53_256kb_wrap_main_0_clkdiv_0_divh_clk4_clk_clk.data,
		.freq_idx =
		AM62LX_FREQ_VALUE_SAM62L_A53_256KB_WRAP_MAIN_0_CLKDIV_0_DIVH_CLK4_CLK_CLK,
	},
	[CLK_AM62LX_SAM62L_A53_256KB_WRAP_MAIN_0_CLKDIV_0_DIVP_CLK1_CLK_CLK] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data = &clk_data_sam62l_a53_256kb_wrap_main_0_clkdiv_0_divp_clk1_clk_clk.data,
		.freq_idx =
		AM62LX_FREQ_VALUE_SAM62L_A53_256KB_WRAP_MAIN_0_CLKDIV_0_DIVP_CLK1_CLK_CLK,
	},
	[CLK_AM62LX_SPI_MAIN_0_IO_CLKSPIO_CLK] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data = &clk_data_spi_main_0_io_clkspio_clk.data,
		.freq_idx = AM62LX_FREQ_VALUE_SPI_MAIN_0_IO_CLKSPIO_CLK,
	},
	[CLK_AM62LX_SPI_MAIN_1_IO_CLKSPIO_CLK] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data = &clk_data_spi_main_1_io_clkspio_clk.data,
		.freq_idx = AM62LX_FREQ_VALUE_SPI_MAIN_1_IO_CLKSPIO_CLK,
	},
	[CLK_AM62LX_SPI_MAIN_2_IO_CLKSPIO_CLK] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data = &clk_data_spi_main_2_io_clkspio_clk.data,
		.freq_idx = AM62LX_FREQ_VALUE_SPI_MAIN_2_IO_CLKSPIO_CLK,
	},
	[CLK_AM62LX_SPI_MAIN_3_IO_CLKSPIO_CLK] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data = &clk_data_spi_main_3_io_clkspio_clk.data,
		.freq_idx = AM62LX_FREQ_VALUE_SPI_MAIN_3_IO_CLKSPIO_CLK,
	},
	[CLK_AM62LX_WIZ16B8M4CDT3_MAIN_0_IP1_PPI_M_RXCLKESC_CLK] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data = &clk_data_wiz16b8m4cdt3_main_0_ip1_ppi_M_RxClkEsc_clk.data,
		.freq_idx = AM62LX_FREQ_VALUE_WIZ16B8M4CDT3_MAIN_0_IP1_PPI_M_RXCLKESC_CLK,
	},
	[CLK_AM62LX_WIZ16B8M4CDT3_MAIN_0_IP1_PPI_TXBYTECLKHS_CL_CLK] = {
		.drv = &clk_drv_from_device,
		.flags = 0,
		.data = &clk_data_wiz16b8m4cdt3_main_0_ip1_ppi_TxByteClkHS_cl_clk.data,
		.freq_idx = AM62LX_FREQ_VALUE_WIZ16B8M4CDT3_MAIN_0_IP1_PPI_TXBYTECLKHS_CL_CLK,
	},
	[CLK_AM62LX_CLKOUT0_CTRL_OUT0] = {
		.drv = &clk_drv_mux_reg.drv,
		.flags = 0,
		.data = &clk_data_CLKOUT0_CTRL_out0.data_mux.data,
		.type = CLK_TYPE_MUX,
	},
	[CLK_AM62LX_CLK_32K_RC_SEL_OUT0] = {
		.drv = &clk_drv_mux_reg.drv,
		.flags = 0,
		.data = &clk_data_CLK_32K_RC_SEL_out0.data_mux.data,
		.type = CLK_TYPE_MUX,
	},
	[CLK_AM62LX_MAIN_DPHYTX_REFCLK_OUT0] = {
		.drv = &clk_drv_mux_reg.drv,
		.flags = 0,
		.data = &clk_data_MAIN_DPHYTX_REFCLK_out0.data_mux.data,
		.type = CLK_TYPE_MUX,
	},
	[CLK_AM62LX_MAIN_EMMCSD0_IO_CLKLB_SEL_OUT0] = {
		.drv = &clk_drv_mux_reg.drv,
		.flags = 0,
		.data = &clk_data_MAIN_EMMCSD0_IO_CLKLB_SEL_out0.data_mux.data,
		.type = CLK_TYPE_MUX,
	},
	[CLK_AM62LX_MAIN_EMMCSD0_REFCLK_SEL_OUT0] = {
		.drv = &clk_drv_mux_reg.drv,
		.flags = 0,
		.data = &clk_data_MAIN_EMMCSD0_REFCLK_SEL_out0.data_mux.data,
		.type = CLK_TYPE_MUX,
	},
	[CLK_AM62LX_MAIN_EMMCSD1_IO_CLKLB_SEL_OUT0] = {
		.drv = &clk_drv_mux_reg.drv,
		.flags = 0,
		.data = &clk_data_MAIN_EMMCSD1_IO_CLKLB_SEL_out0.data_mux.data,
		.type = CLK_TYPE_MUX,
	},
	[CLK_AM62LX_MAIN_EMMCSD1_REFCLK_SEL_OUT0] = {
		.drv = &clk_drv_mux_reg.drv,
		.flags = 0,
		.data = &clk_data_MAIN_EMMCSD1_REFCLK_SEL_out0.data_mux.data,
		.type = CLK_TYPE_MUX,
	},
	[CLK_AM62LX_MAIN_EMMCSD2_IO_CLKLB_SEL_OUT0] = {
		.drv = &clk_drv_mux_reg.drv,
		.flags = 0,
		.data = &clk_data_MAIN_EMMCSD2_IO_CLKLB_SEL_out0.data_mux.data,
		.type = CLK_TYPE_MUX,
	},
	[CLK_AM62LX_MAIN_EMMCSD2_REFCLK_SEL_OUT0] = {
		.drv = &clk_drv_mux_reg.drv,
		.flags = 0,
		.data = &clk_data_MAIN_EMMCSD2_REFCLK_SEL_out0.data_mux.data,
		.type = CLK_TYPE_MUX,
	},
	[CLK_AM62LX_MAIN_OSPI_LOOPBACK_CLK_SEL_OUT0] = {
		.drv = &clk_drv_mux_reg.drv,
		.flags = 0,
		.data = &clk_data_MAIN_OSPI_LOOPBACK_CLK_SEL_out0.data_mux.data,
		.type = CLK_TYPE_MUX,
	},
	[CLK_AM62LX_MAIN_USB0_REFCLK_SEL_OUT0] = {
		.drv = &clk_drv_mux_reg.drv,
		.flags = 0,
		.data = &clk_data_MAIN_USB0_REFCLK_SEL_out0.data_mux.data,
		.type = CLK_TYPE_MUX,
	},
	[CLK_AM62LX_MAIN_USB1_REFCLK_SEL_OUT0] = {
		.drv = &clk_drv_mux_reg.drv,
		.flags = 0,
		.data = &clk_data_MAIN_USB1_REFCLK_SEL_out0.data_mux.data,
		.type = CLK_TYPE_MUX,
	},
	[CLK_AM62LX_MAIN_WWDTCLKN_SEL_OUT0] = {
		.drv = &clk_drv_mux_reg.drv,
		.flags = 0,
		.data = &clk_data_MAIN_WWDTCLKn_SEL_out0.data_mux.data,
		.type = CLK_TYPE_MUX,
	},
	[CLK_AM62LX_MAIN_WWDTCLKN_SEL_OUT1] = {
		.drv = &clk_drv_mux_reg.drv,
		.flags = 0,
		.data = &clk_data_MAIN_WWDTCLKn_SEL_out1.data_mux.data,
		.type = CLK_TYPE_MUX,
	},
	[CLK_AM62LX_MCASPN_AHCLKSEL_AHCLKR_OUT0] = {
		.drv = &clk_drv_mux_reg.drv,
		.flags = 0,
		.data = &clk_data_MCASPn_AHCLKSEL_AHCLKR_out0.data_mux.data,
		.type = CLK_TYPE_MUX,
	},
	[CLK_AM62LX_MCASPN_AHCLKSEL_AHCLKR_OUT1] = {
		.drv = &clk_drv_mux_reg.drv,
		.flags = 0,
		.data = &clk_data_MCASPn_AHCLKSEL_AHCLKR_out1.data_mux.data,
		.type = CLK_TYPE_MUX,
	},
	[CLK_AM62LX_MCASPN_AHCLKSEL_AHCLKR_OUT2] = {
		.drv = &clk_drv_mux_reg.drv,
		.flags = 0,
		.data = &clk_data_MCASPn_AHCLKSEL_AHCLKR_out2.data_mux.data,
		.type = CLK_TYPE_MUX,
	},
	[CLK_AM62LX_MCASPN_AHCLKSEL_AHCLKX_OUT0] = {
		.drv = &clk_drv_mux_reg.drv,
		.flags = 0,
		.data = &clk_data_MCASPn_AHCLKSEL_AHCLKX_out0.data_mux.data,
		.type = CLK_TYPE_MUX,
	},
	[CLK_AM62LX_MCASPN_AHCLKSEL_AHCLKX_OUT1] = {
		.drv = &clk_drv_mux_reg.drv,
		.flags = 0,
		.data = &clk_data_MCASPn_AHCLKSEL_AHCLKX_out1.data_mux.data,
		.type = CLK_TYPE_MUX,
	},
	[CLK_AM62LX_MCASPN_AHCLKSEL_AHCLKX_OUT2] = {
		.drv = &clk_drv_mux_reg.drv,
		.flags = 0,
		.data = &clk_data_MCASPn_AHCLKSEL_AHCLKX_out2.data_mux.data,
		.type = CLK_TYPE_MUX,
	},
	[CLK_AM62LX_HSDIV0_16FFT_MAIN_17_HSDIVOUT0_CLK] = {
		.parent = {
			CLK_AM62LX_PLLFRACF2_SSMOD_16FFT_MAIN_17_FOUTVCOP_CLK,
			1,
		},
		.drv = &clk_drv_div_pll_16fft_hsdiv.drv,
		.flags = CLK_DATA_FLAG_MODIFY_PARENT_FREQ,
		.type = CLK_TYPE_DIV,
		.data = &clk_data_hsdiv0_16fft_main_17_hsdiv0.data_div.data,
	},
	[CLK_AM62LX_HSDIV0_16FFT_MAIN_8_HSDIVOUT0_CLK] = {
		.parent = {
			CLK_AM62LX_PLLFRACF2_SSMOD_16FFT_MAIN_8_FOUTVCOP_CLK,
			1,
		},
		.drv = &clk_drv_div_pll_16fft_hsdiv.drv,
		.flags = CLK_DATA_FLAG_MODIFY_PARENT_FREQ | CLK_DATA_FLAG_NO_HW_REINIT,
		.type = CLK_TYPE_DIV,
		.data = &clk_data_hsdiv0_16fft_main_8_hsdiv0.data_div.data,
	},
	[CLK_AM62LX_HSDIV0_16FFT_WKUP_0_HSDIVOUT0_CLK] = {
		.parent = {
			CLK_AM62LX_PLLFRACF2_SSMOD_16FFT_WKUP_0_FOUTPOSTDIV_CLK,
			1,
		},
		.drv = &clk_drv_div_pll_16fft_hsdiv.drv,
		.flags = 0,
		.type = CLK_TYPE_DIV,
		.data = &clk_data_hsdiv0_16fft_wkup_0_hsdiv0.data_div.data,
	},
	[CLK_AM62LX_HSDIV4_16FFT_MAIN_0_HSDIVOUT0_CLK] = {
		.parent = {
			CLK_AM62LX_PLLFRACF2_SSMOD_16FFT_MAIN_0_FOUTVCOP_CLK,
			1,
		},
		.drv = &clk_drv_div_pll_16fft_hsdiv.drv,
		.flags = 0,
		.type = CLK_TYPE_DIV,
		.data = &clk_data_hsdiv4_16fft_main_0_hsdiv0.data_div.data,
	},
	[CLK_AM62LX_HSDIV4_16FFT_MAIN_0_HSDIVOUT2_CLK] = {
		.parent = {
			CLK_AM62LX_PLLFRACF2_SSMOD_16FFT_MAIN_0_FOUTVCOP_CLK,
			1,
		},
		.drv = &clk_drv_div_pll_16fft_hsdiv.drv,
		.flags = CLK_DATA_FLAG_NO_HW_REINIT,
		.type = CLK_TYPE_DIV,
		.data = &clk_data_hsdiv4_16fft_main_0_hsdiv2.data_div.data,
	},
	[CLK_AM62LX_HSDIV4_16FFT_MAIN_0_HSDIVOUT3_CLK] = {
		.parent = {
			CLK_AM62LX_PLLFRACF2_SSMOD_16FFT_MAIN_0_FOUTVCOP_CLK,
			1,
		},
		.drv = &clk_drv_div_pll_16fft_hsdiv.drv,
		.flags = 0,
		.type = CLK_TYPE_DIV,
		.data = &clk_data_hsdiv4_16fft_main_0_hsdiv3.data_div.data,
	},
	[CLK_AM62LX_HSDIV4_16FFT_MAIN_0_HSDIVOUT4_CLK] = {
		.parent = {
			CLK_AM62LX_PLLFRACF2_SSMOD_16FFT_MAIN_0_FOUTVCOP_CLK,
			1,
		},
		.drv = &clk_drv_div_pll_16fft_hsdiv.drv,
		.flags = 0,
		.type = CLK_TYPE_DIV,
		.data = &clk_data_hsdiv4_16fft_main_0_hsdiv4.data_div.data,
	},
	[CLK_AM62LX_HSDIV4_16FFT_WKUP_0_HSDIVOUT0_CLK] = {
		.parent = {
			CLK_AM62LX_PLLFRACF2_SSMOD_16FFT_WKUP_0_FOUTVCOP_CLK,
			1,
		},
		.drv = &clk_drv_div_pll_16fft_hsdiv.drv,
		.flags = 0,
		.type = CLK_TYPE_DIV,
		.data = &clk_data_hsdiv4_16fft_wkup_0_hsdiv0.data_div.data,
	},
	[CLK_AM62LX_HSDIV4_16FFT_WKUP_0_HSDIVOUT1_CLK] = {
		.parent = {
			CLK_AM62LX_PLLFRACF2_SSMOD_16FFT_WKUP_0_FOUTVCOP_CLK,
			1,
		},
		.drv = &clk_drv_div_pll_16fft_hsdiv.drv,
		.flags = 0,
		.type = CLK_TYPE_DIV,
		.data = &clk_data_hsdiv4_16fft_wkup_0_hsdiv1.data_div.data,
	},
	[CLK_AM62LX_HSDIV4_16FFT_WKUP_0_HSDIVOUT2_CLK] = {
		.parent = {
			CLK_AM62LX_PLLFRACF2_SSMOD_16FFT_WKUP_0_FOUTVCOP_CLK,
			1,
		},
		.drv = &clk_drv_div_pll_16fft_hsdiv.drv,
		.flags = 0,
		.type = CLK_TYPE_DIV,
		.data = &clk_data_hsdiv4_16fft_wkup_0_hsdiv2.data_div.data,
	},
	[CLK_AM62LX_HSDIV4_16FFT_WKUP_0_HSDIVOUT3_CLK] = {
		.parent = {
			CLK_AM62LX_PLLFRACF2_SSMOD_16FFT_WKUP_0_FOUTVCOP_CLK,
			1,
		},
		.drv = &clk_drv_div_pll_16fft_hsdiv.drv,
		.flags = 0,
		.type = CLK_TYPE_DIV,
		.data = &clk_data_hsdiv4_16fft_wkup_0_hsdiv3.data_div.data,
	},
	[CLK_AM62LX_HSDIV4_16FFT_WKUP_0_HSDIVOUT4_CLK] = {
		.parent = {
			CLK_AM62LX_PLLFRACF2_SSMOD_16FFT_WKUP_0_FOUTVCOP_CLK,
			1,
		},
		.drv = &clk_drv_div_pll_16fft_hsdiv.drv,
		.flags = 0,
		.type = CLK_TYPE_DIV,
		.data = &clk_data_hsdiv4_16fft_wkup_0_hsdiv4.data_div.data,
	},
	[CLK_AM62LX_SAM62_PLL_CTRL_WRAP_WKUP_0_SYSCLKOUT_CLK] = {
		.drv = &clk_drv_pllctrl_mux_reg_ro.drv,
		.flags = 0,
		.data = &clk_data_sam62_pll_ctrl_wrap_wkup_0_sysclkout_clk.data_mux.data,
		.type = CLK_TYPE_MUX,
	},
	[CLK_AM62LX_SAM62_PLL_CTRL_WRAP_WKUP_0_CHIP_DIV1_CLK_CLK] = {
		.drv = &clk_drv_div_reg.drv,
		.flags = 0,
		.data = &clk_data_sam62_pll_ctrl_wrap_wkup_0_chip_div1_clk_clk.data_div.data,
		.parent = {
			CLK_AM62LX_SAM62_PLL_CTRL_WRAP_WKUP_0_SYSCLKOUT_CLK,
			1,
		},
	},
	[CLK_AM62LX_ADC0_CLKSEL_OUT0] = {
		.drv = &clk_drv_mux_reg.drv,
		.flags = 0,
		.data = &clk_data_ADC0_CLKSEL_out0.data_mux.data,
		.type = CLK_TYPE_MUX,
	},
	[CLK_AM62LX_AUDIO_REFCLKN_OUT0] = {
		.drv = &clk_drv_mux_reg.drv,
		.flags = 0,
		.data = &clk_data_AUDIO_REFCLKn_out0.data_mux.data,
		.type = CLK_TYPE_MUX,
	},
	[CLK_AM62LX_AUDIO_REFCLKN_OUT1] = {
		.drv = &clk_drv_mux_reg.drv,
		.flags = 0,
		.data = &clk_data_AUDIO_REFCLKn_out1.data_mux.data,
		.type = CLK_TYPE_MUX,
	},
	[CLK_AM62LX_MAIN_CP_GEMAC_CPTS_CLK_SEL_OUT0] = {
		.drv = &clk_drv_mux_reg.drv,
		.flags = 0,
		.data = &clk_data_MAIN_CP_GEMAC_CPTS_CLK_SEL_out0.data_mux.data,
		.type = CLK_TYPE_MUX,
	},
	[CLK_AM62LX_MAIN_DSS_DPI0_OUT0] = {
		.drv = &clk_drv_mux_reg.drv,
		.flags = 0,
		.data = &clk_data_MAIN_DSS_DPI0_out0.data_mux.data,
		.type = CLK_TYPE_MUX,
	},
	[CLK_AM62LX_MAIN_GPMC_FCLK_SEL_OUT0] = {
		.drv = &clk_drv_mux_reg.drv,
		.flags = 0,
		.data = &clk_data_MAIN_GPMC_FCLK_SEL_out0.data_mux.data,
		.type = CLK_TYPE_MUX,
	},
	[CLK_AM62LX_MAIN_MCANN_CLK_SEL_OUT0] = {
		.drv = &clk_drv_mux_reg.drv,
		.flags = 0,
		.data = &clk_data_MAIN_MCANn_CLK_SEL_out0.data_mux.data,
		.type = CLK_TYPE_MUX,
	},
	[CLK_AM62LX_MAIN_MCANN_CLK_SEL_OUT1] = {
		.drv = &clk_drv_mux_reg.drv,
		.flags = 0,
		.data = &clk_data_MAIN_MCANn_CLK_SEL_out1.data_mux.data,
		.type = CLK_TYPE_MUX,
	},
	[CLK_AM62LX_MAIN_MCANN_CLK_SEL_OUT2] = {
		.drv = &clk_drv_mux_reg.drv,
		.flags = 0,
		.data = &clk_data_MAIN_MCANn_CLK_SEL_out2.data_mux.data,
		.type = CLK_TYPE_MUX,
	},
	[CLK_AM62LX_MAIN_OBSCLK0_MUX_SEL_OUT0] = {
		.drv = &clk_drv_mux_reg.drv,
		.flags = 0,
		.data = &clk_data_MAIN_OBSCLK0_MUX_SEL_out0.data_mux.data,
		.type = CLK_TYPE_MUX,
	},
	[CLK_AM62LX_MAIN_OBSCLK_DIV_OUT0] = {
		.parent = {
			CLK_AM62LX_MAIN_OBSCLK0_MUX_SEL_OUT0,
			1,
		},
		.drv = &clk_drv_div_reg_go.drv,
		.flags = 0,
		.data = &clk_data_MAIN_OBSCLK_DIV_out0.data_div.data,
		.type = CLK_TYPE_DIV,
	},
	[CLK_AM62LX_MAIN_OBSCLK_OUTMUX_SEL_OUT0] = {
		.drv = &clk_drv_mux_reg.drv,
		.flags = 0,
		.data = &clk_data_MAIN_OBSCLK_OUTMUX_SEL_out0.data_mux.data,
		.type = CLK_TYPE_MUX,
	},
	[CLK_AM62LX_MAIN_OSPI_REF_CLK_SEL_OUT0] = {
		.drv = &clk_drv_mux_reg.drv,
		.flags = 0,
		.data = &clk_data_MAIN_OSPI_REF_CLK_SEL_out0.data_mux.data,
		.type = CLK_TYPE_MUX,
	},
	[CLK_AM62LX_MAIN_TIMERCLKN_SEL_OUT0] = {
		.drv = &clk_drv_mux_reg.drv,
		.flags = 0,
		.data = &clk_data_MAIN_TIMERCLKn_SEL_out0.data_mux.data,
		.type = CLK_TYPE_MUX,
	},
	[CLK_AM62LX_MAIN_TIMERCLKN_SEL_OUT1] = {
		.drv = &clk_drv_mux_reg.drv,
		.flags = 0,
		.data = &clk_data_MAIN_TIMERCLKn_SEL_out1.data_mux.data,
		.type = CLK_TYPE_MUX,
	},
	[CLK_AM62LX_MAIN_TIMERCLKN_SEL_OUT2] = {
		.drv = &clk_drv_mux_reg.drv,
		.flags = 0,
		.data = &clk_data_MAIN_TIMERCLKn_SEL_out2.data_mux.data,
		.type = CLK_TYPE_MUX,
	},
	[CLK_AM62LX_MAIN_TIMERCLKN_SEL_OUT3] = {
		.drv = &clk_drv_mux_reg.drv,
		.flags = 0,
		.data = &clk_data_MAIN_TIMERCLKn_SEL_out3.data_mux.data,
		.type = CLK_TYPE_MUX,
	},
	[CLK_AM62LX_MAIN_USART_CLKDIV_OUT0] = {
		.parent = {
			CLK_AM62LX_HSDIV4_16FFT_WKUP_0_HSDIVOUT2_CLK,
			1,
		},
		.drv = &clk_drv_div_reg_go.drv,
		.flags = 0,
		.data = &clk_data_MAIN_USART_CLKDIV_out0.data_div.data,
		.type = CLK_TYPE_DIV,
	},
	[CLK_AM62LX_MAIN_USART_CLKDIV_OUT1] = {
		.parent = {
			CLK_AM62LX_HSDIV4_16FFT_WKUP_0_HSDIVOUT2_CLK,
			1,
		},
		.drv = &clk_drv_div_reg_go.drv,
		.flags = 0,
		.data = &clk_data_MAIN_USART_CLKDIV_out1.data_div.data,
		.type = CLK_TYPE_DIV,
	},
	[CLK_AM62LX_MAIN_USART_CLKDIV_OUT2] = {
		.parent = {
			CLK_AM62LX_HSDIV4_16FFT_WKUP_0_HSDIVOUT2_CLK,
			1,
		},
		.drv = &clk_drv_div_reg_go.drv,
		.flags = 0,
		.data = &clk_data_MAIN_USART_CLKDIV_out2.data_div.data,
		.type = CLK_TYPE_DIV,
	},
	[CLK_AM62LX_MAIN_USART_CLKDIV_OUT3] = {
		.parent = {
			CLK_AM62LX_HSDIV4_16FFT_WKUP_0_HSDIVOUT2_CLK,
			1,
		},
		.drv = &clk_drv_div_reg_go.drv,
		.flags = 0,
		.data = &clk_data_MAIN_USART_CLKDIV_out3.data_div.data,
		.type = CLK_TYPE_DIV,
	},
	[CLK_AM62LX_MAIN_USART_CLKDIV_OUT4] = {
		.parent = {
			CLK_AM62LX_HSDIV4_16FFT_WKUP_0_HSDIVOUT2_CLK,
			1,
		},
		.drv = &clk_drv_div_reg_go.drv,
		.flags = 0,
		.data = &clk_data_MAIN_USART_CLKDIV_out4.data_div.data,
		.type = CLK_TYPE_DIV,
	},
	[CLK_AM62LX_MAIN_USART_CLKDIV_OUT5] = {
		.parent = {
			CLK_AM62LX_HSDIV4_16FFT_WKUP_0_HSDIVOUT2_CLK,
			1,
		},
		.drv = &clk_drv_div_reg_go.drv,
		.flags = 0,
		.data = &clk_data_MAIN_USART_CLKDIV_out5.data_div.data,
		.type = CLK_TYPE_DIV,
	},
	[CLK_AM62LX_MAIN_USART_CLKDIV_OUT6] = {
		.parent = {
			CLK_AM62LX_HSDIV4_16FFT_WKUP_0_HSDIVOUT2_CLK,
			1,
		},
		.drv = &clk_drv_div_reg_go.drv,
		.flags = 0,
		.data = &clk_data_MAIN_USART_CLKDIV_out6.data_div.data,
		.type = CLK_TYPE_DIV,
	},
	[CLK_AM62LX_MCASPN_CLKSEL_AUXCLK_OUT0] = {
		.drv = &clk_drv_mux_reg.drv,
		.flags = 0,
		.data = &clk_data_MCASPn_CLKSEL_AUXCLK_out0.data_mux.data,
		.type = CLK_TYPE_MUX,
	},
	[CLK_AM62LX_MCASPN_CLKSEL_AUXCLK_OUT1] = {
		.drv = &clk_drv_mux_reg.drv,
		.flags = 0,
		.data = &clk_data_MCASPn_CLKSEL_AUXCLK_out1.data_mux.data,
		.type = CLK_TYPE_MUX,
	},
	[CLK_AM62LX_MCASPN_CLKSEL_AUXCLK_OUT2] = {
		.drv = &clk_drv_mux_reg.drv,
		.flags = 0,
		.data = &clk_data_MCASPn_CLKSEL_AUXCLK_out2.data_mux.data,
		.type = CLK_TYPE_MUX,
	},
	[CLK_AM62LX_WKUP_CLKOUT_SEL_OUT0] = {
		.drv = &clk_drv_mux_reg.drv,
		.flags = 0,
		.data = &clk_data_WKUP_CLKOUT_SEL_out0.data_mux.data,
		.type = CLK_TYPE_MUX,
	},
	[CLK_AM62LX_WKUP_CLKOUT_SEL_IO_OUT0] = {
		.drv = &clk_drv_mux_reg.drv,
		.flags = 0,
		.data = &clk_data_WKUP_CLKOUT_SEL_IO_out0.data_mux.data,
		.type = CLK_TYPE_MUX,
	},
	[CLK_AM62LX_WKUP_GPIO0_CLKSEL_OUT0] = {
		.drv = &clk_drv_mux_reg.drv,
		.flags = 0,
		.data = &clk_data_WKUP_GPIO0_CLKSEL_out0.data_mux.data,
		.type = CLK_TYPE_MUX,
	},
	[CLK_AM62LX_WKUP_GTCCLK_SEL_OUT0] = {
		.drv = &clk_drv_mux_reg.drv,
		.flags = 0,
		.data = &clk_data_WKUP_GTCCLK_SEL_out0.data_mux.data,
		.type = CLK_TYPE_MUX,
	},
	[CLK_AM62LX_WKUP_GTC_OUTMUX_SEL_OUT0] = {
		.drv = &clk_drv_mux_reg.drv,
		.flags = 0,
		.data = &clk_data_WKUP_GTC_OUTMUX_SEL_out0.data_mux.data,
		.type = CLK_TYPE_MUX,
	},
	[CLK_AM62LX_WKUP_OBSCLK_MUX_SEL_OUT0] = {
		.drv = &clk_drv_mux_reg.drv,
		.flags = 0,
		.data = &clk_data_WKUP_OBSCLK_MUX_SEL_out0.data_mux.data,
		.type = CLK_TYPE_MUX,
	},
	[CLK_AM62LX_WKUP_OBSCLK_OUTMUX_SEL_OUT0] = {
		.drv = &clk_drv_mux_reg.drv,
		.flags = 0,
		.data = &clk_data_WKUP_OBSCLK_OUTMUX_SEL_out0.data_mux.data,
		.type = CLK_TYPE_MUX,
	},
	[CLK_AM62LX_WKUP_TIMERCLKN_SEL_OUT0] = {
		.drv = &clk_drv_mux_reg.drv,
		.flags = 0,
		.data = &clk_data_WKUP_TIMERCLKn_SEL_out0.data_mux.data,
		.type = CLK_TYPE_MUX,
	},
	[CLK_AM62LX_WKUP_TIMERCLKN_SEL_OUT1] = {
		.drv = &clk_drv_mux_reg.drv,
		.flags = 0,
		.data = &clk_data_WKUP_TIMERCLKn_SEL_out1.data_mux.data,
		.type = CLK_TYPE_MUX,
	},
	[CLK_AM62LX_MAIN_TIMER1_CASCADE_OUT0] = {
		.drv = &clk_drv_mux_reg.drv,
		.flags = 0,
		.data = &clk_data_MAIN_TIMER1_CASCADE_out0.data_mux.data,
		.type = CLK_TYPE_MUX,
	},
	[CLK_AM62LX_MAIN_TIMER3_CASCADE_OUT0] = {
		.drv = &clk_drv_mux_reg.drv,
		.flags = 0,
		.data = &clk_data_MAIN_TIMER3_CASCADE_out0.data_mux.data,
		.type = CLK_TYPE_MUX,
	},
	[CLK_AM62LX_WKUP_TIMER1_CASCADE_OUT0] = {
		.drv = &clk_drv_mux_reg.drv,
		.flags = 0,
		.data = &clk_data_WKUP_TIMER1_CASCADE_out0.data_mux.data,
		.type = CLK_TYPE_MUX,
	},
	[CLK_AM62LX_SAM62_PLL_CTRL_WRAP_WKUP_0_CHIP_DIV24_CLK_CLK] = {
		.drv = &clk_drv_div_reg.drv,
		.flags = 0,
		.data = &clk_data_sam62_pll_ctrl_wrap_wkup_0_chip_div24_clk_clk.data_div.data,
		.parent = {
			CLK_AM62LX_SAM62_PLL_CTRL_WRAP_WKUP_0_SYSCLKOUT_CLK,
			1,
		},
	},
};

struct clk soc_clocks[178];
const size_t soc_clock_count = ARRAY_SIZE(soc_clock_data);
