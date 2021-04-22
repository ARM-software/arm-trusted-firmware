/*
 * Copyright 2017-2021 NXP
 * Copyright 2021 Arm
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <common/desc_image_load.h>
#include <context.h>
#include <drivers/console.h>
#include <drivers/generic_delay_timer.h>
#include <drivers/mmc.h>
#include <lib/mmio.h>
#include <lib/optee_utils.h>
#include <lib/utils.h>
#include <stdbool.h>
#include <tbbr_img_def.h>

#include <imx_aipstz.h>
#include <imx_csu.h>
#include <imx_uart.h>
#include <imx_usdhc.h>
#include <plat/common/platform.h>

#include "imx8mm_private.h"
#include "platform_def.h"

static const struct aipstz_cfg aipstz[] = {
	{IMX_AIPSTZ1, 0x77777777, 0x77777777, .opacr = {0x0, 0x0, 0x0, 0x0, 0x0}, },
	{IMX_AIPSTZ2, 0x77777777, 0x77777777, .opacr = {0x0, 0x0, 0x0, 0x0, 0x0}, },
	{IMX_AIPSTZ3, 0x77777777, 0x77777777, .opacr = {0x0, 0x0, 0x0, 0x0, 0x0}, },
	{IMX_AIPSTZ4, 0x77777777, 0x77777777, .opacr = {0x0, 0x0, 0x0, 0x0, 0x0}, },
	{0},
};

static void imx8mm_usdhc_setup(void)
{
	imx_usdhc_params_t params;
	struct mmc_device_info info;

	params.reg_base = PLAT_IMX8MM_BOOT_MMC_BASE;
	/*
	   The imx8mm SD Card Speed modes for USDHC2
	   +--------------+--------------------+--------------+--------------+
	   |Bus Speed Mode|Max. Clock Frequency|Max. Bus Speed|Signal Voltage|
	   +--------------+--------------------+--------------+--------------+
	   |Default Speed | 25 MHz             | 12.5 MB/s    | 3.3V         |
	   |High Speed    | 50 MHz             | 25 MB/s      | 3.3V         |
	   +--------------+--------------------+--------------+--------------+

	   We pick 50 Mhz here for High Speed access.
	*/
	params.clk_rate = 50000000;
	params.bus_width = MMC_BUS_WIDTH_1;
	params.flags = 0;
	info.mmc_dev_type = MMC_IS_SD;
	info.ocr_voltage = OCR_3_3_3_4 | OCR_3_2_3_3;
	imx_usdhc_init(&params, &info);
}

void bl2_el3_early_platform_setup(u_register_t arg1, u_register_t arg2,
				  u_register_t arg3, u_register_t arg4)
{
	int i;
	static console_t console;

	/* enable CSU NS access permission */
	for (i = 0; i < MAX_CSU_NUM; i++) {
		mmio_write_32(IMX_CSU_BASE + i * 4, CSU_CSL_OPEN_ACCESS);
	}

	/* config the aips access permission */
	imx_aipstz_init(aipstz);

	console_imx_uart_register(IMX_BOOT_UART_BASE, IMX_BOOT_UART_CLK_IN_HZ,
		IMX_CONSOLE_BAUDRATE, &console);

	generic_delay_timer_init();

	/* select the CKIL source to 32K OSC */
	mmio_write_32(0x30360124, 0x1);

	imx8mm_usdhc_setup();

	/* Open handles to a FIP image */
	plat_imx8mm_io_setup();
}

void bl2_el3_plat_arch_setup(void)
{
}

void bl2_platform_setup(void)
{
}

int bl2_plat_handle_post_image_load(unsigned int image_id)
{
	int err = 0;
	bl_mem_params_node_t *bl_mem_params = get_bl_mem_params_node(image_id);
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
		if (err != 0) {
			WARN("OPTEE header parse error.\n");
		}

		break;
	default:
		/* Do nothing in default case */
		break;
	}

	return err;
}

unsigned int plat_get_syscnt_freq2(void)
{
	return COUNTER_FREQUENCY;
}

void bl2_plat_runtime_setup(void)
{
	return;
}
