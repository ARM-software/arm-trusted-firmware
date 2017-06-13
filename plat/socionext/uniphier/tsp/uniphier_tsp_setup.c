/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>
#include <xlat_mmu_helpers.h>

#include "../uniphier.h"

#define BL32_END		(unsigned long)(&__BL32_END__)
#define BL32_SIZE		((BL32_END) - (BL32_BASE))

void tsp_early_platform_setup(void)
{
	uniphier_console_setup();
}

void tsp_platform_setup(void)
{
}

void tsp_plat_arch_setup(void)
{
	uniphier_mmap_setup(BL32_BASE, BL32_SIZE, NULL);
	enable_mmu_el1(0);
}
