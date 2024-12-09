/*
 * Copyright (c) 2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <cdefs.h>

#include <arch.h>
#include <arch_features.h>
#include <arch_helpers.h>

#include <bl31/interrupt_mgmt.h>
#include <common/debug.h>
#include <drivers/arm/gicv5.h>

void __init gicv5_driver_init(void)
{
}

/*
 * There exists a theoretical configuration where FEAT_RME is enabled
 * without using TrustZone (i.e., no Secure world present). Currently,
 * there is no reliable mechanism to detect this scenario at runtime.
 *
 * TODO: Add support for this configuration in the future if required.
 */
bool gicv5_has_interrupt_type(unsigned int type)
{
	switch (type) {
	case INTR_TYPE_EL3:
	case INTR_TYPE_S_EL1:
	case INTR_TYPE_NS:
		return true;
	case INTR_TYPE_RL:
		return is_feat_rme_supported();
	default:
		return false;
	}
}

uint8_t gicv5_get_pending_interrupt_type(void)
{
	/* there is no pending interrupt expected */
	return INTR_TYPE_INVAL;
}

/* TODO: these will probably end up contexted. Make Linux work for now */
void gicv5_enable_ppis(void)
{
	uint64_t domainr = 0U;

	/* the only ones described in the device tree at the moment */
	write_icc_ppi_domainr(domainr, PPI_PMUIRQ,	INTDMN_NS);
	write_icc_ppi_domainr(domainr, PPI_GICMNT,	INTDMN_NS);
	write_icc_ppi_domainr(domainr, PPI_CNTHP,	INTDMN_NS);
	write_icc_ppi_domainr(domainr, PPI_CNTV,	INTDMN_NS);
	write_icc_ppi_domainr(domainr, PPI_CNTPS,	INTDMN_NS);
	write_icc_ppi_domainr(domainr, PPI_CNTP,	INTDMN_NS);

	write_icc_ppi_domainr0_el3(domainr);
}
