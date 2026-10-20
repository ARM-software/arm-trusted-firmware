/*
 * Copyright (c) 2025, Qualcomm Technologies, Inc. and/or its subsidiaries.
 * Copyright (c) 2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>

#include <common/bl_common.h>
#include <common/debug.h>
#include <common/desc_image_load.h>
#include <common/image_decompress.h>
#include <common/tbbr/tbbr_img_def.h>
#include <drivers/io/io_storage.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <plat/common/platform.h>

#include <platform_def.h>
#include <qti_plat.h>
#include <qti_uart_console.h>

static console_t g_qti_console_uart;

void bl2_early_platform_setup2(u_register_t x0, u_register_t x1,
			       u_register_t x2, u_register_t x3)
{
	qti_console_uart_register(&g_qti_console_uart,
				  PLAT_QTI_UART_BASE);
	console_set_scope(&g_qti_console_uart,
			  CONSOLE_FLAG_BOOT | CONSOLE_FLAG_CRASH);
}

void bl2_plat_arch_setup(void)
{
	int ret;

	qti_setup_page_tables(BL2_BASE,
			      BL2_SIZE,
			      BL_CODE_BASE,
			      BL_CODE_END,
			      BL_RO_DATA_BASE,
			      BL_RO_DATA_END);
	enable_mmu_el3(0);

	ret = qti_io_setup();
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

	image_info = qti_get_image_info(image_id);

	return mmap_add_dynamic_region(image_info->image_base,
				      image_info->image_base,
				      image_info->image_max_size,
				      MT_MEMORY | MT_RW | MT_NS);
}

int bl2_plat_handle_post_image_load(unsigned int image_id)
{
#if defined(SPD_spmd)
	/*
	 * The SPMC core manifest is loaded as TOS_FW_CONFIG and BL31/SPMD
	 * expects its address in the BL32 entry point's arg0. This is only
	 * wired up when building with SPD=spmd.
	 */
	bl_mem_params_node_t *tos_fw_config;
	bl_mem_params_node_t *bl32;

	if (image_id != BL32_IMAGE_ID) {
		return 0;
	}

	tos_fw_config = get_bl_mem_params_node(TOS_FW_CONFIG_ID);
	if (tos_fw_config == NULL) {
		return 0;
	}

	bl32 = get_bl_mem_params_node(BL32_IMAGE_ID);
	bl32->ep_info.args.arg0 = tos_fw_config->image_info.image_base;
#endif
	return 0;
}
