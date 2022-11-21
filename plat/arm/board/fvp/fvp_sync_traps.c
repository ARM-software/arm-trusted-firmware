/*
 * Copyright (c) 2022, ARM Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file just contains demonstration code, to "handle" RNG traps.
 */

#include <stdbool.h>

#include <arch.h>
#include <arch_helpers.h>
#include <bl31/sync_handle.h>
#include <context.h>

/*
 * SCR_EL3.SCR_TRNDR_BIT also affects execution in EL3, so allow to disable
 * the trap temporarily.
 */
static void enable_rng_trap(bool enable)
{
	uint64_t scr_el3 = read_scr_el3();

	if (enable) {
		scr_el3 |= SCR_TRNDR_BIT;
	} else {
		scr_el3 &= ~SCR_TRNDR_BIT;
	}

	write_scr_el3(scr_el3);
	isb();
}

/*
 * This emulation code here is not very meaningful: enabling the RNG
 * trap typically happens for a reason, so just calling the actual
 * hardware instructions might not be useful or even possible.
 */
int plat_handle_rng_trap(uint64_t esr_el3, cpu_context_t *ctx)
{
	/* extract the target register number from the exception syndrome */
	unsigned int rt = get_sysreg_iss_rt(esr_el3);

	/* ignore XZR accesses and writes to the register */
	if (rt == 31 || is_sysreg_iss_write(esr_el3)) {
		return TRAP_RET_CONTINUE;
	}

	enable_rng_trap(false);
	if ((esr_el3 & ISS_SYSREG_OPCODE_MASK) == ISS_SYSREG_OPCODE_RNDR) {
		ctx->gpregs_ctx.ctx_regs[rt] = read_rndr();
	} else {
		ctx->gpregs_ctx.ctx_regs[rt] = read_rndrrs();
	}
	enable_rng_trap(true);

	/*
	 * We successfully handled the trap, continue with the next
	 * instruction.
	 */
	return TRAP_RET_CONTINUE;
}
