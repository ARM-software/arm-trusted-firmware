/*
 * Copyright (c) 2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "../../../../bl1/bl1_private.h"
#include <arch.h>

#include <fvp_r_arch_helpers.h>

/*******************************************************************************
 * Function that does the first bit of architectural setup that affects
 * execution in the non-secure address space.
 ******************************************************************************/
void bl1_arch_setup(void)
{
	/* v8-R64 does not include SCRs. */
}
