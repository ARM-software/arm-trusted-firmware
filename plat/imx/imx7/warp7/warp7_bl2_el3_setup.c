/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <assert.h>
#include <bl_common.h>
#include <caam.h>
#include <console.h>
#include <debug.h>
#include <desc_image_load.h>
#include <emmc.h>
#include <mxc_usdhc.h>
#include <mmio.h>
#include <optee_utils.h>
#include <platform_def.h>
#include <utils.h>
#include <xlat_mmu_helpers.h>
#include <xlat_tables_defs.h>
#include <aips.h>
#include <clock.h>
#include <csu.h>
#include <mxc_gpt.h>
#include <io_mux.h>
#include <mxc_console.h>
#include <snvs.h>
#include <wdog.h>
#include "warp7_private.h"

#define UART1_CLK_SELECT (CCM_TARGET_ROOT_ENABLE |\
			  CCM_TRGT_MUX_UART1_CLK_ROOT_OSC_24M)

#define USDHC_CLK_SELECT (CCM_TARGET_ROOT_ENABLE |\
			  CCM_TRGT_MUX_NAND_USDHC_BUS_CLK_ROOT_AHB |\
			  CCM_TARGET_POST_PODF(2))

#define WDOG_CLK_SELECT (CCM_TARGET_ROOT_ENABLE |\
			 CCM_TRGT_MUX_WDOG_CLK_ROOT_OSC_24M)

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

static void warp7_setup_pinmux(void)
{
	/* Configure UART1 TX */
	io_muxc_set_pad_alt_function(IOMUXC_SW_MUX_CTL_PAD_UART1_TX_DATA_OFFSET,
				     WARP7_UART1_TX_MUX);
	io_muxc_set_pad_features(IOMUXC_SW_PAD_CTL_PAD_UART1_TX_DATA_OFFSET,
				 WARP7_UART1_TX_FEATURES);

	/* Configure UART1 RX */
	io_muxc_set_pad_alt_function(IOMUXC_SW_MUX_CTL_PAD_UART1_RX_DATA_OFFSET,
				     WARP7_UART1_RX_MUX);
	io_muxc_set_pad_features(IOMUXC_SW_PAD_CTL_PAD_UART1_RX_DATA_OFFSET,
				 WARP7_UART1_RX_FEATURES);
}

static void warp7_usdhc_setup(void)
{
	mxc_usdhc_params_t params;

	zeromem(&params, sizeof(mxc_usdhc_params_t));
	params.reg_base = PLAT_WARP7_BOOT_EMMC_BASE;
	params.clk_rate = 25000000;
	params.bus_width = EMMC_BUS_WIDTH_8;
	mxc_usdhc_init(&params);
}

static void warp7_setup_system_counter(void)
{
	unsigned long freq = SYS_COUNTER_FREQ_IN_TICKS;

	/* Set the frequency table index to our target frequency */
	asm volatile("mcr p15, 0, %0, c14, c0, 0" : : "r" (freq));

	/* Enable system counter @ frequency table index 0, halt on debug */
	mmio_write_32(SYS_CNTCTL_BASE + CNTCR_OFF,
		      CNTCR_FCREQ(0) | CNTCR_HDBG | CNTCR_EN);
}

static void warp7_setup_wdog_clocks(void)
{
	uint32_t wdog_en_bits = (uint32_t)WDOG_CLK_SELECT;

	clock_set_wdog_clk_root_bits(wdog_en_bits);
	clock_enable_wdog(0);
	clock_enable_wdog(1);
	clock_enable_wdog(2);
	clock_enable_wdog(3);
}

/*
 * bl2_early_platform_setup()
 * MMU off
 */
void bl2_el3_early_platform_setup(u_register_t arg1, u_register_t arg2,
				  u_register_t arg3, u_register_t arg4)
{
	uint32_t uart_en_bits = (uint32_t)UART1_CLK_SELECT;
	uint32_t usdhc_clock_sel = PLAT_WARP7_SD - 1;

	/* Initialize the AIPS */
	aips_init();
	csu_init();
	snvs_init();
	mxc_gpt_ops_init(GPT1_BASE_ADDR);

	/* Initialize clocks, regulators, pin-muxes etc */
	clock_init();
	clock_enable_uart(0, uart_en_bits);
	clock_enable_usdhc(usdhc_clock_sel, USDHC_CLK_SELECT);
	warp7_setup_system_counter();
	warp7_setup_wdog_clocks();

	/* Setup pin-muxes */
	warp7_setup_pinmux();

	/* Init UART, storage and friends */
	console_init(PLAT_WARP7_BOOT_UART_BASE, PLAT_WARP7_BOOT_UART_CLK_IN_HZ,
		     PLAT_WARP7_CONSOLE_BAUDRATE);
	warp7_usdhc_setup();

	/* Open handles to persistent storage */
	plat_warp7_io_setup();

	/* Setup higher-level functionality CAAM, RTC etc */
	caam_init();
	wdog_init();

	/* Print out the expected memory map */
	VERBOSE("\tOPTEE      0x%08x-0x%08x\n", WARP7_OPTEE_BASE, WARP7_OPTEE_LIMIT);
	VERBOSE("\tATF/BL2    0x%08x-0x%08x\n", BL2_RAM_BASE, BL2_RAM_LIMIT);
	VERBOSE("\tSHRAM      0x%08x-0x%08x\n", SHARED_RAM_BASE, SHARED_RAM_LIMIT);
	VERBOSE("\tFIP        0x%08x-0x%08x\n", WARP7_FIP_BASE, WARP7_FIP_LIMIT);
	VERBOSE("\tDTB        0x%08x-0x%08x\n", WARP7_DTB_BASE, WARP7_DTB_LIMIT);
	VERBOSE("\tUBOOT/BL33 0x%08x-0x%08x\n", WARP7_UBOOT_BASE, WARP7_UBOOT_LIMIT);
}

/*
 * bl2_platform_setup()
 * MMU on - enabled by bl2_el3_plat_arch_setup()
 */
void bl2_platform_setup(void)
{
}
