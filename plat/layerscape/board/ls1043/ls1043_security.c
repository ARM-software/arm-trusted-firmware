/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "plat_ls.h"

/*
 * We assume that all security programming is done by the primary core.
 */
void plat_ls_security_setup(void)
{
	tzc380_setup();
}
