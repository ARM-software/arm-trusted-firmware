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

/*******************************************************************************
 * Set the Secure EL1 required architectural state
 ******************************************************************************/
void bl1_arch_next_el_setup(void)
{
	u_register_t next_sctlr;

	/* Use the same endianness than the current BL */
	next_sctlr = (read_sctlr_el2() & SCTLR_EE_BIT);

	/* Set SCTLR Secure EL1 */
	next_sctlr |= SCTLR_EL1_RES1;

	write_sctlr_el1(next_sctlr);
}
