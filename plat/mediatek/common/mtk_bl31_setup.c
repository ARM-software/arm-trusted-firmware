/*
 * Copyright (c) 2022, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <arch.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <drivers/generic_delay_timer.h>
#if XLAT_TABLES_LIB_V2 && PLAT_XLAT_TABLES_DYNAMIC
#include <lib/xlat_tables/xlat_tables_v2.h>
#endif
#include <plat/common/platform.h>

#if COREBOOT
#include <common/desc_image_load.h>

#include <drivers/ti/uart/uart_16550.h>
#include <lib/coreboot.h>
#include <plat_params.h>
#endif

/* MTK headers */
#if MTK_SIP_KERNEL_BOOT_ENABLE
#include <cold_boot.h>
#endif
#include <lib/mtk_init/mtk_init.h>
#include <mtk_mmap_pool.h>

IMPORT_SYM(uintptr_t, __RW_START__, RW_START);
IMPORT_SYM(uintptr_t, __DATA_START__, DATA_START);

#if COREBOOT
static entry_point_info_t bl32_ep_info;
static entry_point_info_t bl33_ep_info;

/*******************************************************************************
 * Return a pointer to the 'entry_point_info' structure of the next image for
 * the security state specified. BL33 corresponds to the non-secure image type
 * while BL32 corresponds to the secure image type. A NULL pointer is returned
 * if the image does not exist.
 ******************************************************************************/
entry_point_info_t *bl31_plat_get_next_image_ep_info(uint32_t type)
{
	entry_point_info_t *next_image_info;

	next_image_info = (type == NON_SECURE) ? &bl33_ep_info : &bl32_ep_info;
	assert(next_image_info->h.type == PARAM_EP);

	/* None of the images on this platform can have 0x0 as the entrypoint */
	if (next_image_info->pc) {
		return next_image_info;
	} else {
		return NULL;
	}
}
#else
#ifndef MTK_BL31_AS_BL2
static struct mtk_bl31_fw_config bl31_fw_config;
#else
struct mtk_bl31_fw_config bl31_fw_config;
#endif
/* In order to be accessed after MMU enable */
static struct mtk_bl_param_t bl_param_clone;

void *get_mtk_bl31_fw_config(int index)
{
	void *arg = NULL;

	switch (index) {
	case BOOT_ARG_FROM_BL2:
		arg = bl31_fw_config.from_bl2;
		break;
	case BOOT_ARG_SOC_FW_CONFIG:
		arg = bl31_fw_config.soc_fw_config;
		break;
	case BOOT_ARG_HW_CONFIG:
		arg = bl31_fw_config.hw_config;
		break;
	case BOOT_ARG_RESERVED:
		arg = bl31_fw_config.reserved;
		break;
	default:
		WARN("Fail to get boot arg, index:%d", index);
		break;
	}
	return arg;
}
#endif
/*****************************************************************************
 * Perform the very early platform specific architectural setup shared between
 * ARM standard platforms. This only does basic initialization. Later
 * architectural setup (bl31_arch_setup()) does not do anything platform
 * specific.
 ******************************************************************************/
void bl31_early_platform_setup2(u_register_t from_bl2,
				u_register_t soc_fw_config,
				u_register_t hw_config, u_register_t plat_params_from_bl2)

{
#if COREBOOT
	static console_t console;

	params_early_setup(soc_fw_config);
	if (coreboot_serial.type) {
		console_16550_register(coreboot_serial.baseaddr,
				       coreboot_serial.input_hertz,
				       coreboot_serial.baud,
				       &console);
	}
	bl31_params_parse_helper(from_bl2, &bl32_ep_info, &bl33_ep_info);
#else
	struct mtk_bl_param_t *p_mtk_bl_param = (struct mtk_bl_param_t *)from_bl2;

	if (p_mtk_bl_param == NULL) {
		ERROR("from_bl2 should not be NULL\n");
		panic();
	}
	memcpy(&bl_param_clone, p_mtk_bl_param, sizeof(struct mtk_bl_param_t));
	bl31_fw_config.from_bl2 = (void *)&bl_param_clone;
	bl31_fw_config.soc_fw_config = (void *)soc_fw_config;
	bl31_fw_config.hw_config = (void *)hw_config;
	bl31_fw_config.reserved = (void *)plat_params_from_bl2;
#endif

	INFO("MTK BL31 start\n");
	/* Init delay function */
	generic_delay_timer_init();
	/* Initialize module initcall */
	mtk_init_one_level(MTK_INIT_LVL_EARLY_PLAT);
}

void bl31_plat_arch_setup(void)
{
	const mmap_region_t bl_regions[] = {
		MAP_BL_RO,
		MAP_BL_RW,
#if USE_COHERENT_MEM
		MAP_BL_COHERENT_RAM,
#endif
		{0},
	};

	mtk_xlat_init(bl_regions);
	/* Initialize module initcall */
	mtk_init_one_level(MTK_INIT_LVL_ARCH);
}

/*****************************************************************************
 * Perform any BL31 platform setup common to ARM standard platforms
 ******************************************************************************/

void bl31_platform_setup(void)
{
	mtk_init_one_level(MTK_INIT_LVL_PLAT_SETUP_0);
	mtk_init_one_level(MTK_INIT_LVL_PLAT_SETUP_1);
}

/*******************************************************************************
 * Operations before cold CPU leave BL31.
 * Switch console to runtime state.
 ******************************************************************************/
void bl31_plat_runtime_setup(void)
{
	mtk_init_one_level(MTK_INIT_LVL_PLAT_RUNTIME);
	console_switch_state(CONSOLE_FLAG_RUNTIME);
}

unsigned int plat_get_syscnt_freq2(void)
{
	return SYS_COUNTER_FREQ_IN_HZ;
}
