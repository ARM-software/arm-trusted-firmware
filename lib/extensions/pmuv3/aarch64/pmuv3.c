/*
 * Copyright (c) 2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_features.h>
#include <arch_helpers.h>
#include <lib/extensions/pmuv3.h>

static u_register_t init_mdcr_el2_hpmn(u_register_t mdcr_el2)
{
	/*
	 * Initialize MDCR_EL2.HPMN to its hardware reset value so we don't
	 * throw anyone off who expects this to be sensible.
	 */
	mdcr_el2 &= ~MDCR_EL2_HPMN_MASK;
	mdcr_el2 |= ((read_pmcr_el0() >> PMCR_EL0_N_SHIFT) & PMCR_EL0_N_MASK);

	return mdcr_el2;
}

void pmuv3_enable(cpu_context_t *ctx)
{
#if CTX_INCLUDE_EL2_REGS
	u_register_t mdcr_el2;

	mdcr_el2 = read_ctx_reg(get_el2_sysregs_ctx(ctx), CTX_MDCR_EL2);
	mdcr_el2 = init_mdcr_el2_hpmn(mdcr_el2);
	write_ctx_reg(get_el2_sysregs_ctx(ctx), CTX_MDCR_EL2, mdcr_el2);
#endif /* CTX_INCLUDE_EL2_REGS */
}

static u_register_t mtpmu_disable_el3(u_register_t mdcr_el3)
{
	if (!is_feat_mtpmu_supported()) {
		return mdcr_el3;
	}

	/*
	 * MDCR_EL3.MTPME = 0
	 * FEAT_MTPMU is disabled. The Effective value of PMEVTYPER<n>_EL0.MT is
	 * zero.
	 */
	mdcr_el3 &= ~MDCR_MTPME_BIT;

	return mdcr_el3;
}

void pmuv3_init_el3(void)
{
	u_register_t mdcr_el3 = read_mdcr_el3();

	/* ---------------------------------------------------------------------
	 * Initialise MDCR_EL3, setting all fields rather than relying on hw.
	 * Some fields are architecturally UNKNOWN on reset.
	 *
	 * MDCR_EL3.MPMX: Set to zero to not affect event counters (when
	 * SPME = 0).
	 *
	 * MDCR_EL3.MCCD: Set to one so that cycle counting by PMCCNTR_EL0 is
	 *  prohibited in EL3. This bit is RES0 in versions of the
	 *  architecture with FEAT_PMUv3p7 not implemented.
	 *
	 * MDCR_EL3.SCCD: Set to one so that cycle counting by PMCCNTR_EL0 is
	 *  prohibited in Secure state. This bit is RES0 in versions of the
	 *  architecture with FEAT_PMUv3p5 not implemented.
	 *
	 * MDCR_EL3.SPME: Set to zero so that event counting is prohibited in
	 *  Secure state (and explicitly EL3 with later revisions). If ARMv8.2
	 *  Debug is not implemented this bit does not have any effect on the
	 *  counters unless there is support for the implementation defined
	 *  authentication interface ExternalSecureNoninvasiveDebugEnabled().
	 *
	 * The SPME/MPMX combination is a little tricky. Below is a small
	 * summary if another combination is ever needed:
	 * SPME | MPMX | secure world |   EL3
	 * -------------------------------------
	 *   0  |  0   |    disabled  | disabled
	 *   1  |  0   |    enabled   | enabled
	 *   0  |  1   |    enabled   | disabled
	 *   1  |  1   |    enabled   | disabled only for counters 0 to
	 *                              MDCR_EL2.HPMN - 1. Enabled for the rest
	 *
	 * MDCR_EL3.TPM: Set to zero so that EL0, EL1, and EL2 System register
	 *  accesses to all Performance Monitors registers do not trap to EL3.
	 */
	mdcr_el3 = (mdcr_el3 | MDCR_SCCD_BIT | MDCR_MCCD_BIT) &
		  ~(MDCR_MPMX_BIT | MDCR_SPME_BIT | MDCR_TPM_BIT);
	mdcr_el3 = mtpmu_disable_el3(mdcr_el3);
	write_mdcr_el3(mdcr_el3);

	/* ---------------------------------------------------------------------
	 * Initialise PMCR_EL0 setting all fields rather than relying
	 * on hw. Some fields are architecturally UNKNOWN on reset.
	 *
	 * PMCR_EL0.DP: Set to one so that the cycle counter,
	 *  PMCCNTR_EL0 does not count when event counting is prohibited.
	 *  Necessary on PMUv3 <= p7 where MDCR_EL3.{SCCD,MCCD} are not
	 *  available
	 *
	 * PMCR_EL0.X: Set to zero to disable export of events.
	 *
	 * PMCR_EL0.C: Set to one to reset PMCCNTR_EL0 to zero.
	 *
	 * PMCR_EL0.P: Set to one to reset each event counter PMEVCNTR<n>_EL0 to
	 *  zero.
	 *
	 * PMCR_EL0.E: Set to zero to disable cycle and event counters.
	 * ---------------------------------------------------------------------
	 */
	write_pmcr_el0((read_pmcr_el0() | PMCR_EL0_DP_BIT | PMCR_EL0_C_BIT |
			PMCR_EL0_P_BIT) & ~(PMCR_EL0_X_BIT | PMCR_EL0_E_BIT));
}

static u_register_t mtpmu_disable_el2(u_register_t mdcr_el2)
{
	if (!is_feat_mtpmu_supported()) {
		return mdcr_el2;
	}

	/*
	 * MDCR_EL2.MTPME = 0
	 * FEAT_MTPMU is disabled. The Effective value of PMEVTYPER<n>_EL0.MT is
	 * zero.
	 */
	mdcr_el2 &= ~MDCR_EL2_MTPME;

	return mdcr_el2;
}

void pmuv3_init_el2_unused(void)
{
	u_register_t mdcr_el2 = read_mdcr_el2();

	/*
	 * Initialise MDCR_EL2, setting all fields rather than
	 * relying on hw. Some fields are architecturally
	 * UNKNOWN on reset.
	 *
	 * MDCR_EL2.HLP: Set to one so that event counter overflow, that is
	 *  recorded in PMOVSCLR_EL0[0-30], occurs on the increment that changes
	 *  PMEVCNTR<n>_EL0[63] from 1 to 0, when ARMv8.5-PMU is implemented.
	 *  This bit is RES0 in versions of the architecture earlier than
	 *  ARMv8.5, setting it to 1 doesn't have any effect on them.
	 *
	 * MDCR_EL2.HCCD: Set to one to prohibit cycle counting at EL2. This bit
	 *  is RES0 in versions of the architecture with FEAT_PMUv3p5 not
	 *  implemented.
	 *
	 * MDCR_EL2.HPMD: Set to one so that event counting is
	 *  prohibited at EL2 for counter n < MDCR_EL2.HPMN. This bit  is RES0
	 *  in versions of the architecture with FEAT_PMUv3p1 not implemented.
	 *
	 * MDCR_EL2.HPME: Set to zero to disable event counters for counters
	 *  n >= MDCR_EL2.HPMN.
	 *
	 * MDCR_EL2.TPM: Set to zero so that Non-secure EL0 and
	 *  EL1 accesses to all Performance Monitors registers
	 *  are not trapped to EL2.
	 *
	 * MDCR_EL2.TPMCR: Set to zero so that Non-secure EL0
	 *  and EL1 accesses to the PMCR_EL0 or PMCR are not
	 *  trapped to EL2.
	 */
	mdcr_el2 = (mdcr_el2 | MDCR_EL2_HLP_BIT | MDCR_EL2_HPMD_BIT |
		    MDCR_EL2_HCCD_BIT) &
		  ~(MDCR_EL2_HPME_BIT | MDCR_EL2_TPM_BIT | MDCR_EL2_TPMCR_BIT);
	mdcr_el2 = init_mdcr_el2_hpmn(mdcr_el2);
	mdcr_el2 = mtpmu_disable_el2(mdcr_el2);
	write_mdcr_el2(mdcr_el2);
}
