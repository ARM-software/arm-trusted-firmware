/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "plat_ls.h"

void tsp_early_platform_setup(void)
{
	ls_tsp_early_platform_setup();

	/*Todo: Initialize the platform config for future decision making */
}
