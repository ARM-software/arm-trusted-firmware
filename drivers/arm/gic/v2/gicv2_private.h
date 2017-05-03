/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __GICV2_PRIVATE_H__
#define __GICV2_PRIVATE_H__

#include <gicv2.h>
#include <mmio.h>
#include <stdint.h>

/*******************************************************************************
 * Private function prototypes
 ******************************************************************************/
void gicv2_spis_configure_defaults(uintptr_t gicd_base);
void gicv2_secure_spis_configure(uintptr_t gicd_base,
				     unsigned int num_ints,
				     const unsigned int *sec_intr_list);
void gicv2_secure_ppi_sgi_setup(uintptr_t gicd_base,
					unsigned int num_ints,
					const unsigned int *sec_intr_list);
unsigned int gicv2_get_cpuif_id(uintptr_t base);

/*******************************************************************************
 * GIC Distributor interface accessors for reading entire registers
 ******************************************************************************/
static inline unsigned int gicd_read_pidr2(uintptr_t base)
{
	return mmio_read_32(base + GICD_PIDR2_GICV2);
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

#endif /* __GICV2_PRIVATE_H__ */
