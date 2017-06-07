/*
 * Copyright (c) 2013-2014, ARM Limited and Contributors. All rights reserved.
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

/*******************************************************************************
 * Set the Secure EL1 required architectural state
 ******************************************************************************/
void bl1_arch_next_el_setup(void)
{
	unsigned long next_sctlr;

	/* Use the same endianness than the current BL */
	next_sctlr = (read_sctlr_el3() & SCTLR_EE_BIT);

	/* Set SCTLR Secure EL1 */
	next_sctlr |= SCTLR_EL1_RES1;

	write_sctlr_el1(next_sctlr);
}
