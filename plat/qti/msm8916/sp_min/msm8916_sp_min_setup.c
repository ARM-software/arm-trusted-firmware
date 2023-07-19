/*
 * Copyright (c) 2022-2023, Stephan Gerhold <stephan@gerhold.net>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <common/debug.h>
#include <lib/xlat_tables/xlat_mmu_helpers.h>
#include <platform_sp_min.h>

#include "../msm8916_config.h"
#include "../msm8916_setup.h"

static struct {
	entry_point_info_t bl33;
} image_ep_info = {
	/* BL33 entry point */
	SET_STATIC_PARAM_HEAD(bl33, PARAM_EP, VERSION_1,
			      entry_point_info_t, NON_SECURE),
	.bl33.pc = PRELOADED_BL33_BASE,
	.bl33.spsr = SPSR_MODE32(MODE32_hyp, SPSR_T_ARM, SPSR_E_LITTLE,
				 DISABLE_ALL_EXCEPTIONS),
};

void sp_min_early_platform_setup2(u_register_t arg0, u_register_t arg1,
				  u_register_t arg2, u_register_t arg3)
{
	msm8916_early_platform_setup();
	msm8916_configure_early();
}

void sp_min_plat_arch_setup(void)
{
	msm8916_plat_arch_setup(BL32_BASE, BL32_END - BL32_BASE);
	enable_mmu_svc_mon(0);
}

void sp_min_platform_setup(void)
{
	INFO("SP_MIN: Platform setup start\n");
	msm8916_platform_setup();
	msm8916_configure();
	INFO("SP_MIN: Platform setup done\n");
}

entry_point_info_t *sp_min_plat_get_bl33_ep_info(void)
{
	return &image_ep_info.bl33;
}
