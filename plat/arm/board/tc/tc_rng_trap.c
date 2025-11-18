/*
 * Copyright (c) 2025, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <bl31/sync_handle.h>
#include <context.h>
#include <plat/common/plat_trng.h>

#define XZR_REG_NUM   31


int plat_handle_rng_trap(uint64_t esr_el3, cpu_context_t *ctx)
{
	uint64_t entropy;

	/* extract the target register number from the exception syndrome */
	unsigned int rt = get_sysreg_iss_rt(esr_el3);

	/* ignore XZR accesses and writes to the register */
	assert(rt != XZR_REG_NUM && !is_sysreg_iss_write(esr_el3));

	if (!plat_get_entropy(&entropy)) {
		ERROR("Failed to get entropy\n");
		panic();
	}

	/* Emulate RNDR and RNDRRS */
	gp_regs_t *gpregs = get_gpregs_ctx(ctx);

	gpregs->ctx_regs[rt] = entropy;

	/*
	 * We successfully handled the trap, continue with the next
	 * instruction.
	 */
	return TRAP_RET_CONTINUE;
}
