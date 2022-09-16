/*
 * Copyright (c) 2021-2023, Stephan Gerhold <stephan@gerhold.net>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <arch.h>
#include <common/debug.h>
#include <plat/common/platform.h>

#include "msm8916_config.h"
#include "msm8916_setup.h"

static struct {
	entry_point_info_t bl32;
	entry_point_info_t bl33;
} image_ep_info = {
	/* BL32 entry point */
	SET_STATIC_PARAM_HEAD(bl32, PARAM_EP, VERSION_1,
			      entry_point_info_t, SECURE),
	.bl32.pc = BL32_BASE,

	/* BL33 entry point */
	SET_STATIC_PARAM_HEAD(bl33, PARAM_EP, VERSION_1,
			      entry_point_info_t, NON_SECURE),
	.bl33.pc = PRELOADED_BL33_BASE,
	.bl33.spsr = SPSR_64(MODE_EL2, MODE_SP_ELX, DISABLE_ALL_EXCEPTIONS),
};

void bl31_early_platform_setup2(u_register_t arg0, u_register_t arg1,
				u_register_t arg2, u_register_t arg3)
{
	msm8916_early_platform_setup();
	msm8916_configure_early();
}

void bl31_plat_arch_setup(void)
{
	msm8916_plat_arch_setup(BL31_BASE, BL31_END - BL31_BASE);
	enable_mmu_el3(0);
}

void bl31_platform_setup(void)
{
	INFO("BL31: Platform setup start\n");
	msm8916_platform_setup();
	msm8916_configure();
	INFO("BL31: Platform setup done\n");
}

entry_point_info_t *bl31_plat_get_next_image_ep_info(uint32_t type)
{
	switch (type) {
	case SECURE:
		return &image_ep_info.bl32;
	case NON_SECURE:
		return &image_ep_info.bl33;
	default:
		assert(sec_state_is_valid(type));
		return NULL;
	}
}
