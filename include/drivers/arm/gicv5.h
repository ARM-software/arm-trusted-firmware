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

/* IRS register fields */
#define IRS_IDR6_SPI_IRS_RANGE_SHIFT		0
#define IRS_IDR6_SPI_IRS_RANGE_WIDTH		24
#define IRS_IDR7_SPI_BASE_SHIFT			0
#define IRS_IDR7_SPI_BASE_WIDTH			24

#define IRS_SPI_STATUSR_IDLE_BIT		BIT(0)
#define IRS_SPI_STATUSR_V_BIT			BIT(1)

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

#define DEFINE_GICV5_MMIO_RW_FUNCS(_name, _offset)				\
	DEFINE_GICV5_MMIO_READ_FUNC(_name, _offset)				\
	DEFINE_GICV5_MMIO_WRITE_FUNC(_name, _offset)

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

struct gicv5_driver_data {
	struct gicv5_irs *irss;
	uint32_t num_irss;
};

extern const struct gicv5_driver_data plat_gicv5_driver_data;

void gicv5_driver_init();
uint8_t gicv5_get_pending_interrupt_type(void);
bool gicv5_has_interrupt_type(unsigned int type);
void gicv5_enable_ppis();
#endif /* __ASSEMBLER__ */
#endif /* GICV5_H */
