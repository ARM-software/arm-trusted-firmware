/*
 * Copyright (c) 2015-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef GICV2_PRIVATE_H
#define GICV2_PRIVATE_H

#include <stdint.h>

#include <drivers/arm/gicv2.h>
#include <lib/mmio.h>

/*******************************************************************************
 * Private function prototypes
 ******************************************************************************/
void gicv2_spis_configure_defaults(uintptr_t gicd_base);
void gicv2_secure_spis_configure_props(uintptr_t gicd_base,
		const interrupt_prop_t *interrupt_props,
		unsigned int interrupt_props_num);
void gicv2_secure_ppi_sgi_setup_props(uintptr_t gicd_base,
		const interrupt_prop_t *interrupt_props,
		unsigned int interrupt_props_num);
unsigned int gicv2_get_cpuif_id(uintptr_t base);

/*******************************************************************************
 * GIC Distributor interface accessors for reading entire registers
 ******************************************************************************/
static inline unsigned int gicd_read_pidr2(uintptr_t base)
{
	return mmio_read_32(base + GICD_PIDR2_GICV2);
}

/*******************************************************************************
 * GIC Distributor interface accessors for writing entire registers
 ******************************************************************************/
static inline unsigned int gicd_get_itargetsr(uintptr_t base, unsigned int id)
{
	return mmio_read_8(base + GICD_ITARGETSR + id);
}

static inline void gicd_set_itargetsr(uintptr_t base, unsigned int id,
		unsigned int target)
{
	uint8_t val = target & GIC_TARGET_CPU_MASK;

	mmio_write_8(base + GICD_ITARGETSR + id, val);
}

static inline void gicd_write_sgir(uintptr_t base, unsigned int val)
{
	mmio_write_32(base + GICD_SGIR, val);
}

/*******************************************************************************
 * GIC CPU interface accessors for reading entire registers
 ******************************************************************************/

static inline unsigned int gicc_read_ctlr(uintptr_t base)
{
	return mmio_read_32(base + GICC_CTLR);
}

static inline unsigned int gicc_read_pmr(uintptr_t base)
{
	return mmio_read_32(base + GICC_PMR);
}

static inline unsigned int gicc_read_BPR(uintptr_t base)
{
	return mmio_read_32(base + GICC_BPR);
}

static inline unsigned int gicc_read_IAR(uintptr_t base)
{
	return mmio_read_32(base + GICC_IAR);
}

static inline unsigned int gicc_read_EOIR(uintptr_t base)
{
	return mmio_read_32(base + GICC_EOIR);
}

static inline unsigned int gicc_read_hppir(uintptr_t base)
{
	return mmio_read_32(base + GICC_HPPIR);
}

static inline unsigned int gicc_read_ahppir(uintptr_t base)
{
	return mmio_read_32(base + GICC_AHPPIR);
}

static inline unsigned int gicc_read_dir(uintptr_t base)
{
	return mmio_read_32(base + GICC_DIR);
}

static inline unsigned int gicc_read_iidr(uintptr_t base)
{
	return mmio_read_32(base + GICC_IIDR);
}

static inline unsigned int gicc_read_rpr(uintptr_t base)
{
	return mmio_read_32(base + GICC_RPR);
}

/*******************************************************************************
 * GIC CPU interface accessors for writing entire registers
 ******************************************************************************/

static inline void gicc_write_ctlr(uintptr_t base, unsigned int val)
{
	mmio_write_32(base + GICC_CTLR, val);
}

static inline void gicc_write_pmr(uintptr_t base, unsigned int val)
{
	mmio_write_32(base + GICC_PMR, val);
}

static inline void gicc_write_BPR(uintptr_t base, unsigned int val)
{
	mmio_write_32(base + GICC_BPR, val);
}


static inline void gicc_write_IAR(uintptr_t base, unsigned int val)
{
	mmio_write_32(base + GICC_IAR, val);
}

static inline void gicc_write_EOIR(uintptr_t base, unsigned int val)
{
	mmio_write_32(base + GICC_EOIR, val);
}

static inline void gicc_write_hppir(uintptr_t base, unsigned int val)
{
	mmio_write_32(base + GICC_HPPIR, val);
}

static inline void gicc_write_dir(uintptr_t base, unsigned int val)
{
	mmio_write_32(base + GICC_DIR, val);
}

#endif /* GICV2_PRIVATE_H */
