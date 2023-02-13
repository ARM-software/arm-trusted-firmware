/*
 * Copyright (c) 2021-2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>

#include <arch.h>
#include <arch_helpers.h>
#include <lib/extensions/sys_reg_trace.h>

void sys_reg_trace_enable(cpu_context_t *ctx)
{
	/*
	 * CPTR_EL3.TTA: Set to zero so that System register accesses to the
	 *  trace registers do not trap to EL3.
	 */
	uint64_t val = read_ctx_reg(get_el3state_ctx(ctx), CTX_CPTR_EL3);

	val &= ~(TTA_BIT);
	write_ctx_reg(get_el3state_ctx(ctx), CTX_CPTR_EL3, val);
}

void sys_reg_trace_disable(cpu_context_t *ctx)
{
	/*
	 * CPTR_EL3.TTA: Set to one so that System register accesses to the
	 *  trace registers trap to EL3, unless it is trapped by CPACR.TRCDIS,
	 *  CPACR_EL1.TTA, or CPTR_EL2.TTA
	 */
	uint64_t val = read_ctx_reg(get_el3state_ctx(ctx), CTX_CPTR_EL3);

	val |= TTA_BIT;
	write_ctx_reg(get_el3state_ctx(ctx), CTX_CPTR_EL3, val);
}

void sys_reg_trace_init_el2_unused(void)
{
	/*
	 * CPTR_EL2.TTA: Set to zero so that Non-secure System register accesses
	 *  to the trace registers from both Execution states do not trap to
	 *  EL2. If PE trace unit System registers are not implemented then this
	 *  bit is reserved, and must be set to zero.
	 */
	write_cptr_el2(read_cptr_el2() & ~CPTR_EL2_TTA_BIT);
}
