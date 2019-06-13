/*
 * Copyright (c) 2018-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <platform_def.h>

#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <common/desc_image_load.h>
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
#include <imx_uart.h>
#include <imx_snvs.h>
#include <imx_wdog.h>
#include <imx7_def.h>

#ifndef AARCH32_SP_OPTEE
#error "Must build with OPTEE support included"
#endif

uintptr_t plat_get_ns_image_entrypoint(void)
{
	return IMX7_UBOOT_BASE;
}

static uint32_t imx7_get_spsr_for_bl32_entry(void)
{
	return SPSR_MODE32(MODE32_svc, SPSR_T_ARM, SPSR_E_LITTLE,
			   DISABLE_ALL_EXCEPTIONS);
}

static uint32_t imx7_get_spsr_for_bl33_entry(void)
{
	return SPSR_MODE32(MODE32_svc,
			   plat_get_ns_image_entrypoint() & 0x1,
			   SPSR_E_LITTLE, DISABLE_ALL_EXCEPTIONS);
}

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
		bl_mem_params->ep_info.spsr = imx7_get_spsr_for_bl32_entry();
		break;

	case BL33_IMAGE_ID:
		/* AArch32 only core: OP-TEE expects NSec EP in register LR */
		pager_mem_params = get_bl_mem_params_node(BL32_IMAGE_ID);
		assert(pager_mem_params);
		pager_mem_params->ep_info.lr_svc = bl_mem_params->ep_info.pc;

		/* BL33 expects to receive the primary CPU MPID (through r0) */
		bl_mem_params->ep_info.args.arg0 = 0xffff & read_mpidr();
		bl_mem_params->ep_info.spsr = imx7_get_spsr_for_bl33_entry();
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

static void imx7_setup_system_counter(void)
{
	unsigned long freq = SYS_COUNTER_FREQ_IN_TICKS;

	/* Set the frequency table index to our target frequency */
	write_cntfrq(freq);

	/* Enable system counter @ frequency table index 0, halt on debug */
	mmio_write_32(SYS_CNTCTL_BASE + CNTCR_OFF,
		      CNTCR_FCREQ(0) | CNTCR_HDBG | CNTCR_EN);
}

static void imx7_setup_wdog_clocks(void)
{
	uint32_t wdog_en_bits = (uint32_t)WDOG_DEFAULT_CLK_SELECT;

	imx_clock_set_wdog_clk_root_bits(wdog_en_bits);
	imx_clock_enable_wdog(0);
	imx_clock_enable_wdog(1);
	imx_clock_enable_wdog(2);
	imx_clock_enable_wdog(3);
}


/*
 * bl2_el3_early_platform_setup()
 * MMU off
 */
void bl2_el3_early_platform_setup(u_register_t arg1, u_register_t arg2,
				  u_register_t arg3, u_register_t arg4)
{
	static console_imx_uart_t console;
	int console_scope = CONSOLE_FLAG_BOOT | CONSOLE_FLAG_RUNTIME;

	/* Initialize common components */
	imx_aips_init();
	imx_csu_init();
	imx_snvs_init();
	imx_gpt_ops_init(GPT1_BASE_ADDR);
	imx_clock_init();
	imx7_setup_system_counter();
	imx7_setup_wdog_clocks();

	/* Platform specific setup */
	imx7_platform_setup(arg1, arg2, arg3, arg4);

	/* Init UART, clock should be enabled in imx7_platform_setup() */
	console_imx_uart_register(PLAT_IMX7_BOOT_UART_BASE,
				  PLAT_IMX7_BOOT_UART_CLK_IN_HZ,
				  PLAT_IMX7_CONSOLE_BAUDRATE,
				  &console);
	console_set_scope(&console.console, console_scope);

	/* Open handles to persistent storage */
	plat_imx7_io_setup();

	/* Setup higher-level functionality CAAM, RTC etc */
	imx_caam_init();
	imx_wdog_init();

	/* Print out the expected memory map */
	VERBOSE("\tOPTEE       0x%08x-0x%08x\n", IMX7_OPTEE_BASE, IMX7_OPTEE_LIMIT);
	VERBOSE("\tATF/BL2     0x%08x-0x%08x\n", BL2_RAM_BASE, BL2_RAM_LIMIT);
	VERBOSE("\tSHRAM       0x%08x-0x%08x\n", SHARED_RAM_BASE, SHARED_RAM_LIMIT);
	VERBOSE("\tFIP         0x%08x-0x%08x\n", IMX7_FIP_BASE, IMX7_FIP_LIMIT);
	VERBOSE("\tDTB-OVERLAY 0x%08x-0x%08x\n", IMX7_DTB_OVERLAY_BASE, IMX7_DTB_OVERLAY_LIMIT);
	VERBOSE("\tDTB         0x%08x-0x%08x\n", IMX7_DTB_BASE, IMX7_DTB_LIMIT);
	VERBOSE("\tUBOOT/BL33  0x%08x-0x%08x\n", IMX7_UBOOT_BASE, IMX7_UBOOT_LIMIT);
}

/*
 * bl2_platform_setup()
 * MMU on - enabled by bl2_el3_plat_arch_setup()
 */
void bl2_platform_setup(void)
{
}
