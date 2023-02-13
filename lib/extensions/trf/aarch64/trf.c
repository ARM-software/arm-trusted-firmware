/*
 * Copyright (c) 2021-2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_features.h>
#include <arch_helpers.h>
#include <lib/extensions/trf.h>

void trf_init_el3(void)
{
	u_register_t val;

	/*
	 * MDCR_EL3.STE = b0
	 * Trace prohibited in Secure state unless overridden by the
	 * IMPLEMENTATION DEFINED authentication interface.
	 *
	 * MDCR_EL3.TTRF = b0
	 * Allow access of trace filter control registers from NS-EL2
	 * and NS-EL1 when NS-EL2 is implemented but not used
	 */
	val = read_mdcr_el3();
	val &= ~(MDCR_STE_BIT | MDCR_TTRF_BIT);
	write_mdcr_el3(val);
}

void trf_init_el2_unused(void)
{
	/*
	 * MDCR_EL2.TTRF: Set to zero so that access to Trace
	 *  Filter Control register TRFCR_EL1 at EL1 is not
	 *  trapped to EL2. This bit is RES0 in versions of
	 *  the architecture earlier than ARMv8.4.
	 *
	 */
	write_mdcr_el2(read_mdcr_el2() & ~MDCR_EL2_TTRF);
}
