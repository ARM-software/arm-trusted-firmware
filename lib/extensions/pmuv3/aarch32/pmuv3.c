/*
 * Copyright (c) 2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_features.h>
#include <arch_helpers.h>
#include <lib/extensions/pmuv3.h>

static u_register_t mtpmu_disable_el3(u_register_t sdcr)
{
	if (!is_feat_mtpmu_supported()) {
		return sdcr;
	}

	/*
	 * SDCR.MTPME = 0
	 * FEAT_MTPMU is disabled. The Effective value of PMEVTYPER<n>.MT is
	 * zero.
	 */
	sdcr &= ~SDCR_MTPME_BIT;

	return sdcr;
}

/*
 * Applies to all PMU versions. Name is PMUv3 for compatibility with aarch64 and
 * to not clash with platforms which reuse the PMU name
 */
void pmuv3_init_el3(void)
{
	u_register_t sdcr = read_sdcr();

	/* ---------------------------------------------------------------------
	 * Initialise SDCR, setting all the fields rather than relying on hw.
	 *
	 * SDCR.SCCD: Set to one so that cycle counting by PMCCNTR is prohibited
	 *  in Secure state. This bit is RES0 in versions of the architecture
	 *  earlier than ARMv8.5
	 *
	 * SDCR.SPME: Set to zero so that event counting is prohibited in Secure
	 *  state (and explicitly EL3 with later revisions). If ARMv8.2 Debug is
	 *  not implemented this bit does not have any effect on the counters
	 *  unless there is support for the implementation defined
	 *  authentication interface ExternalSecureNoninvasiveDebugEnabled().
	 * ---------------------------------------------------------------------
	 */
	sdcr = (sdcr | SDCR_SCCD_BIT) & ~SDCR_SPME_BIT;
	sdcr = mtpmu_disable_el3(sdcr);
	write_sdcr(sdcr);

	/* ---------------------------------------------------------------------
	 * Initialise PMCR, setting all fields rather than relying
	 * on hw. Some fields are architecturally UNKNOWN on reset.
	 *
	 * PMCR.DP: Set to one to prohibit cycle counting whilst in Secure mode.
	 *
	 * PMCR.X: Set to zero to disable export of events.
	 *
	 * PMCR.C: Set to one to reset PMCCNTR.
	 *
	 * PMCR.P: Set to one to reset each event counter PMEVCNTR<n> to zero.
	 *
	 * PMCR.E: Set to zero to disable cycle and event counters.
	 * ---------------------------------------------------------------------
	 */

	write_pmcr(read_pmcr() | PMCR_DP_BIT | PMCR_C_BIT | PMCR_P_BIT |
		 ~(PMCR_X_BIT | PMCR_E_BIT));
}
