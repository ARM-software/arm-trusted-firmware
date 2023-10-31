/*
 * Copyright (c) 2021-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>

#include <arch.h>
#include <arch_features.h>
#include <arch_helpers.h>
#include <common/debug.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <lib/extensions/sme.h>
#include <lib/extensions/sve.h>

void sme_enable(cpu_context_t *context)
{
	u_register_t reg;
	el3_state_t *state;

	/* Get the context state. */
	state = get_el3state_ctx(context);

	/* Set the ENTP2 bit in SCR_EL3 to enable access to TPIDR2_EL0. */
	reg = read_ctx_reg(state, CTX_SCR_EL3);
	reg |= SCR_ENTP2_BIT;
	write_ctx_reg(state, CTX_SCR_EL3, reg);
}

void sme_enable_per_world(per_world_context_t *per_world_ctx)
{
	u_register_t reg;

	/* Enable SME in CPTR_EL3. */
	reg = per_world_ctx->ctx_cptr_el3;
	reg |= ESM_BIT;
	per_world_ctx->ctx_cptr_el3 = reg;
}

void sme_init_el3(void)
{
	u_register_t cptr_el3 = read_cptr_el3();
	u_register_t smcr_el3;

	/* Set CPTR_EL3.ESM bit so we can access SMCR_EL3 without trapping. */
	write_cptr_el3(cptr_el3 | ESM_BIT);
	isb();

	/*
	 * Set the max LEN value and FA64 bit. This register is set up per_world
	 * to be the least restrictive, then lower ELs can restrict as needed
	 * using SMCR_EL2 and SMCR_EL1.
	 */
	smcr_el3 = SMCR_ELX_LEN_MAX;
	if (read_feat_sme_fa64_id_field() != 0U) {
		VERBOSE("[SME] FA64 enabled\n");
		smcr_el3 |= SMCR_ELX_FA64_BIT;
	}

	/*
	 * Enable access to ZT0 register.
	 * Make sure FEAT_SME2 is supported by the hardware before continuing.
	 * If supported, Set the EZT0 bit in SMCR_EL3 to allow instructions to
	 * access ZT0 register without trapping.
	 */
	if (is_feat_sme2_supported()) {
		VERBOSE("SME2 enabled\n");
		smcr_el3 |= SMCR_ELX_EZT0_BIT;
	}
	write_smcr_el3(smcr_el3);

	/* Reset CPTR_EL3 value. */
	write_cptr_el3(cptr_el3);
	isb();
}

void sme_init_el2_unused(void)
{
	/*
	 * CPTR_EL2.TCPAC: Set to zero so that Non-secure EL1 accesses to the
	 *  CPACR_EL1 or CPACR from both Execution states do not trap to EL2.
	 */
	write_cptr_el2(read_cptr_el2() & ~CPTR_EL2_TCPAC_BIT);
}

void sme_disable(cpu_context_t *context)
{
	u_register_t reg;
	el3_state_t *state;

	/* Get the context state. */
	state = get_el3state_ctx(context);

	/* Disable access to TPIDR2_EL0. */
	reg = read_ctx_reg(state, CTX_SCR_EL3);
	reg &= ~SCR_ENTP2_BIT;
	write_ctx_reg(state, CTX_SCR_EL3, reg);
}

void sme_disable_per_world(per_world_context_t *per_world_ctx)
{
	u_register_t reg;

	/* Disable SME, SVE, and FPU since they all share registers. */
	reg = per_world_ctx->ctx_cptr_el3;
	reg &= ~ESM_BIT;	/* Trap SME */
	reg &= ~CPTR_EZ_BIT;	/* Trap SVE */
	reg |= TFP_BIT;		/* Trap FPU/SIMD */
	per_world_ctx->ctx_cptr_el3 = reg;
}
