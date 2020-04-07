/*
 * Copyright (c) 2015-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include "gicv3_private.h"

/*******************************************************************************
 * GIC Distributor functions for accessing the GIC registers
 * corresponding to a single interrupt ID. These functions use bitwise
 * operations or appropriate register accesses to modify or return
 * the bit-field corresponding the single interrupt ID.
 ******************************************************************************/

/*
 * Accessors to set the bits corresponding to interrupt ID
 * in GIC Distributor ICFGR and ICFGRE.
 */
void gicd_set_icfgr(uintptr_t base, unsigned int id, unsigned int cfg)
{
	/* Interrupt configuration is a 2-bit field */
	unsigned int bit_shift = BIT_NUM(ICFG, id) << 1U;

	/* Clear the field, and insert required configuration */
	mmio_clrsetbits_32(base + GICD_OFFSET(ICFG, id),
				(uint32_t)GIC_CFG_MASK << bit_shift,
				(cfg & GIC_CFG_MASK) << bit_shift);
}

/*
 * Accessors to get/set/clear the bit corresponding to interrupt ID
 * in GIC Distributor IGROUPR and IGROUPRE.
 */
unsigned int gicd_get_igroupr(uintptr_t base, unsigned int id)
{
	return GICD_GET_BIT(IGROUP, base, id);
}

void gicd_set_igroupr(uintptr_t base, unsigned int id)
{
	GICD_SET_BIT(IGROUP, base, id);
}

void gicd_clr_igroupr(uintptr_t base, unsigned int id)
{
	GICD_CLR_BIT(IGROUP, base, id);
}

/*
 * Accessors to get/set/clear the bit corresponding to interrupt ID
 * in GIC Distributor IGRPMODR and IGRPMODRE.
 */
unsigned int gicd_get_igrpmodr(uintptr_t base, unsigned int id)
{
	return GICD_GET_BIT(IGRPMOD, base, id);
}

void gicd_set_igrpmodr(uintptr_t base, unsigned int id)
{
	GICD_SET_BIT(IGRPMOD, base, id);
}

void gicd_clr_igrpmodr(uintptr_t base, unsigned int id)
{
	GICD_CLR_BIT(IGRPMOD, base, id);
}

/*
 * Accessors to set the bit corresponding to interrupt ID
 * in GIC Distributor ICENABLER and ICENABLERE.
 */
void gicd_set_icenabler(uintptr_t base, unsigned int id)
{
	GICD_WRITE_BIT(ICENABLE, base, id);
}

/*
 * Accessors to set the bit corresponding to interrupt ID
 * in GIC Distributor ICPENDR and ICPENDRE.
 */
void gicd_set_icpendr(uintptr_t base, unsigned int id)
{
	GICD_WRITE_BIT(ICPEND, base, id);
}

/*
 * Accessors to get/set the bit corresponding to interrupt ID
 * in GIC Distributor ISACTIVER and ISACTIVERE.
 */
unsigned int gicd_get_isactiver(uintptr_t base, unsigned int id)
{
	return GICD_GET_BIT(ISACTIVE, base, id);
}

void gicd_set_isactiver(uintptr_t base, unsigned int id)
{
	GICD_WRITE_BIT(ISACTIVE, base, id);
}

/*
 * Accessors to set the bit corresponding to interrupt ID
 * in GIC Distributor ISENABLER and ISENABLERE.
 */
void gicd_set_isenabler(uintptr_t base, unsigned int id)
{
	GICD_WRITE_BIT(ISENABLE, base, id);
}

/*
 * Accessors to set the bit corresponding to interrupt ID
 * in GIC Distributor ISPENDR and ISPENDRE.
 */
void gicd_set_ispendr(uintptr_t base, unsigned int id)
{
	GICD_WRITE_BIT(ISPEND, base, id);
}

/*
 * Accessors to set the bit corresponding to interrupt ID
 * in GIC Distributor IPRIORITYR and IPRIORITYRE.
 */
void gicd_set_ipriorityr(uintptr_t base, unsigned int id, unsigned int pri)
{
	GICD_WRITE_8(IPRIORITY, base, id, (uint8_t)(pri & GIC_PRI_MASK));
}

/*******************************************************************************
 * GIC Distributor interface accessors for reading/writing entire registers
 ******************************************************************************/

/*
 * Accessors to read/write the GIC Distributor ICGFR and ICGFRE
 * corresponding to the interrupt ID, 16 interrupt IDs at a time.
 */
unsigned int gicd_read_icfgr(uintptr_t base, unsigned int id)
{
	return GICD_READ(ICFG, base, id);
}

void gicd_write_icfgr(uintptr_t base, unsigned int id, unsigned int val)
{
	GICD_WRITE(ICFG, base, id, val);
}

/*
 * Accessors to read/write the GIC Distributor IGROUPR and IGROUPRE
 * corresponding to the interrupt ID, 32 interrupt IDs at a time.
 */
unsigned int gicd_read_igroupr(uintptr_t base, unsigned int id)
{
	return GICD_READ(IGROUP, base, id);
}

void gicd_write_igroupr(uintptr_t base, unsigned int id, unsigned int val)
{
	GICD_WRITE(IGROUP, base, id, val);
}

/*
 * Accessors to read/write the GIC Distributor IGRPMODR and IGRPMODRE
 * corresponding to the interrupt ID, 32 interrupt IDs at a time.
 */
unsigned int gicd_read_igrpmodr(uintptr_t base, unsigned int id)
{
	return GICD_READ(IGRPMOD, base, id);
}

void gicd_write_igrpmodr(uintptr_t base, unsigned int id, unsigned int val)
{
	GICD_WRITE(IGRPMOD, base, id, val);
}

/*
 * Accessors to read/write the GIC Distributor IPRIORITYR and IPRIORITYRE
 * corresponding to the interrupt ID, 4 interrupt IDs at a time.
 */
unsigned int gicd_read_ipriorityr(uintptr_t base, unsigned int id)
{
	return GICD_READ(IPRIORITY, base, id);
}

void gicd_write_ipriorityr(uintptr_t base, unsigned int id, unsigned int val)
{
	GICD_WRITE(IPRIORITY, base, id, val);
}

/*
 * Accessors to read/write the GIC Distributor ISACTIVER and ISACTIVERE
 * corresponding to the interrupt ID, 32 interrupt IDs at a time.
 */
unsigned int gicd_read_isactiver(uintptr_t base, unsigned int id)
{
	return GICD_READ(ISACTIVE, base, id);
}

void gicd_write_isactiver(uintptr_t base, unsigned int id, unsigned int val)
{
	GICD_WRITE(ISACTIVE, base, id, val);
}

/*
 * Accessors to read/write the GIC Distributor ISENABLER and ISENABLERE
 * corresponding to the interrupt ID, 32 interrupt IDs at a time.
 */
unsigned int gicd_read_isenabler(uintptr_t base, unsigned int id)
{
	return GICD_READ(ISENABLE, base, id);
}

void gicd_write_isenabler(uintptr_t base, unsigned int id, unsigned int val)
{
	GICD_WRITE(ISENABLE, base, id, val);
}

/*
 * Accessors to read/write the GIC Distributor ISPENDR and ISPENDRE
 * corresponding to the interrupt ID, 32 interrupt IDs at a time.
 */
unsigned int gicd_read_ispendr(uintptr_t base, unsigned int id)
{
	return GICD_READ(ISPEND, base, id);
}

void gicd_write_ispendr(uintptr_t base, unsigned int id, unsigned int val)
{
	GICD_WRITE(ISPEND, base, id, val);
}

/*
 * Accessors to read/write the GIC Distributor NSACR and NSACRE
 * corresponding to the interrupt ID, 16 interrupt IDs at a time.
 */
unsigned int gicd_read_nsacr(uintptr_t base, unsigned int id)
{
	return GICD_READ(NSAC, base, id);
}

void gicd_write_nsacr(uintptr_t base, unsigned int id, unsigned int val)
{
	GICD_WRITE(NSAC, base, id, val);
}
