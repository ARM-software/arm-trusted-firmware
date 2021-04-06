/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdbool.h>

#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <common/desc_image_load.h>
#include <common/tbbr/tbbr_img_def.h>
#include <context.h>
#include <drivers/arm/tzc380.h>
#include <drivers/console.h>
#include <drivers/generic_delay_timer.h>
#include <drivers/mmc.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <lib/mmio.h>
#include <lib/optee_utils.h>
#include <lib/xlat_tables/xlat_tables_v2.h>

#include <imx8m_caam.h>
#include "imx8mp_private.h"
#include <imx_aipstz.h>
#include <imx_rdc.h>
#include <imx_uart.h>
#include <plat/common/platform.h>
#include <plat_imx8.h>
#include <platform_def.h>


static const struct aipstz_cfg aipstz[] = {
	{IMX_AIPSTZ1, 0x77777777, 0x77777777, .opacr = {0x0, 0x0, 0x0, 0x0, 0x0}, },
	{IMX_AIPSTZ2, 0x77777777, 0x77777777, .opacr = {0x0, 0x0, 0x0, 0x0, 0x0}, },
	{IMX_AIPSTZ3, 0x77777777, 0x77777777, .opacr = {0x0, 0x0, 0x0, 0x0, 0x0}, },
	{IMX_AIPSTZ4, 0x77777777, 0x77777777, .opacr = {0x0, 0x0, 0x0, 0x0, 0x0}, },
	{0},
};

void bl2_el3_early_platform_setup(u_register_t arg0, u_register_t arg1,
		u_register_t arg2, u_register_t arg3)
{
	static console_t console;
	unsigned int i;

	/* Enable CSU NS access permission */
	for (i = 0U; i < 64; i++) {
		mmio_write_32(IMX_CSU_BASE + i * 4, 0x00ff00ff);
	}

	imx_aipstz_init(aipstz);

	console_imx_uart_register(IMX_BOOT_UART_BASE, IMX_BOOT_UART_CLK_IN_HZ,
		IMX_CONSOLE_BAUDRATE, &console);

	generic_delay_timer_init();

	/* select the CKIL source to 32K OSC */
	mmio_write_32(IMX_ANAMIX_BASE + ANAMIX_MISC_CTL, 0x1);

	/* Open handles to a FIP image */
	plat_imx_io_setup();
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
