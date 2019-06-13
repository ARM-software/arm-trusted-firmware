/*
 * Copyright (c) 2018-2019, ARM Limited and Contributors. All rights reserved.
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

#define UART5_CLK_SELECT (CCM_TARGET_ROOT_ENABLE |\
			  CCM_TRGT_MUX_UART5_CLK_ROOT_OSC_24M)

#define USDHC_CLK_SELECT (CCM_TARGET_ROOT_ENABLE |\
			  CCM_TRGT_MUX_NAND_USDHC_BUS_CLK_ROOT_AHB |\
			  CCM_TARGET_POST_PODF(2))

#define USB_CLK_SELECT (CCM_TARGET_ROOT_ENABLE |\
			CCM_TRGT_MUX_USB_HSIC_CLK_ROOT_SYS_PLL)

#define PICOPI_UART5_RX_MUX \
	IOMUXC_SW_MUX_CTL_PAD_I2C4_SCL_ALT1_UART5_RX_DATA

#define PICOPI_UART5_TX_MUX \
	IOMUXC_SW_MUX_CTL_PAD_I2C4_SDA_ALT1_UART5_TX_DATA

#define PICOPI_SD3_FEATURES \
	(IOMUXC_SW_PAD_CTL_PAD_SD3_PU_47K            | \
	 IOMUXC_SW_PAD_CTL_PAD_SD3_PE                | \
	 IOMUXC_SW_PAD_CTL_PAD_SD3_HYS               | \
	 IOMUXC_SW_PAD_CTL_PAD_SD3_SLEW_SLOW         | \
	 IOMUXC_SW_PAD_CTL_PAD_SD3_DSE_3_X6)

static void picopi_setup_pinmux(void)
{
	/* Configure UART5 TX */
	imx_io_muxc_set_pad_alt_function(IOMUXC_SW_MUX_CTL_PAD_I2C4_SDA_OFFSET,
					 PICOPI_UART5_TX_MUX);
	/* Configure UART5 RX */
	imx_io_muxc_set_pad_alt_function(IOMUXC_SW_MUX_CTL_PAD_I2C4_SCL_OFFSET,
					 PICOPI_UART5_RX_MUX);

	/* Configure USDHC3 */
	imx_io_muxc_set_pad_alt_function(IOMUXC_SW_MUX_CTL_PAD_SD3_CLK_OFFSET, 0);
	imx_io_muxc_set_pad_alt_function(IOMUXC_SW_MUX_CTL_PAD_SD3_CMD_OFFSET, 0);
	imx_io_muxc_set_pad_alt_function(IOMUXC_SW_MUX_CTL_PAD_SD3_DATA0_OFFSET, 0);
	imx_io_muxc_set_pad_alt_function(IOMUXC_SW_MUX_CTL_PAD_SD3_DATA1_OFFSET, 0);
	imx_io_muxc_set_pad_alt_function(IOMUXC_SW_MUX_CTL_PAD_SD3_DATA2_OFFSET, 0);
	imx_io_muxc_set_pad_alt_function(IOMUXC_SW_MUX_CTL_PAD_SD3_DATA3_OFFSET, 0);
	imx_io_muxc_set_pad_alt_function(IOMUXC_SW_MUX_CTL_PAD_SD3_DATA4_OFFSET, 0);
	imx_io_muxc_set_pad_alt_function(IOMUXC_SW_MUX_CTL_PAD_SD3_DATA5_OFFSET, 0);
	imx_io_muxc_set_pad_alt_function(IOMUXC_SW_MUX_CTL_PAD_SD3_DATA6_OFFSET, 0);
	imx_io_muxc_set_pad_alt_function(IOMUXC_SW_MUX_CTL_PAD_SD3_DATA7_OFFSET, 0);
	imx_io_muxc_set_pad_alt_function(IOMUXC_SW_MUX_CTL_PAD_GPIO1_IO14_OFFSET,
					 IOMUXC_SW_MUX_CTL_PAD_GPIO1_IO14_ALT1_SD3_CD_B);

	imx_io_muxc_set_pad_features(IOMUXC_SW_PAD_CTL_PAD_SD3_CLK_OFFSET,
				     PICOPI_SD3_FEATURES);
	imx_io_muxc_set_pad_features(IOMUXC_SW_PAD_CTL_PAD_SD3_CMD_OFFSET,
				     PICOPI_SD3_FEATURES);
	imx_io_muxc_set_pad_features(IOMUXC_SW_PAD_CTL_PAD_SD3_DATA0_OFFSET,
				     PICOPI_SD3_FEATURES);
	imx_io_muxc_set_pad_features(IOMUXC_SW_PAD_CTL_PAD_SD3_DATA1_OFFSET,
				     PICOPI_SD3_FEATURES);
	imx_io_muxc_set_pad_features(IOMUXC_SW_PAD_CTL_PAD_SD3_DATA2_OFFSET,
				     PICOPI_SD3_FEATURES);
	imx_io_muxc_set_pad_features(IOMUXC_SW_PAD_CTL_PAD_SD3_DATA3_OFFSET,
				     PICOPI_SD3_FEATURES);
	imx_io_muxc_set_pad_features(IOMUXC_SW_PAD_CTL_PAD_SD3_DATA4_OFFSET,
				     PICOPI_SD3_FEATURES);
	imx_io_muxc_set_pad_features(IOMUXC_SW_PAD_CTL_PAD_SD3_DATA5_OFFSET,
				     PICOPI_SD3_FEATURES);
	imx_io_muxc_set_pad_features(IOMUXC_SW_PAD_CTL_PAD_SD3_DATA6_OFFSET,
				     PICOPI_SD3_FEATURES);
	imx_io_muxc_set_pad_features(IOMUXC_SW_PAD_CTL_PAD_SD3_DATA7_OFFSET,
				     PICOPI_SD3_FEATURES);
	imx_io_muxc_set_pad_features(IOMUXC_SW_PAD_CTL_PAD_GPIO1_IO14_OFFSET,
				     PICOPI_SD3_FEATURES);
}

static void picopi_usdhc_setup(void)
{
	imx_usdhc_params_t params;
	struct mmc_device_info info;

	zeromem(&params, sizeof(imx_usdhc_params_t));
	params.reg_base = PLAT_PICOPI_BOOT_MMC_BASE;
	params.clk_rate = 25000000;
	params.bus_width = MMC_BUS_WIDTH_8;
	info.mmc_dev_type = MMC_IS_EMMC;
	imx_usdhc_init(&params, &info);
}

static void picopi_setup_usb_clocks(void)
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
	uint32_t uart5_en_bits = (uint32_t)UART5_CLK_SELECT;
	uint32_t usdhc_clock_sel = PLAT_PICOPI_SD - 1;

	/* Initialize clocks etc */
	imx_clock_enable_uart(4, uart5_en_bits);
	imx_clock_enable_usdhc(usdhc_clock_sel, USDHC_CLK_SELECT);

	picopi_setup_usb_clocks();

	/* Setup pin-muxes */
	picopi_setup_pinmux();

	picopi_usdhc_setup();
}
