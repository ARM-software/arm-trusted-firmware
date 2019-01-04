/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <drivers/arm/gic_common.h>
#include <lib/mmio.h>

#include "gic_common_private.h"

/*******************************************************************************
 * GIC Distributor interface accessors for reading entire registers
 ******************************************************************************/
/*
 * Accessor to read the GIC Distributor IGROUPR corresponding to the interrupt
 * `id`, 32 interrupt ids at a time.
 */
unsigned int gicd_read_igroupr(uintptr_t base, unsigned int id)
{
	unsigned int n = id >> IGROUPR_SHIFT;

	return mmio_read_32(base + GICD_IGROUPR + (n << 2));
}

/*
 * Accessor to read the GIC Distributor ISENABLER corresponding to the
 * interrupt `id`, 32 interrupt ids at a time.
 */
unsigned int gicd_read_isenabler(uintptr_t base, unsigned int id)
{
	unsigned int n = id >> ISENABLER_SHIFT;

	return mmio_read_32(base + GICD_ISENABLER + (n << 2));
}

/*
 * Accessor to read the GIC Distributor ICENABLER corresponding to the
 * interrupt `id`, 32 interrupt IDs at a time.
 */
unsigned int gicd_read_icenabler(uintptr_t base, unsigned int id)
{
	unsigned int n = id >> ICENABLER_SHIFT;

	return mmio_read_32(base + GICD_ICENABLER + (n << 2));
}

/*
 * Accessor to read the GIC Distributor ISPENDR corresponding to the
 * interrupt `id`, 32 interrupt IDs at a time.
 */
unsigned int gicd_read_ispendr(uintptr_t base, unsigned int id)
{
	unsigned int n = id >> ISPENDR_SHIFT;

	return mmio_read_32(base + GICD_ISPENDR + (n << 2));
}

/*
 * Accessor to read the GIC Distributor ICPENDR corresponding to the
 * interrupt `id`, 32 interrupt IDs at a time.
 */
unsigned int gicd_read_icpendr(uintptr_t base, unsigned int id)
{
	unsigned int n = id >> ICPENDR_SHIFT;

	return mmio_read_32(base + GICD_ICPENDR + (n << 2));
}

/*
 * Accessor to read the GIC Distributor ISACTIVER corresponding to the
 * interrupt `id`, 32 interrupt IDs at a time.
 */
unsigned int gicd_read_isactiver(uintptr_t base, unsigned int id)
{
	unsigned int n = id >> ISACTIVER_SHIFT;

	return mmio_read_32(base + GICD_ISACTIVER + (n << 2));
}

/*
 * Accessor to read the GIC Distributor ICACTIVER corresponding to the
 * interrupt `id`, 32 interrupt IDs at a time.
 */
unsigned int gicd_read_icactiver(uintptr_t base, unsigned int id)
{
	unsigned int n = id >> ICACTIVER_SHIFT;

	return mmio_read_32(base + GICD_ICACTIVER + (n << 2));
}

/*
 * Accessor to read the GIC Distributor IPRIORITYR corresponding to the
 * interrupt `id`, 4 interrupt IDs at a time.
 */
unsigned int gicd_read_ipriorityr(uintptr_t base, unsigned int id)
{
	unsigned int n = id >> IPRIORITYR_SHIFT;

	return mmio_read_32(base + GICD_IPRIORITYR + (n << 2));
}

/*
 * Accessor to read the GIC Distributor ICGFR corresponding to the
 * interrupt `id`, 16 interrupt IDs at a time.
 */
unsigned int gicd_read_icfgr(uintptr_t base, unsigned int id)
{
	unsigned int n = id >> ICFGR_SHIFT;

	return mmio_read_32(base + GICD_ICFGR + (n << 2));
}

/*
 * Accessor to read the GIC Distributor NSACR corresponding to the
 * interrupt `id`, 16 interrupt IDs at a time.
 */
unsigned int gicd_read_nsacr(uintptr_t base, unsigned int id)
{
	unsigned int n = id >> NSACR_SHIFT;

	return mmio_read_32(base + GICD_NSACR + (n << 2));
}

/*******************************************************************************
 * GIC Distributor interface accessors for writing entire registers
 ******************************************************************************/
/*
 * Accessor to write the GIC Distributor IGROUPR corresponding to the
 * interrupt `id`, 32 interrupt IDs at a time.
 */
void gicd_write_igroupr(uintptr_t base, unsigned int id, unsigned int val)
{
	unsigned int n = id >> IGROUPR_SHIFT;

	mmio_write_32(base + GICD_IGROUPR + (n << 2), val);
}

/*
 * Accessor to write the GIC Distributor ISENABLER corresponding to the
 * interrupt `id`, 32 interrupt IDs at a time.
 */
void gicd_write_isenabler(uintptr_t base, unsigned int id, unsigned int val)
{
	unsigned int n = id >> ISENABLER_SHIFT;

	mmio_write_32(base + GICD_ISENABLER + (n << 2), val);
}

/*
 * Accessor to write the GIC Distributor ICENABLER corresponding to the
 * interrupt `id`, 32 interrupt IDs at a time.
 */
void gicd_write_icenabler(uintptr_t base, unsigned int id, unsigned int val)
{
	unsigned int n = id >> ICENABLER_SHIFT;

	mmio_write_32(base + GICD_ICENABLER + (n << 2), val);
}

/*
 * Accessor to write the GIC Distributor ISPENDR corresponding to the
 * interrupt `id`, 32 interrupt IDs at a time.
 */
void gicd_write_ispendr(uintptr_t base, unsigned int id, unsigned int val)
{
	unsigned int n = id >> ISPENDR_SHIFT;

	mmio_write_32(base + GICD_ISPENDR + (n << 2), val);
}

/*
 * Accessor to write the GIC Distributor ICPENDR corresponding to the
 * interrupt `id`, 32 interrupt IDs at a time.
 */
void gicd_write_icpendr(uintptr_t base, unsigned int id, unsigned int val)
{
	unsigned int n = id >> ICPENDR_SHIFT;

	mmio_write_32(base + GICD_ICPENDR + (n << 2), val);
}

/*
 * Accessor to write the GIC Distributor ISACTIVER corresponding to the
 * interrupt `id`, 32 interrupt IDs at a time.
 */
void gicd_write_isactiver(uintptr_t base, unsigned int id, unsigned int val)
{
	unsigned int n = id >> ISACTIVER_SHIFT;

	mmio_write_32(base + GICD_ISACTIVER + (n << 2), val);
}

/*
 * Accessor to write the GIC Distributor ICACTIVER corresponding to the
 * interrupt `id`, 32 interrupt IDs at a time.
 */
void gicd_write_icactiver(uintptr_t base, unsigned int id, unsigned int val)
{
	unsigned int n = id >> ICACTIVER_SHIFT;

	mmio_write_32(base + GICD_ICACTIVER + (n << 2), val);
}

/*
 * Accessor to write the GIC Distributor IPRIORITYR corresponding to the
 * interrupt `id`, 4 interrupt IDs at a time.
 */
void gicd_write_ipriorityr(uintptr_t base, unsigned int id, unsigned int val)
{
	unsigned int n = id >> IPRIORITYR_SHIFT;

	mmio_write_32(base + GICD_IPRIORITYR + (n << 2), val);
}

/*
 * Accessor to write the GIC Distributor ICFGR corresponding to the
 * interrupt `id`, 16 interrupt IDs at a time.
 */
void gicd_write_icfgr(uintptr_t base, unsigned int id, unsigned int val)
{
	unsigned int n = id >> ICFGR_SHIFT;

	mmio_write_32(base + GICD_ICFGR + (n << 2), val);
}

/*
 * Accessor to write the GIC Distributor NSACR corresponding to the
 * interrupt `id`, 16 interrupt IDs at a time.
 */
void gicd_write_nsacr(uintptr_t base, unsigned int id, unsigned int val)
{
	unsigned int n = id >> NSACR_SHIFT;

	mmio_write_32(base + GICD_NSACR + (n << 2), val);
}

/*******************************************************************************
 * GIC Distributor functions for accessing the GIC registers
 * corresponding to a single interrupt ID. These functions use bitwise
 * operations or appropriate register accesses to modify or return
 * the bit-field corresponding the single interrupt ID.
 ******************************************************************************/
unsigned int gicd_get_igroupr(uintptr_t base, unsigned int id)
{
	unsigned int bit_num = id & ((1U << IGROUPR_SHIFT) - 1U);
	unsigned int reg_val = gicd_read_igroupr(base, id);

	return (reg_val >> bit_num) & 0x1U;
}

void gicd_set_igroupr(uintptr_t base, unsigned int id)
{
	unsigned int bit_num = id & ((1U << IGROUPR_SHIFT) - 1U);
	unsigned int reg_val = gicd_read_igroupr(base, id);

	gicd_write_igroupr(base, id, reg_val | (1U << bit_num));
}

void gicd_clr_igroupr(uintptr_t base, unsigned int id)
{
	unsigned int bit_num = id & ((1U << IGROUPR_SHIFT) - 1U);
	unsigned int reg_val = gicd_read_igroupr(base, id);

	gicd_write_igroupr(base, id, reg_val & ~(1U << bit_num));
}

void gicd_set_isenabler(uintptr_t base, unsigned int id)
{
	unsigned int bit_num = id & ((1U << ISENABLER_SHIFT) - 1U);

	gicd_write_isenabler(base, id, (1U << bit_num));
}

void gicd_set_icenabler(uintptr_t base, unsigned int id)
{
	unsigned int bit_num = id & ((1U << ICENABLER_SHIFT) - 1U);

	gicd_write_icenabler(base, id, (1U << bit_num));
}

void gicd_set_ispendr(uintptr_t base, unsigned int id)
{
	unsigned int bit_num = id & ((1U << ISPENDR_SHIFT) - 1U);

	gicd_write_ispendr(base, id, (1U << bit_num));
}

void gicd_set_icpendr(uintptr_t base, unsigned int id)
{
	unsigned int bit_num = id & ((1U << ICPENDR_SHIFT) - 1U);

	gicd_write_icpendr(base, id, (1U << bit_num));
}

unsigned int gicd_get_isactiver(uintptr_t base, unsigned int id)
{
	unsigned int bit_num = id & ((1U << ISACTIVER_SHIFT) - 1U);
	unsigned int reg_val = gicd_read_isactiver(base, id);

	return (reg_val >> bit_num) & 0x1U;
}

void gicd_set_isactiver(uintptr_t base, unsigned int id)
{
	unsigned int bit_num = id & ((1U << ISACTIVER_SHIFT) - 1U);

	gicd_write_isactiver(base, id, (1U << bit_num));
}

void gicd_set_icactiver(uintptr_t base, unsigned int id)
{
	unsigned int bit_num = id & ((1U << ICACTIVER_SHIFT) - 1U);

	gicd_write_icactiver(base, id, (1U << bit_num));
}

void gicd_set_ipriorityr(uintptr_t base, unsigned int id, unsigned int pri)
{
	uint8_t val = pri & GIC_PRI_MASK;

	mmio_write_8(base + GICD_IPRIORITYR + id, val);
}

void gicd_set_icfgr(uintptr_t base, unsigned int id, unsigned int cfg)
{
	/* Interrupt configuration is a 2-bit field */
	unsigned int bit_num = id & ((1U << ICFGR_SHIFT) - 1U);
	unsigned int bit_shift = bit_num << 1;

	uint32_t reg_val = gicd_read_icfgr(base, id);

	/* Clear the field, and insert required configuration */
	reg_val &= ~(GIC_CFG_MASK << bit_shift);
	reg_val |= ((cfg & GIC_CFG_MASK) << bit_shift);

	gicd_write_icfgr(base, id, reg_val);
}
