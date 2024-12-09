/*
 * Copyright (c) 2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <cdefs.h>

#include <arch.h>
#include <arch_features.h>
#include <arch_helpers.h>

#include <bl31/interrupt_mgmt.h>
#include <common/debug.h>
#include <drivers/arm/gicv5.h>

static void irs_configure_wire(uintptr_t base_addr, uint32_t wire, uint8_t domain)
{
	write_irs_spi_selr(base_addr, wire);
	WAIT_FOR_VIDLE_IRS_SPI_STATUSR(base_addr);

	write_irs_spi_domainr(base_addr, domain);
	WAIT_FOR_VIDLE_IRS_SPI_STATUSR(base_addr);
}

static void irs_enable(const struct gicv5_irs *config)
{
	uint32_t spi_base, spi_range;
	uintptr_t base_addr = config->el3_config_frame;

	spi_base  = EXTRACT(IRS_IDR7_SPI_BASE, read_irs_idr7(base_addr));
	spi_range = EXTRACT(IRS_IDR6_SPI_IRS_RANGE, read_irs_idr6(base_addr));

	if (spi_range == 0U) {
		assert(config->num_spis == 0U);
	}

	/* default all wires to the NS domain */
	for (uint32_t i = spi_base; i < spi_base + spi_range; i++) {
		irs_configure_wire(base_addr, i, INTDMN_NS);
	}

	for (uint32_t i = 0U; i < config->num_spis; i++) {
		assert((config->spis[i].id >= spi_base) &&
		       (config->spis[i].id < spi_base + spi_range));

		irs_configure_wire(base_addr, config->spis[i].id, config->spis[i].domain);

		/* don't (can't) configure TM of wires for other domains */
		if (config->spis[i].domain == INTDMN_EL3) {
			write_irs_spi_cfgr(base_addr, config->spis[i].tm);
			WAIT_FOR_VIDLE_IRS_SPI_STATUSR(base_addr);
		}
	}
}

void __init gicv5_driver_init(void)
{
	for (size_t i = 0U; i < plat_gicv5_driver_data.num_irss; i++) {
		irs_enable(&plat_gicv5_driver_data.irss[i]);
	}
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
