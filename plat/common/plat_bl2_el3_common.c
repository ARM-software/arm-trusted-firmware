/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <assert.h>
#include <bl_common.h>
#include <debug.h>
#include <errno.h>
#include <platform_def.h>

/*
 * The following platform functions are weakly defined. They
 * are default implementations that allow BL2 to compile in
 * absence of real definitions. The Platforms may override
 * with more complex definitions.
 */
#pragma weak bl2_el3_plat_prepare_exit

void bl2_el3_plat_prepare_exit(void)
{
}
