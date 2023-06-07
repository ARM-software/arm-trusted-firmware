/*
 * Copyright (c) 2013-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_helpers.h>
#include "../bl1_private.h"

/*******************************************************************************
 * Function that does the first bit of architectural setup that affects
 * execution in the non-secure address space.
 ******************************************************************************/
void bl1_arch_setup(void)
{
	/* Set the next EL to be AArch64 */
	write_scr_el3(read_scr_el3() | SCR_RW_BIT);
}
