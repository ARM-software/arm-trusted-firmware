/*
 * Copyright (c) 2013-2014, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_helpers.h>

/*******************************************************************************
 * Place holder function to perform any S-EL1 specific architectural setup. At
 * the moment there is nothing to do.
 ******************************************************************************/
void bl2_arch_setup(void)
{
	/* Give access to FP/SIMD registers */
	write_cpacr(CPACR_EL1_FPEN(CPACR_EL1_FP_TRAP_NONE));
}
