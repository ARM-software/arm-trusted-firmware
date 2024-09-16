/*
 * Copyright (c) 2023-2024, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdint.h>

#include <common/bl_common.h>
#include <drivers/st/stm32_console.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <plat/common/platform.h>

#include <platform_def.h>

void bl31_early_platform_setup2(u_register_t arg0, u_register_t arg1,
				u_register_t arg2, u_register_t arg3)
{
	bl_params_t *params_from_bl2;
	int ret;

	/*
	 * Invalidate remaining data from second half of SYSRAM (used by BL2) as this area will
	 * be later used as non-secure.
	 */
	inv_dcache_range(STM32MP_SYSRAM_BASE + STM32MP_SYSRAM_SIZE / 2U,
			 STM32MP_SYSRAM_SIZE / 2U);

	mmap_add_region(BL_CODE_BASE, BL_CODE_BASE,
			BL_CODE_END - BL_CODE_BASE,
			MT_CODE | MT_SECURE);

#if USE_COHERENT_MEM
	/* Map coherent memory */
	mmap_add_region(BL_COHERENT_RAM_BASE, BL_COHERENT_RAM_BASE,
			BL_COHERENT_RAM_END - BL_COHERENT_RAM_BASE,
			MT_DEVICE | MT_RW | MT_SECURE);
#endif

	configure_mmu();

	/*
	 * Map upper SYSRAM where bl_params_t are stored in BL2
	 */
	ret = mmap_add_dynamic_region(STM32MP_SYSRAM_BASE + STM32MP_SYSRAM_SIZE / 2U,
				      STM32MP_SYSRAM_BASE + STM32MP_SYSRAM_SIZE / 2U,
				      STM32MP_SYSRAM_SIZE / 2U, MT_RO_DATA | MT_SECURE);
	if (ret < 0) {
		ERROR("BL2 params area mapping: %d\n", ret);
		panic();
	}

	assert(arg0 != 0UL);
	params_from_bl2 = (bl_params_t *)arg0;
	assert(params_from_bl2 != NULL);
	assert(params_from_bl2->h.type == PARAM_BL_PARAMS);
	assert(params_from_bl2->h.version >= VERSION_2);

	bl_params_node_t *bl_params = params_from_bl2->head;

	while (bl_params != NULL) {
		bl_params = bl_params->next_params_info;
	}

	ret = mmap_remove_dynamic_region(STM32MP_SYSRAM_BASE + STM32MP_SYSRAM_SIZE / 2U,
					 STM32MP_SYSRAM_SIZE / 2U);
	if (ret < 0) {
		ERROR("BL2 params area unmapping: %d\n", ret);
		panic();
	}
}

void bl31_plat_arch_setup(void)
{
}

void bl31_platform_setup(void)
{
}

entry_point_info_t *bl31_plat_get_next_image_ep_info(unsigned int type)
{
	return NULL;
}
