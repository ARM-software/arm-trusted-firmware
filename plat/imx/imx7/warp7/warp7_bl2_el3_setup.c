/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <platform_def.h>

#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <common/desc_image_load.h>
#include <drivers/console.h>
#include <drivers/mmc.h>
#include <lib/xlat_tables/xlat_mmu_helpers.h>
#include <lib/xlat_tables/xlat_tables_defs.h>
#include <lib/mmio.h>
#include <lib/optee_utils.h>
#include <lib/utils.h>

#include <imx_aips.h>
#include <imx_caam.h>
#include <imx_clock.h>
#include <imx_csu.h>
#include <imx_gpt.h>
#include <imx_io_mux.h>
#include <imx_uart.h>
#include <imx_snvs.h>
#include <imx_usdhc.h>
#include <imx_wdog.h>

#include "warp7_private.h"

#define UART1_CLK_SELECT (CCM_TARGET_ROOT_ENABLE |\
			  CCM_TRGT_MUX_UART1_CLK_ROOT_OSC_24M)

#define UART6_CLK_SELECT (CCM_TARGET_ROOT_ENABLE |\
			  CCM_TRGT_MUX_UART6_CLK_ROOT_OSC_24M)

#define USDHC_CLK_SELECT (CCM_TARGET_ROOT_ENABLE |\
			  CCM_TRGT_MUX_NAND_USDHC_BUS_CLK_ROOT_AHB |\
			  CCM_TARGET_POST_PODF(2))

#define WDOG_CLK_SELECT (CCM_TARGET_ROOT_ENABLE |\
			 CCM_TRGT_MUX_WDOG_CLK_ROOT_OSC_24M)

#define USB_CLK_SELECT (CCM_TARGET_ROOT_ENABLE |\
			CCM_TRGT_MUX_USB_HSIC_CLK_ROOT_SYS_PLL)

uintptr_t plat_get_ns_image_entrypoint(void)
{
	return WARP7_UBOOT_BASE;
}

static uint32_t warp7_get_spsr_for_bl32_entry(void)
{
	return SPSR_MODE32(MODE32_svc, SPSR_T_ARM, SPSR_E_LITTLE,
			   DISABLE_ALL_EXCEPTIONS);
}

static uint32_t warp7_get_spsr_for_bl33_entry(void)
{
	return SPSR_MODE32(MODE32_svc,
			   plat_get_ns_image_entrypoint() & 0x1,
			   SPSR_E_LITTLE, DISABLE_ALL_EXCEPTIONS);
}

#ifndef AARCH32_SP_OPTEE
#error "Must build with OPTEE support included"
#endif

int bl2_plat_handle_post_image_load(unsigned int image_id)
{
	int err = 0;
	bl_mem_params_node_t *bl_mem_params = get_bl_mem_params_node(image_id);
	bl_mem_params_node_t *hw_cfg_mem_params = NULL;

	bl_mem_params_node_t *pager_mem_params = NULL;
	bl_mem_params_node_t *paged_mem_params = NULL;

	assert(bl_mem_params);

	switch (image_id) {
	case BL32_IMAGE_ID:
		pager_mem_params = get_bl_mem_params_node(BL32_EXTRA1_IMAGE_ID);
		assert(pager_mem_params);

		paged_mem_params = get_bl_mem_params_node(BL32_EXTRA2_IMAGE_ID);
		assert(paged_mem_params);

		err = parse_optee_header(&bl_mem_params->ep_info,
					 &pager_mem_params->image_info,
					 &paged_mem_params->image_info);
		if (err != 0)
			WARN("OPTEE header parse error.\n");

		/*
		 * When ATF loads the DTB the address of the DTB is passed in
		 * arg2, if an hw config image is present use the base address
		 * as DTB address an pass it as arg2
		 */
		hw_cfg_mem_params = get_bl_mem_params_node(HW_CONFIG_ID);

		bl_mem_params->ep_info.args.arg0 =
					bl_mem_params->ep_info.args.arg1;
		bl_mem_params->ep_info.args.arg1 = 0;
		if (hw_cfg_mem_params)
			bl_mem_params->ep_info.args.arg2 =
					hw_cfg_mem_params->image_info.image_base;
		else
			bl_mem_params->ep_info.args.arg2 = 0;
		bl_mem_params->ep_info.args.arg3 = 0;
		bl_mem_params->ep_info.spsr = warp7_get_spsr_for_bl32_entry();
		break;

	case BL33_IMAGE_ID:
		/* AArch32 only core: OP-TEE expects NSec EP in register LR */
		pager_mem_params = get_bl_mem_params_node(BL32_IMAGE_ID);
		assert(pager_mem_params);
		pager_mem_params->ep_info.lr_svc = bl_mem_params->ep_info.pc;

		/* BL33 expects to receive the primary CPU MPID (through r0) */
		bl_mem_params->ep_info.args.arg0 = 0xffff & read_mpidr();
		bl_mem_params->ep_info.spsr = warp7_get_spsr_for_bl33_entry();
		break;

	default:
		/* Do nothing in default case */
		break;
	}

	return err;
}

void bl2_el3_plat_arch_setup(void)
{
	/* Setup the MMU here */
}

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
	struct mmc_device_info info;

	zeromem(&params, sizeof(imx_usdhc_params_t));
	params.reg_base = PLAT_WARP7_BOOT_MMC_BASE;
	params.clk_rate = 25000000;
	params.bus_width = MMC_BUS_WIDTH_8;
	info.mmc_dev_type = MMC_IS_EMMC;
	imx_usdhc_init(&params, &info);
}

static void warp7_setup_system_counter(void)
{
	unsigned long freq = SYS_COUNTER_FREQ_IN_TICKS;

	/* Set the frequency table index to our target frequency */
	write_cntfrq(freq);

	/* Enable system counter @ frequency table index 0, halt on debug */
	mmio_write_32(SYS_CNTCTL_BASE + CNTCR_OFF,
		      CNTCR_FCREQ(0) | CNTCR_HDBG | CNTCR_EN);
}

static void warp7_setup_wdog_clocks(void)
{
	uint32_t wdog_en_bits = (uint32_t)WDOG_CLK_SELECT;

	imx_clock_set_wdog_clk_root_bits(wdog_en_bits);
	imx_clock_enable_wdog(0);
	imx_clock_enable_wdog(1);
	imx_clock_enable_wdog(2);
	imx_clock_enable_wdog(3);
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
/*
 * bl2_el3_early_platform_setup()
 * MMU off
 */
void bl2_el3_early_platform_setup(u_register_t arg1, u_register_t arg2,
				  u_register_t arg3, u_register_t arg4)
{
	uint32_t uart1_en_bits = (uint32_t)UART1_CLK_SELECT;
	uint32_t uart6_en_bits = (uint32_t)UART6_CLK_SELECT;
	uint32_t usdhc_clock_sel = PLAT_WARP7_SD - 1;
	static console_imx_uart_t console;
	int console_scope = CONSOLE_FLAG_BOOT | CONSOLE_FLAG_RUNTIME;

	/* Initialize the AIPS */
	imx_aips_init();
	imx_csu_init();
	imx_snvs_init();
	imx_gpt_ops_init(GPT1_BASE_ADDR);

	/* Initialize clocks, regulators, pin-muxes etc */
	imx_clock_init();
	imx_clock_enable_uart(0, uart1_en_bits);
	imx_clock_enable_uart(5, uart6_en_bits);
	imx_clock_enable_usdhc(usdhc_clock_sel, USDHC_CLK_SELECT);
	warp7_setup_system_counter();
	warp7_setup_wdog_clocks();
	warp7_setup_usb_clocks();

	/* Setup pin-muxes */
	warp7_setup_pinmux();

	/* Init UART, storage and friends */
	console_imx_uart_register(PLAT_WARP7_BOOT_UART_BASE,
				  PLAT_WARP7_BOOT_UART_CLK_IN_HZ,
				  PLAT_WARP7_CONSOLE_BAUDRATE,
				  &console);
	console_set_scope(&console.console, console_scope);

	warp7_usdhc_setup();

	/* Open handles to persistent storage */
	plat_warp7_io_setup();

	/* Setup higher-level functionality CAAM, RTC etc */
	imx_caam_init();
	imx_wdog_init();

	/* Print out the expected memory map */
	VERBOSE("\tOPTEE       0x%08x-0x%08x\n", WARP7_OPTEE_BASE, WARP7_OPTEE_LIMIT);
	VERBOSE("\tATF/BL2     0x%08x-0x%08x\n", BL2_RAM_BASE, BL2_RAM_LIMIT);
	VERBOSE("\tSHRAM       0x%08x-0x%08x\n", SHARED_RAM_BASE, SHARED_RAM_LIMIT);
	VERBOSE("\tFIP         0x%08x-0x%08x\n", WARP7_FIP_BASE, WARP7_FIP_LIMIT);
	VERBOSE("\tDTB-OVERLAY 0x%08x-0x%08x\n", WARP7_DTB_OVERLAY_BASE, WARP7_DTB_OVERLAY_LIMIT);
	VERBOSE("\tDTB         0x%08x-0x%08x\n", WARP7_DTB_BASE, WARP7_DTB_LIMIT);
	VERBOSE("\tUBOOT/BL33  0x%08x-0x%08x\n", WARP7_UBOOT_BASE, WARP7_UBOOT_LIMIT);
}

/*
 * bl2_platform_setup()
 * MMU on - enabled by bl2_el3_plat_arch_setup()
 */
void bl2_platform_setup(void)
{
}
