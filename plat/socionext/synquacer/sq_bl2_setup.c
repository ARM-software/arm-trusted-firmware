/*
 * Copyright (c) 2022, Socionext Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>

#include <common/bl_common.h>
#include <common/debug.h>
#include <common/desc_image_load.h>
#include <common/image_decompress.h>
#include <drivers/arm/pl011.h>
#include <drivers/io/io_storage.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <plat/common/platform.h>

#include <platform_def.h>
#include <sq_common.h>

static console_t console;

void bl2_el3_early_platform_setup(u_register_t x0, u_register_t x1,
				  u_register_t x2, u_register_t x3)
{
	/* Initialize the console to provide early debug support */
	(void)console_pl011_register(PLAT_SQ_BOOT_UART_BASE,
			       PLAT_SQ_BOOT_UART_CLK_IN_HZ,
			       SQ_CONSOLE_BAUDRATE, &console);
	console_set_scope(&console, CONSOLE_FLAG_BOOT);
}

void bl2_el3_plat_arch_setup(void)
{
	int ret;

	sq_mmap_setup(BL2_BASE, BL2_SIZE, NULL);

	ret = sq_io_setup();
	if (ret) {
		ERROR("failed to setup io devices\n");
		plat_error_handler(ret);
	}
}

void bl2_platform_setup(void)
{
}

void plat_flush_next_bl_params(void)
{
	flush_bl_params_desc();
}

bl_load_info_t *plat_get_bl_image_load_info(void)
{
	return get_bl_load_info_from_mem_params_desc();
}

bl_params_t *plat_get_next_bl_params(void)
{
	return get_next_bl_params_from_mem_params_desc();
}

void bl2_plat_preload_setup(void)
{
}

int bl2_plat_handle_pre_image_load(unsigned int image_id)
{
	struct image_info *image_info;

	image_info = sq_get_image_info(image_id);

	return mmap_add_dynamic_region(image_info->image_base,
				      image_info->image_base,
				      image_info->image_max_size,
				      MT_MEMORY | MT_RW | MT_NS);
}

int bl2_plat_handle_post_image_load(unsigned int image_id)
{
	return 0;
}
