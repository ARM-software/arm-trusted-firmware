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
	uint64_t features;

	features = read_id_aa64dfr0_el1() >> ID_AA64DFR0_TRACEVER_SHIFT;
	return ((features & ID_AA64DFR0_TRACEVER_MASK) ==
		ID_AA64DFR0_TRACEVER_SUPPORTED);
}

void sys_reg_trace_enable(cpu_context_t *ctx)
{
	uint64_t val;

	if (sys_reg_trace_supported()) {
		/* Retrieve CPTR_EL3 value from the given context 'ctx',
		 * and update CPTR_EL3.TTA bit to 0.
		 * This function is called while switching context to NS to
		 * allow system trace register access to NS-EL2 and NS-EL1
		 * when NS-EL2 is implemented but not used.
		 */
		val = read_ctx_reg(get_el3state_ctx(ctx), CTX_CPTR_EL3);
		val &= ~TTA_BIT;
		write_ctx_reg(get_el3state_ctx(ctx), CTX_CPTR_EL3, val);
	}
}
