/*
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_helpers.h>
#include <common/debug.h>
#include <common/interrupt_props.h>
#include <drivers/arm/gicv3.h>
#include "gicv3_private.h"

/*******************************************************************************
 * GIC Redistributor functions
 * Note: The raw register values correspond to multiple interrupt IDs and
 * the number of interrupt IDs involved depends on the register accessed.
 ******************************************************************************/

/*
 * Accessor to read the GIC Redistributor IPRIORITYR corresponding to the
 * interrupt `id`, 4 interrupts IDs at a time.
 */
unsigned int gicr_read_ipriorityr(uintptr_t base, unsigned int id)
{
	unsigned int n = id >> IPRIORITYR_SHIFT;

	return mmio_read_32(base + GICR_IPRIORITYR + (n << 2));
}

/*
 * Accessor to write the GIC Redistributor IPRIORITYR corresponding to the
 * interrupt `id`, 4 interrupts IDs at a time.
 */
void gicr_write_ipriorityr(uintptr_t base, unsigned int id, unsigned int val)
{
	unsigned int n = id >> IPRIORITYR_SHIFT;

	mmio_write_32(base + GICR_IPRIORITYR + (n << 2), val);
}

/*
 * Accessor to set the byte corresponding to interrupt ID
 * in GIC Redistributor IPRIORITYR.
 */
void gicr_set_ipriorityr(uintptr_t base, unsigned int id, unsigned int pri)
{
	GICR_WRITE_8(IPRIORITYR, base, id, pri & GIC_PRI_MASK);
}

/*
 * Accessor to get the bit corresponding to interrupt ID
 * from GIC Redistributor IGROUPR0.
 */
unsigned int gicr_get_igroupr0(uintptr_t base, unsigned int id)
{
	unsigned int bit_num = id & ((1U << IGROUPR_SHIFT) - 1U);
	unsigned int reg_val = gicr_read_igroupr0(base);

	return (reg_val >> bit_num) & 0x1U;
}

/*
 * Accessor to set the bit corresponding to interrupt ID
 * in GIC Redistributor IGROUPR0.
 */
void gicr_set_igroupr0(uintptr_t base, unsigned int id)
{
	unsigned int bit_num = id & ((1U << IGROUPR_SHIFT) - 1U);
	unsigned int reg_val = gicr_read_igroupr0(base);

	gicr_write_igroupr0(base, reg_val | (1U << bit_num));
}

/*
 * Accessor to clear the bit corresponding to interrupt ID
 * in GIC Redistributor IGROUPR0.
 */
void gicr_clr_igroupr0(uintptr_t base, unsigned int id)
{
	unsigned int bit_num = id & ((1U << IGROUPR_SHIFT) - 1U);
	unsigned int reg_val = gicr_read_igroupr0(base);

	gicr_write_igroupr0(base, reg_val & ~(1U << bit_num));
}

/*
 * Accessor to get the bit corresponding to interrupt ID
 * from GIC Redistributor IGRPMODR0.
 */
unsigned int gicr_get_igrpmodr0(uintptr_t base, unsigned int id)
{
	unsigned int bit_num = id & ((1U << IGRPMODR_SHIFT) - 1U);
	unsigned int reg_val = gicr_read_igrpmodr0(base);

	return (reg_val >> bit_num) & 0x1U;
}

/*
 * Accessor to set the bit corresponding to interrupt ID
 * in GIC Redistributor IGRPMODR0.
 */
void gicr_set_igrpmodr0(uintptr_t base, unsigned int id)
{
	unsigned int bit_num = id & ((1U << IGRPMODR_SHIFT) - 1U);
	unsigned int reg_val = gicr_read_igrpmodr0(base);

	gicr_write_igrpmodr0(base, reg_val | (1U << bit_num));
}

/*
 * Accessor to clear the bit corresponding to interrupt ID
 * in GIC Redistributor IGRPMODR0.
 */
void gicr_clr_igrpmodr0(uintptr_t base, unsigned int id)
{
	unsigned int bit_num = id & ((1U << IGRPMODR_SHIFT) - 1U);
	unsigned int reg_val = gicr_read_igrpmodr0(base);

	gicr_write_igrpmodr0(base, reg_val & ~(1U << bit_num));
}

/*
 * Accessor to set the bit corresponding to interrupt ID
 * in GIC Redistributor ISENABLER0.
 */
void gicr_set_isenabler0(uintptr_t base, unsigned int id)
{
	unsigned int bit_num = id & ((1U << ISENABLER_SHIFT) - 1U);

	gicr_write_isenabler0(base, (1U << bit_num));
}

/*
 * Accessor to set the bit corresponding to interrupt ID in GIC Redistributor
 * ICENABLER0.
 */
void gicr_set_icenabler0(uintptr_t base, unsigned int id)
{
	unsigned int bit_num = id & ((1U << ICENABLER_SHIFT) - 1U);

	gicr_write_icenabler0(base, (1U << bit_num));
}

/*
 * Accessor to set the bit corresponding to interrupt ID in GIC Redistributor
 * ISACTIVER0.
 */
unsigned int gicr_get_isactiver0(uintptr_t base, unsigned int id)
{
	unsigned int bit_num = id & ((1U << ISACTIVER_SHIFT) - 1U);
	unsigned int reg_val = gicr_read_isactiver0(base);

	return (reg_val >> bit_num) & 0x1U;
}

/*
 * Accessor to clear the bit corresponding to interrupt ID in GIC Redistributor
 * ICPENDRR0.
 */
void gicr_set_icpendr0(uintptr_t base, unsigned int id)
{
	unsigned int bit_num = id & ((1U << ICPENDR_SHIFT) - 1U);

	gicr_write_icpendr0(base, (1U << bit_num));
}

/*
 * Accessor to set the bit corresponding to interrupt ID in GIC Redistributor
 * ISPENDR0.
 */
void gicr_set_ispendr0(uintptr_t base, unsigned int id)
{
	unsigned int bit_num = id & ((1U << ISPENDR_SHIFT) - 1U);

	gicr_write_ispendr0(base, (1U << bit_num));
}

/*
 * Accessor to set the bit fields corresponding to interrupt ID
 * in GIC Redistributor ICFGR0.
 */
void gicr_set_icfgr0(uintptr_t base, unsigned int id, unsigned int cfg)
{
	/* Interrupt configuration is a 2-bit field */
	unsigned int bit_num = id & ((1U << ICFGR_SHIFT) - 1U);
	unsigned int bit_shift = bit_num << 1U;

	uint32_t reg_val = gicr_read_icfgr0(base);

	/* Clear the field, and insert required configuration */
	reg_val &= ~(GIC_CFG_MASK << bit_shift);
	reg_val |= ((cfg & GIC_CFG_MASK) << bit_shift);

	gicr_write_icfgr0(base, reg_val);
}

/*
 * Accessor to set the bit fields corresponding to interrupt ID
 * in GIC Redistributor ICFGR1.
 */
void gicr_set_icfgr1(uintptr_t base, unsigned int id, unsigned int cfg)
{
	/* Interrupt configuration is a 2-bit field */
	unsigned int bit_num = id & ((1U << ICFGR_SHIFT) - 1U);
	unsigned int bit_shift = bit_num << 1U;

	uint32_t reg_val = gicr_read_icfgr1(base);

	/* Clear the field, and insert required configuration */
	reg_val &= ~(GIC_CFG_MASK << bit_shift);
	reg_val |= ((cfg & GIC_CFG_MASK) << bit_shift);

	gicr_write_icfgr1(base, reg_val);
}
