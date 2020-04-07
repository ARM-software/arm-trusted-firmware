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
 * Note: The raw register values correspond to multiple interrupt `id`s and
 * the number of interrupt `id`s involved depends on the register accessed.
 ******************************************************************************/

/*
 * Accessors to read/write the GIC Redistributor IPRIORITYR and IPRIORITYRE
 * register corresponding to the interrupt `id`, 4 interrupts IDs at a time.
 */
unsigned int gicr_read_ipriorityr(uintptr_t base, unsigned int id)
{
	return GICR_READ(IPRIORITY, base, id);
}

void gicr_write_ipriorityr(uintptr_t base, unsigned int id, unsigned int val)
{
	GICR_WRITE(IPRIORITY, base, id, val);
}

/*
 * Accessor to set the byte corresponding to interrupt `id`
 * in GIC Redistributor IPRIORITYR and IPRIORITYRE.
 */
void gicr_set_ipriorityr(uintptr_t base, unsigned int id, unsigned int pri)
{
	GICR_WRITE_8(IPRIORITY, base, id, (uint8_t)(pri & GIC_PRI_MASK));
}

/*
 * Accessors to get/set/clear the bit corresponding to interrupt `id`
 * from GIC Redistributor IGROUPR0 and IGROUPRE
 */
unsigned int gicr_get_igroupr(uintptr_t base, unsigned int id)
{
	return GICR_GET_BIT(IGROUP, base, id);
}

void gicr_set_igroupr(uintptr_t base, unsigned int id)
{
	GICR_SET_BIT(IGROUP, base, id);
}

void gicr_clr_igroupr(uintptr_t base, unsigned int id)
{
	GICR_CLR_BIT(IGROUP, base, id);
}

/*
 * Accessors to get/set/clear the bit corresponding to interrupt `id`
 * from GIC Redistributor IGRPMODR0 and IGRPMODRE
 */
unsigned int gicr_get_igrpmodr(uintptr_t base, unsigned int id)
{
	return GICR_GET_BIT(IGRPMOD, base, id);
}

void gicr_set_igrpmodr(uintptr_t base, unsigned int id)
{
	GICR_SET_BIT(IGRPMOD, base, id);
}

void gicr_clr_igrpmodr(uintptr_t base, unsigned int id)
{
	GICR_CLR_BIT(IGRPMOD, base, id);
}

/*
 * Accessor to write the bit corresponding to interrupt `id`
 * in GIC Redistributor ISENABLER0 and ISENABLERE
 */
void gicr_set_isenabler(uintptr_t base, unsigned int id)
{
	GICR_WRITE_BIT(ISENABLE, base, id);
}

/*
 * Accessor to write the bit corresponding to interrupt `id`
 * in GIC Redistributor ICENABLER0 and ICENABLERE
 */
void gicr_set_icenabler(uintptr_t base, unsigned int id)
{
	GICR_WRITE_BIT(ICENABLE, base, id);
}

/*
 * Accessor to get the bit corresponding to interrupt `id`
 * in GIC Redistributor ISACTIVER0 and ISACTIVERE
 */
unsigned int gicr_get_isactiver(uintptr_t base, unsigned int id)
{
	return	GICR_GET_BIT(ISACTIVE, base, id);
}

/*
 * Accessor to clear the bit corresponding to interrupt `id`
 * in GIC Redistributor ICPENDR0 and ICPENDRE
 */
void gicr_set_icpendr(uintptr_t base, unsigned int id)
{
	GICR_WRITE_BIT(ICPEND, base, id);
}

/*
 * Accessor to write the bit corresponding to interrupt `id`
 * in GIC Redistributor ISPENDR0 and ISPENDRE
 */
void gicr_set_ispendr(uintptr_t base, unsigned int id)
{
	GICR_WRITE_BIT(ISPEND, base, id);
}

/*
 * Accessor to set the bit fields corresponding to interrupt `id`
 * in GIC Redistributor ICFGR0, ICFGR1 and ICFGRE
 */
void gicr_set_icfgr(uintptr_t base, unsigned int id, unsigned int cfg)
{
	/* Interrupt configuration is a 2-bit field */
	unsigned int bit_shift = BIT_NUM(ICFG, id) << 1U;

	/* Clear the field, and insert required configuration */
	mmio_clrsetbits_32(base + GICR_OFFSET(ICFG, id),
				(uint32_t)GIC_CFG_MASK << bit_shift,
				(cfg & GIC_CFG_MASK) << bit_shift);
}
