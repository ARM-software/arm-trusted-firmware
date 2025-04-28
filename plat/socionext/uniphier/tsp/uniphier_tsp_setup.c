/*
 * Copyright (c) 2017-2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>

#include <platform_def.h>

#include <common/bl_common.h>
#include <plat/common/platform.h>

#include "../uniphier.h"

static unsigned int uniphier_soc = UNIPHIER_SOC_UNKNOWN;

void tsp_early_platform_setup(u_register_t arg0, u_register_t arg1,
			      u_register_t arg2, u_register_t arg3)
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
}
