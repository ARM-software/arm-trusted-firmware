/*
 * Copyright (c) 2017-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>

#include <common/bl_common.h>
#include <lib/xlat_tables/xlat_mmu_helpers.h>

#include "../uniphier.h"

void tsp_early_platform_setup(void)
{
	uniphier_console_setup();
}

void tsp_platform_setup(void)
{
}

void tsp_plat_arch_setup(void)
{
	uniphier_mmap_setup();
	enable_mmu_el1(0);
}
