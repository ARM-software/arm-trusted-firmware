/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <assert.h>
#include <bl_common.h>
#include <console.h>
#include <debug.h>
#include <errno.h>
#include <mmio.h>
#include <platform.h>
#include <platform_def.h>
#include <xlat_mmu_helpers.h>

#include "uniphier.h"

#define BL31_END		(unsigned long)(&__BL31_END__)
#define BL31_SIZE		((BL31_END) - (BL31_BASE))

static entry_point_info_t bl32_image_ep_info;
static entry_point_info_t bl33_image_ep_info;

entry_point_info_t *bl31_plat_get_next_image_ep_info(uint32_t type)
{
	assert(sec_state_is_valid(type));
	return type == NON_SECURE ? &bl33_image_ep_info : &bl32_image_ep_info;
}

void bl31_early_platform_setup(void *from_bl2, void *plat_params_from_bl2)
{
	bl_params_node_t *bl_params = ((bl_params_t *)from_bl2)->head;

	uniphier_console_setup();

	while (bl_params) {
		if (bl_params->image_id == BL32_IMAGE_ID)
			bl32_image_ep_info = *bl_params->ep_info;

		if (bl_params->image_id == BL33_IMAGE_ID)
			bl33_image_ep_info = *bl_params->ep_info;

		bl_params = bl_params->next_params_info;
	}

	if (bl33_image_ep_info.pc == 0)
		panic();
}

#define UNIPHIER_SYS_CNTCTL_BASE	0x60E00000

void bl31_platform_setup(void)
{
	unsigned int soc;

	soc = uniphier_get_soc_id();
	if (soc == UNIPHIER_SOC_UNKNOWN) {
		ERROR("unsupported SoC\n");
		plat_error_handler(-ENOTSUP);
	}

	uniphier_cci_init(soc);
	uniphier_cci_enable();

	/* Initialize the GIC driver, cpu and distributor interfaces */
	uniphier_gic_driver_init(soc);
	uniphier_gic_init();

	/* Enable and initialize the System level generic timer */
	mmio_write_32(UNIPHIER_SYS_CNTCTL_BASE + CNTCR_OFF,
			CNTCR_FCREQ(0) | CNTCR_EN);
}

void bl31_plat_arch_setup(void)
{
	uniphier_mmap_setup(BL31_BASE, BL31_SIZE, NULL);
	enable_mmu_el3(0);
}

void bl31_plat_runtime_setup(void)
{
	/* Suppress any runtime logs unless DEBUG is defined */
#if !DEBUG
	console_uninit();
#endif
}
