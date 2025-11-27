/*
 * Copyright (c) 2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef GICV5_H
#define GICV5_H

#ifndef __ASSEMBLER__
#include <stdbool.h>
#include <stdint.h>

#include <lib/mmio.h>
#endif

#include <lib/utils_def.h>

/* Interrupt Domain definitions */
#define INTDMN_S				0
#define INTDMN_NS				1
#define INTDMN_EL3				2
#define INTDMN_RL				3

/* Trigger modes */
#define TM_EDGE					0
#define TM_LEVEL				1

/* Architected PPI numbers */
#define PPI_TRBIRQ				31
#define PPI_CNTP				30
#define PPI_CNTPS				29
#define PPI_CNTHV				28
#define PPI_CNTV				27
#define PPI_CNTHP				26
#define PPI_GICMNT				25
#define PPI_CTIIRQ				24
#define PPI_PMUIRQ				23
#define PPI_COMMIRQ				22
#define PPI_PMBIRQ				21
#define PPI_CNTHPS				20
#define PPI_CNTHVS				19
#define PPI_DB_NS				2
#define PPI_DB_RL				1
#define PPI_DB_S				0

/* Register fields common to all IRI components.
 * They have the same name and offset in every config frame */
#define IRI_AIDR_COMPONENT_SHIFT		8
#define IRI_AIDR_COMPONENT_WIDTH		4
#define IRI_AIDR_COMPONENT_IRS			0
#define IRI_AIDR_COMPONENT_ITS			1
#define IRI_AIDR_COMPONENT_IWB			2
#define IRI_AIDR_ARCH_MAJOR_SHIFT		4
#define IRI_AIDR_ARCH_MAJOR_WIDTH		4
#define IRI_AIDR_ARCH_MAJOR_V5			0
#define IRI_AIDR_ARCH_MINOR_SHIFT		0
#define IRI_AIDR_ARCH_MINOR_WIDTH		4
#define IRI_AIDR_ARCH_MINOR_P0			0

/* IRS register fields */
#define IRS_IDR6_SPI_IRS_RANGE_SHIFT		0
#define IRS_IDR6_SPI_IRS_RANGE_WIDTH		24
#define IRS_IDR7_SPI_BASE_SHIFT			0
#define IRS_IDR7_SPI_BASE_WIDTH			24

#define IRS_SPI_STATUSR_IDLE_BIT		BIT(0)
#define IRS_SPI_STATUSR_V_BIT			BIT(1)

/* IWB register fields */
#define IWB_IDR0_DOMAINS_SHIFT			11
#define IWB_IDR0_DOMAINS_WIDTH			4
#define IWB_IDR0_IWRANGE_SHIFT			0
#define IWB_IDR0_IWRANGE_WIDTH			10

#define IWB_CR0_IWBEN_BIT			BIT(0)
#define IWB_CR0_IDLE_BIT			BIT(1)

#define IWB_WENABLE_STATUSR_IDLE_BIT		BIT(0)
#define IWB_WDOMAIN_STATUSR_IDLE_BIT		BIT(0)

#define IWB_WDOMAINR_DOMAINX_MASK		0x3

#ifndef __ASSEMBLER__

#define _PPI_FIELD_SHIFT(_REG, _ppi_id)						\
	((_ppi_id % (ICC_PPI_##_REG##_COUNT)) * (64 / ICC_PPI_##_REG##_COUNT))

#define write_icc_ppi_domainr(_var, _ppi_id, _value)				\
	do {									\
		_var |=  (uint64_t)_value << _PPI_FIELD_SHIFT(DOMAINR, _ppi_id);\
	} while (false)


#define DEFINE_GICV5_MMIO_WRITE_FUNC(_name, _offset)				\
static inline void write_##_name(uintptr_t base, uint32_t val)			\
{										\
	mmio_write_32(base + _offset, val);					\
}

#define DEFINE_GICV5_MMIO_READ_FUNC(_name, _offset)				\
static inline uint32_t read_##_name(uintptr_t base)				\
{										\
	return mmio_read_32(base + _offset);					\
}

#define DEFINE_GICV5_MMIO_WRITE_INDEXED_FUNC(_name, _offset)			\
static inline void write_##_name(uintptr_t base, uint16_t index, uint32_t val)	\
{										\
	mmio_write_32(base + _offset + (index * sizeof(uint32_t)), val);	\
}

#define DEFINE_GICV5_MMIO_READ_INDEXED_FUNC(_name, _offset)			\
static inline uint32_t read_##_name(uintptr_t base, uint16_t index)		\
{										\
	return mmio_read_32(base + _offset + (index * sizeof(uint32_t)));	\
}

#define DEFINE_GICV5_MMIO_RW_FUNCS(_name, _offset)				\
	DEFINE_GICV5_MMIO_READ_FUNC(_name, _offset)				\
	DEFINE_GICV5_MMIO_WRITE_FUNC(_name, _offset)

#define DEFINE_GICV5_MMIO_RW_INDEXED_FUNCS(_name, _offset)			\
	DEFINE_GICV5_MMIO_READ_INDEXED_FUNC(_name, _offset)			\
	DEFINE_GICV5_MMIO_WRITE_INDEXED_FUNC(_name, _offset)

DEFINE_GICV5_MMIO_READ_FUNC(iri_aidr,			0x44)

DEFINE_GICV5_MMIO_READ_FUNC(iwb_idr0,			0x00)
DEFINE_GICV5_MMIO_RW_FUNCS( iwb_cr0,			0x80)
DEFINE_GICV5_MMIO_READ_FUNC(iwb_wenable_statusr,	0xc0)
DEFINE_GICV5_MMIO_READ_FUNC(iwb_wdomain_statusr,	0xc4)
DEFINE_GICV5_MMIO_RW_INDEXED_FUNCS(iwb_wenabler,	0x2000)
DEFINE_GICV5_MMIO_RW_INDEXED_FUNCS(iwb_wtmr,		0x4000)
DEFINE_GICV5_MMIO_RW_INDEXED_FUNCS(iwb_wdomainr,	0x8000)

DEFINE_GICV5_MMIO_READ_FUNC(irs_idr6,			0x0018)
DEFINE_GICV5_MMIO_READ_FUNC(irs_idr7,			0x001c)
DEFINE_GICV5_MMIO_RW_FUNCS( irs_spi_selr,		0x0108)
DEFINE_GICV5_MMIO_RW_FUNCS( irs_spi_domainr,		0x010c)
DEFINE_GICV5_MMIO_RW_FUNCS( irs_spi_cfgr,		0x0114)
DEFINE_GICV5_MMIO_READ_FUNC(irs_spi_statusr,		0x0118)

#define WAIT_FOR_IDLE(base, reg, reg_up)					\
	do {									\
		while ((read_##reg(base) & reg_up##_IDLE_BIT) == 0U) {}		\
	} while (0)

/* wait for IDLE but also check the V bit was set */
#define WAIT_FOR_VIDLE(base, reg, reg_up)					\
	do {									\
		uint32_t val;							\
		while (((val = read_##reg(base)) & reg_up##_IDLE_BIT) == 0U) {}	\
		assert((val & reg##_V_BIT) != 0U);				\
	} while (0)

#define WAIT_FOR_VIDLE_IRS_SPI_STATUSR(base)					\
	WAIT_FOR_IDLE(base, irs_spi_statusr, IRS_SPI_STATUSR)

#define WAIT_FOR_IDLE_IWB_WENABLE_STATUSR(base)					\
	WAIT_FOR_IDLE(base, iwb_wenable_statusr, IWB_WENABLE_STATUSR)
#define WAIT_FOR_IDLE_IWB_WDOMAIN_STATUSR(base)					\
	WAIT_FOR_IDLE(base, iwb_wdomain_statusr, IWB_WDOMAIN_STATUSR)
#define WAIT_FOR_IDLE_IWB_CR0(base)						\
	WAIT_FOR_IDLE(base, iwb_cr0, IWB_CR0)

#define WIRE_PROP_DESC(_id, _domain, _tm) \
	{ \
		.id = (_id), \
		.domain = (_domain), \
		.tm = (_tm), \
	}

struct gicv5_wire_props {
	/* continuous wire ID as seen by the attached component */
	uint32_t id;
	/* use the INTDMN_XYZ macros */
	uint8_t domain:2;
	/* use the TM_XYZ (eg. TM_EDGE) macros */
	uint8_t tm:1;
};

/* to describe every IRS in the system */
struct gicv5_irs {
	/* mapped device nGnRnE by the platform*/
	uintptr_t el3_config_frame;
	struct gicv5_wire_props *spis;
	uint32_t num_spis;
};

/*
 * to describe every IWB in the system where EL3 is the MPPAS. IWBs that have
 * another world as an MPPAS need not be included
 */
struct gicv5_iwb {
	/* mapped device nGnRnE by the platform*/
	uintptr_t config_frame;
	struct gicv5_wire_props *wires;
	uint32_t num_wires;
};

struct gicv5_driver_data {
	struct gicv5_irs *irss;
	struct gicv5_iwb *iwbs;
	uint32_t num_irss;
	uint32_t num_iwbs;
};

extern const struct gicv5_driver_data plat_gicv5_driver_data;

void gicv5_driver_init();
uint8_t gicv5_get_pending_interrupt_type(void);
bool gicv5_has_interrupt_type(unsigned int type);
void gicv5_enable_ppis();
#endif /* __ASSEMBLER__ */
#endif /* GICV5_H */
