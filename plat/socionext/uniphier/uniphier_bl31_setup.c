/*
 * Copyright (c) 2017-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>

#include <platform_def.h>

#include <arch.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <drivers/console.h>
#include <lib/mmio.h>
#include <plat/common/platform.h>

#include "uniphier.h"

static entry_point_info_t bl32_image_ep_info;
static entry_point_info_t bl33_image_ep_info;
static unsigned int uniphier_soc = UNIPHIER_SOC_UNKNOWN;

entry_point_info_t *bl31_plat_get_next_image_ep_info(uint32_t type)
{
	assert(sec_state_is_valid(type));
	return type == NON_SECURE ? &bl33_image_ep_info : &bl32_image_ep_info;
}

void bl31_early_platform_setup2(u_register_t arg0, u_register_t arg1,
				u_register_t arg2, u_register_t arg3)
{
	void *from_bl2;

	from_bl2 = (void *)arg0;

	bl_params_node_t *bl_params = ((bl_params_t *)from_bl2)->head;

	uniphier_soc = uniphier_get_soc_id();
	if (uniphier_soc == UNIPHIER_SOC_UNKNOWN)
		plat_error_handler(-ENOTSUP);

	uniphier_console_setup(uniphier_soc);

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

static const uintptr_t uniphier_cntctl_base[] = {
	[UNIPHIER_SOC_LD11] = 0x60e00000,
	[UNIPHIER_SOC_LD20] = 0x60e00000,
	[UNIPHIER_SOC_PXS3] = 0x60e00000,
};

void bl31_platform_setup(void)
{
	uintptr_t cntctl_base;

	uniphier_cci_init(uniphier_soc);
	uniphier_cci_enable();

	/* Initialize the GIC driver, cpu and distributor interfaces */
	uniphier_gic_driver_init(uniphier_soc);
	uniphier_gic_init();

	assert(uniphier_soc < ARRAY_SIZE(uniphier_cntctl_base));
	cntctl_base = uniphier_cntctl_base[uniphier_soc];

	/* Enable and initialize the System level generic timer */
	mmio_write_32(cntctl_base + CNTCR_OFF, CNTCR_FCREQ(0U) | CNTCR_EN);

	uniphier_psci_init(uniphier_soc);
}

void bl31_plat_arch_setup(void)
{
	uniphier_mmap_setup(uniphier_soc);
}
