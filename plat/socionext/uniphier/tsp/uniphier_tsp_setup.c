/*
 * Copyright (c) 2017-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>

#include <platform_def.h>

#include <common/bl_common.h>
#include <lib/xlat_tables/xlat_mmu_helpers.h>
#include <plat/common/platform.h>

#include "../uniphier.h"

static unsigned int uniphier_soc = UNIPHIER_SOC_UNKNOWN;

void tsp_early_platform_setup(void)
{
	uniphier_soc = uniphier_get_soc_id();
	if (uniphier_soc == UNIPHIER_SOC_UNKNOWN)
		plat_error_handler(-ENOTSUP);

	uniphier_console_setup(uniphier_soc);
}

void tsp_platform_setup(void)
{
}

void tsp_plat_arch_setup(void)
{
	uniphier_mmap_setup(uniphier_soc);
	enable_mmu_el1(0);
}
