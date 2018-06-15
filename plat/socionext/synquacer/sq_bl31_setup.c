/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_helpers.h>
#include <platform_def.h>
#include <assert.h>
#include <bl_common.h>
#include <debug.h>

void bl31_early_platform_setup(bl31_params_t *from_bl2,
				void *plat_params_from_bl2)
{
	/* There are no parameters from BL2 if BL31 is a reset vector */
	assert(from_bl2 == NULL);
	assert(plat_params_from_bl2 == NULL);
}

void bl31_platform_setup(void)
{
}

void bl31_plat_runtime_setup(void)
{
}

void bl31_plat_arch_setup(void)
{
}
