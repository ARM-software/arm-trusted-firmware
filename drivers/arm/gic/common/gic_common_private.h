/*
 * Copyright (c) 2016-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef GIC_COMMON_PRIVATE_H
#define GIC_COMMON_PRIVATE_H

#include <stdint.h>

#include <drivers/arm/gic_common.h>
#include <lib/mmio.h>

/*******************************************************************************
 * GIC Distributor interface register accessors that are common to GICv3 & GICv2
 ******************************************************************************/
static inline unsigned int gicd_read_ctlr(uintptr_t base)
{
	return mmio_read_32(base + GICD_CTLR);
}

static inline unsigned int gicd_read_typer(uintptr_t base)
{
	return mmio_read_32(base + GICD_TYPER);
}

static inline unsigned int gicd_read_iidr(uintptr_t base)
{
	return mmio_read_32(base + GICD_IIDR);
}

static inline void gicd_write_ctlr(uintptr_t base, unsigned int val)
{
	mmio_write_32(base + GICD_CTLR, val);
}

/*******************************************************************************
 * GIC Distributor function prototypes for accessing entire registers.
 * Note: The raw register values correspond to multiple interrupt IDs and
 * the number of interrupt IDs involved depends on the register accessed.
 ******************************************************************************/
unsigned int gicd_read_igroupr(uintptr_t base, unsigned int id);
unsigned int gicd_read_isenabler(uintptr_t base, unsigned int id);
unsigned int gicd_read_icenabler(uintptr_t base, unsigned int id);
unsigned int gicd_read_ispendr(uintptr_t base, unsigned int id);
unsigned int gicd_read_icpendr(uintptr_t base, unsigned int id);
unsigned int gicd_read_isactiver(uintptr_t base, unsigned int id);
unsigned int gicd_read_icactiver(uintptr_t base, unsigned int id);
unsigned int gicd_read_ipriorityr(uintptr_t base, unsigned int id);
unsigned int gicd_read_icfgr(uintptr_t base, unsigned int id);
unsigned int gicd_read_nsacr(uintptr_t base, unsigned int id);
unsigned int gicd_read_spendsgir(uintptr_t base, unsigned int id);
unsigned int gicd_read_cpendsgir(uintptr_t base, unsigned int id);
unsigned int gicd_read_itargetsr(uintptr_t base, unsigned int id);
void gicd_write_igroupr(uintptr_t base, unsigned int id, unsigned int val);
void gicd_write_isenabler(uintptr_t base, unsigned int id, unsigned int val);
void gicd_write_icenabler(uintptr_t base, unsigned int id, unsigned int val);
void gicd_write_ispendr(uintptr_t base, unsigned int id, unsigned int val);
void gicd_write_icpendr(uintptr_t base, unsigned int id, unsigned int val);
void gicd_write_isactiver(uintptr_t base, unsigned int id, unsigned int val);
void gicd_write_icactiver(uintptr_t base, unsigned int id, unsigned int val);
void gicd_write_ipriorityr(uintptr_t base, unsigned int id, unsigned int val);
void gicd_write_icfgr(uintptr_t base, unsigned int id, unsigned int val);
void gicd_write_nsacr(uintptr_t base, unsigned int id, unsigned int val);
void gicd_write_spendsgir(uintptr_t base, unsigned int id, unsigned int val);
void gicd_write_cpendsgir(uintptr_t base, unsigned int id, unsigned int val);
void gicd_write_itargetsr(uintptr_t base, unsigned int id, unsigned int val);

/*******************************************************************************
 * GIC Distributor function prototypes for accessing the GIC registers
 * corresponding to a single interrupt ID. These functions use bitwise
 * operations or appropriate register accesses to modify or return
 * the bit-field corresponding the single interrupt ID.
 ******************************************************************************/
unsigned int gicd_get_igroupr(uintptr_t base, unsigned int id);
void gicd_set_igroupr(uintptr_t base, unsigned int id);
void gicd_clr_igroupr(uintptr_t base, unsigned int id);
void gicd_set_isenabler(uintptr_t base, unsigned int id);
void gicd_set_icenabler(uintptr_t base, unsigned int id);
void gicd_set_ispendr(uintptr_t base, unsigned int id);
void gicd_set_icpendr(uintptr_t base, unsigned int id);
unsigned int gicd_get_isactiver(uintptr_t base, unsigned int id);
void gicd_set_isactiver(uintptr_t base, unsigned int id);
void gicd_set_icactiver(uintptr_t base, unsigned int id);
void gicd_set_ipriorityr(uintptr_t base, unsigned int id, unsigned int pri);
void gicd_set_icfgr(uintptr_t base, unsigned int id, unsigned int cfg);

#endif /* GIC_COMMON_PRIVATE_H */
