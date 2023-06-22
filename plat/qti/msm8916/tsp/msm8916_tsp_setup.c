/*
 * Copyright (c) 2023, Stephan Gerhold <stephan@gerhold.net>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <platform_tsp.h>

#include "../msm8916_setup.h"
#include <platform_def.h>

void tsp_early_platform_setup(void)
{
	msm8916_early_platform_setup();
}

void tsp_plat_arch_setup(void)
{
	msm8916_plat_arch_setup(BL32_BASE, BL32_END - BL32_BASE);
	enable_mmu_el1(0);
}

void tsp_platform_setup(void)
{
	INFO("TSP: Platform setup start\n");
	msm8916_platform_setup();
	INFO("TSP: Platform setup done\n");

	console_switch_state(CONSOLE_FLAG_RUNTIME);
}
