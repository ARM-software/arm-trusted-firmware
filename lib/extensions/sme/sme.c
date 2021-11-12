/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>

#include <arch.h>
#include <arch_helpers.h>
#include <common/debug.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <lib/extensions/sme.h>
#include <lib/extensions/sve.h>

static bool feat_sme_supported(void)
{
	uint64_t features;

	features = read_id_aa64pfr1_el1() >> ID_AA64PFR1_EL1_SME_SHIFT;
	return (features & ID_AA64PFR1_EL1_SME_MASK) != 0U;
}

static bool feat_sme_fa64_supported(void)
{
	uint64_t features;

	features = read_id_aa64smfr0_el1();
	return (features & ID_AA64SMFR0_EL1_FA64_BIT) != 0U;
}

void sme_enable(cpu_context_t *context)
{
	u_register_t reg;
	u_register_t cptr_el3;
	el3_state_t *state;

	/* Make sure SME is implemented in hardware before continuing. */
	if (!feat_sme_supported()) {
		return;
	}

	/* Get the context state. */
	state = get_el3state_ctx(context);

	/* Enable SME in CPTR_EL3. */
	reg = read_ctx_reg(state, CTX_CPTR_EL3);
	reg |= ESM_BIT;
	write_ctx_reg(state, CTX_CPTR_EL3, reg);

	/* Set the ENTP2 bit in SCR_EL3 to enable access to TPIDR2_EL0. */
	reg = read_ctx_reg(state, CTX_SCR_EL3);
	reg |= SCR_ENTP2_BIT;
	write_ctx_reg(state, CTX_SCR_EL3, reg);

	/* Set CPTR_EL3.ESM bit so we can write SMCR_EL3 without trapping. */
	cptr_el3 = read_cptr_el3();
	write_cptr_el3(cptr_el3 | ESM_BIT);

	/*
	 * Set the max LEN value and FA64 bit. This register is set up globally
	 * to be the least restrictive, then lower ELs can restrict as needed
	 * using SMCR_EL2 and SMCR_EL1.
	 */
	reg = SMCR_ELX_LEN_MASK;
	if (feat_sme_fa64_supported()) {
		VERBOSE("[SME] FA64 enabled\n");
		reg |= SMCR_ELX_FA64_BIT;
	}
	write_smcr_el3(reg);

	/* Reset CPTR_EL3 value. */
	write_cptr_el3(cptr_el3);

	/* Enable SVE/FPU in addition to SME. */
	sve_enable(context);
}

void sme_disable(cpu_context_t *context)
{
	u_register_t reg;
	el3_state_t *state;

	/* Make sure SME is implemented in hardware before continuing. */
	if (!feat_sme_supported()) {
		return;
	}

	/* Get the context state. */
	state = get_el3state_ctx(context);

	/* Disable SME, SVE, and FPU since they all share registers. */
	reg = read_ctx_reg(state, CTX_CPTR_EL3);
	reg &= ~ESM_BIT;	/* Trap SME */
	reg &= ~CPTR_EZ_BIT;	/* Trap SVE */
	reg |= TFP_BIT;		/* Trap FPU/SIMD */
	write_ctx_reg(state, CTX_CPTR_EL3, reg);

	/* Disable access to TPIDR2_EL0. */
	reg = read_ctx_reg(state, CTX_SCR_EL3);
	reg &= ~SCR_ENTP2_BIT;
	write_ctx_reg(state, CTX_SCR_EL3, reg);
}
