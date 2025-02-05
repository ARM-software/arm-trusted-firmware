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

/*
 * Check for a bad platform configuration.
 * Not expected to be called on release builds.
 */
static inline bool probe_component(uintptr_t base_addr, uint8_t component)
{
	uint32_t aidr = read_iri_aidr(base_addr);

	if (EXTRACT(IRI_AIDR_COMPONENT, aidr) != component) {
		ERROR("GICv5 frame belongs to wrong component\n");
		return false;
	}

	if (EXTRACT(IRI_AIDR_ARCH_MAJOR, aidr) != IRI_AIDR_ARCH_MAJOR_V5) {
		ERROR("Bad GICv5 major version\n");
		return false;
	}

	/* there was a bump in architecture and we've not updated the driver */
	assert(EXTRACT(IRI_AIDR_ARCH_MINOR, aidr) == IRI_AIDR_ARCH_MINOR_P0);

	return true;
}

static inline bool iwb_domain_supported(uint32_t idr0, uint8_t domain)
{
	return (EXTRACT(IWB_IDR0_DOMAINS, idr0) & (1U << domain)) != 0U;
}

static void iwb_configure_domainr(uintptr_t base_addr, struct gicv5_wire_props wire)
{
	uint32_t reg_offset = (wire.id % 16U) * 2U;
	uint32_t reg_index = wire.id / 16U;
	uint32_t val = read_iwb_wdomainr(base_addr, reg_index) &
		       ~(IWB_WDOMAINR_DOMAINX_MASK << reg_offset);

	write_iwb_wdomainr(base_addr, reg_index, val | wire.domain << reg_offset);
}

static void iwb_configure_wtmr(uintptr_t base_addr, struct gicv5_wire_props wire)
{
	uint32_t reg_offset = wire.id % 32U;
	uint32_t reg_index = wire.id / 32U;
	uint32_t val = read_iwb_wtmr(base_addr, reg_index) & ~(1U << reg_offset);

	write_iwb_wtmr(base_addr, reg_index, val | wire.tm << reg_offset);
}

static void iwb_enable(const struct gicv5_iwb *config)
{
	uintptr_t base_addr = config->config_frame;
	uint32_t idr0;
	uint16_t num_regs;

	assert(probe_component(base_addr, IRI_AIDR_COMPONENT_IWB));

	idr0 = read_iwb_idr0(base_addr);
	num_regs = EXTRACT(IWB_IDR0_IWRANGE, idr0) + 1U;

	/* initialise all wires as disabled */
	for (int i = 0U; i < num_regs; i++) {
		write_iwb_wenabler(base_addr, i, 0U);
	}

	/* default all wires to the NS domain */
	for (int i = 0U; i < num_regs * 2; i++) {
		write_iwb_wdomainr(base_addr, i, 0x55555555);
	}

	for (uint32_t i = 0U; i < config->num_wires; i++) {
		assert(iwb_domain_supported(idr0, config->wires[i].domain));
		assert(config->wires[i].id <= num_regs * 32);

		iwb_configure_domainr(base_addr, config->wires[i]);
		iwb_configure_wtmr(base_addr, config->wires[i]);
	}

	write_iwb_cr0(base_addr, IWB_CR0_IWBEN_BIT);
	WAIT_FOR_IDLE_IWB_WENABLE_STATUSR(base_addr);
	WAIT_FOR_IDLE_IWB_WDOMAIN_STATUSR(base_addr);
	WAIT_FOR_IDLE_IWB_CR0(base_addr);
}

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

	assert(probe_component(base_addr, IRI_AIDR_COMPONENT_IRS));

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
	for (size_t i = 0U; i < plat_gicv5_driver_data.num_iwbs; i++) {
		iwb_enable(&plat_gicv5_driver_data.iwbs[i]);
	}

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
