/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>

#include <arch.h>
#include <arch_helpers.h>
#include <lib/extensions/sys_reg_trace.h>

static bool sys_reg_trace_supported(void)
{
	uint32_t features;

	features = read_id_dfr0() >> ID_DFR0_COPTRC_SHIFT;
	return ((features & ID_DFR0_COPTRC_MASK) ==
		ID_DFR0_COPTRC_SUPPORTED);
}

void sys_reg_trace_enable(void)
{
	uint32_t val;

	if (sys_reg_trace_supported()) {
		/*
		 * NSACR.NSTRCDIS = b0
		 * enable NS system register access to implemented trace
		 * registers.
		 */
		val = read_nsacr();
		val &= ~NSTRCDIS_BIT;
		write_nsacr(val);
	}
}
