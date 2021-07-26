/*
 * Copyright (c) 2018-2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <platform_def.h>

#include <common/debug.h>
#include <drivers/console.h>
#include <drivers/mmc.h>
#include <lib/utils.h>

#include <imx_caam.h>
#include <imx_clock.h>
#include <imx_io_mux.h>
#include <imx_uart.h>
#include <imx_usdhc.h>
#include <imx7_def.h>

#define UART1_CLK_SELECT (CCM_TARGET_ROOT_ENABLE |\
			  CCM_TRGT_MUX_UART1_CLK_ROOT_OSC_24M)

#define UART6_CLK_SELECT (CCM_TARGET_ROOT_ENABLE |\
			  CCM_TRGT_MUX_UART6_CLK_ROOT_OSC_24M)

#define USDHC_CLK_SELECT (CCM_TARGET_ROOT_ENABLE |\
			  CCM_TRGT_MUX_NAND_USDHC_BUS_CLK_ROOT_AHB |\
			  CCM_TARGET_POST_PODF(2))

#define USB_CLK_SELECT (CCM_TARGET_ROOT_ENABLE |\
			CCM_TRGT_MUX_USB_HSIC_CLK_ROOT_SYS_PLL)

#define WARP7_UART1_TX_MUX \
	IOMUXC_SW_MUX_CTL_PAD_UART1_TX_DATA_ALT0_UART1_TX_DATA

#define WARP7_UART1_TX_FEATURES \
	(IOMUXC_SW_PAD_CTL_PAD_UART1_TX_DATA_PS_3_100K_PU	| \
	 IOMUXC_SW_PAD_CTL_PAD_UART1_TX_DATA_PE_EN		| \
	 IOMUXC_SW_PAD_CTL_PAD_UART1_TX_DATA_HYS_EN		| \
	 IOMUXC_SW_PAD_CTL_PAD_UART1_TX_DATA_DSE_1_X4)

#define WARP7_UART1_RX_MUX \
	IOMUXC_SW_MUX_CTL_PAD_UART1_RX_DATA_ALT0_UART1_RX_DATA

#define WARP7_UART1_RX_FEATURES \
	(IOMUXC_SW_PAD_CTL_PAD_UART1_RX_DATA_PS_3_100K_PU	| \
	 IOMUXC_SW_PAD_CTL_PAD_UART1_RX_DATA_PE_EN		| \
	 IOMUXC_SW_PAD_CTL_PAD_UART1_RX_DATA_HYS_EN		| \
	 IOMUXC_SW_PAD_CTL_PAD_UART1_RX_DATA_DSE_1_X4)

#define WARP7_UART6_TX_MUX \
	IOMUXC_SW_MUX_CTL_PAD_ECSPI1_MOSI_ALT1_UART6_TX_DATA

#define WARP7_UART6_TX_FEATURES \
	(IOMUXC_SW_PAD_CTL_PAD_ECSPI1_MOSI_PS_3_100K_PU		| \
	 IOMUXC_SW_PAD_CTL_PAD_ECSPI1_MOSI_PE_EN		| \
	 IOMUXC_SW_PAD_CTL_PAD_ECSPI1_MOSI_HYS_EN		| \
	 IOMUXC_SW_PAD_CTL_PAD_ECSPI1_MOSI_DSE_1_X4)

#define WARP7_UART6_RX_MUX \
	IOMUXC_SW_MUX_CTL_PAD_ECSPI1_SCLK_ALT1_UART6_RX_DATA

#define WARP7_UART6_RX_FEATURES \
	(IOMUXC_SW_PAD_CTL_PAD_ECSPI1_SCLK_PS_3_100K_PU		| \
	 IOMUXC_SW_PAD_CTL_PAD_ECSPI1_SCLK_PE_EN		| \
	 IOMUXC_SW_PAD_CTL_PAD_ECSPI1_SCLK_HYS_EN		| \
	 IOMUXC_SW_PAD_CTL_PAD_ECSPI1_SCLK_DSE_1_X4)

static struct mmc_device_info mmc_info;

static void warp7_setup_pinmux(void)
{
	/* Configure UART1 TX */
	imx_io_muxc_set_pad_alt_function(IOMUXC_SW_MUX_CTL_PAD_UART1_TX_DATA_OFFSET,
					 WARP7_UART1_TX_MUX);
	imx_io_muxc_set_pad_features(IOMUXC_SW_PAD_CTL_PAD_UART1_TX_DATA_OFFSET,
				     WARP7_UART1_TX_FEATURES);

	/* Configure UART1 RX */
	imx_io_muxc_set_pad_alt_function(IOMUXC_SW_MUX_CTL_PAD_UART1_RX_DATA_OFFSET,
					 WARP7_UART1_RX_MUX);
	imx_io_muxc_set_pad_features(IOMUXC_SW_PAD_CTL_PAD_UART1_RX_DATA_OFFSET,
				     WARP7_UART1_RX_FEATURES);

	/* Configure UART6 TX */
	imx_io_muxc_set_pad_alt_function(IOMUXC_SW_MUX_CTL_PAD_ECSPI1_MOSI_OFFSET,
					 WARP7_UART6_TX_MUX);
	imx_io_muxc_set_pad_features(IOMUXC_SW_PAD_CTL_PAD_ECSPI1_MOSI_OFFSET,
				     WARP7_UART6_TX_FEATURES);

	/* Configure UART6 RX */
	imx_io_muxc_set_pad_alt_function(IOMUXC_SW_MUX_CTL_PAD_ECSPI1_SCLK_OFFSET,
					 WARP7_UART6_RX_MUX);
	imx_io_muxc_set_pad_features(IOMUXC_SW_PAD_CTL_PAD_ECSPI1_SCLK_OFFSET,
				     WARP7_UART6_RX_FEATURES);
}

static void warp7_usdhc_setup(void)
{
	imx_usdhc_params_t params;

	zeromem(&params, sizeof(imx_usdhc_params_t));
	params.reg_base = PLAT_WARP7_BOOT_MMC_BASE;
	params.clk_rate = 25000000;
	params.bus_width = MMC_BUS_WIDTH_8;
	mmc_info.mmc_dev_type = MMC_IS_EMMC;
	imx_usdhc_init(&params, &mmc_info);
}

static void warp7_setup_usb_clocks(void)
{
	uint32_t usb_en_bits = (uint32_t)USB_CLK_SELECT;

	imx_clock_set_usb_clk_root_bits(usb_en_bits);
	imx_clock_enable_usb(CCM_CCGR_ID_USB_IPG);
	imx_clock_enable_usb(CCM_CCGR_ID_USB_PHY_480MCLK);
	imx_clock_enable_usb(CCM_CCGR_ID_USB_OTG1_PHY);
	imx_clock_enable_usb(CCM_CCGR_ID_USB_OTG2_PHY);
}

void imx7_platform_setup(u_register_t arg1, u_register_t arg2,
			 u_register_t arg3, u_register_t arg4)
{
	uint32_t uart1_en_bits = (uint32_t)UART1_CLK_SELECT;
	uint32_t uart6_en_bits = (uint32_t)UART6_CLK_SELECT;
	uint32_t usdhc_clock_sel = PLAT_WARP7_SD - 1;

	/* Initialize clocks etc */
	imx_clock_enable_uart(0, uart1_en_bits);
	imx_clock_enable_uart(5, uart6_en_bits);

	imx_clock_enable_usdhc(usdhc_clock_sel, USDHC_CLK_SELECT);

	warp7_setup_usb_clocks();

	/* Setup pin-muxes */
	warp7_setup_pinmux();

	warp7_usdhc_setup();
}
