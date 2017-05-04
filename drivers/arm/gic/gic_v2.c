/*
 * Copyright (c) 2013-2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <assert.h>
#include <gic_v2.h>
#include <interrupt_mgmt.h>
#include <mmio.h>

/*******************************************************************************
 * GIC Distributor interface accessors for reading entire registers
 ******************************************************************************/

unsigned int gicd_read_igroupr(uintptr_t base, unsigned int id)
{
	unsigned n = id >> IGROUPR_SHIFT;
	return mmio_read_32(base + GICD_IGROUPR + (n << 2));
}

unsigned int gicd_read_isenabler(uintptr_t base, unsigned int id)
{
	unsigned n = id >> ISENABLER_SHIFT;
	return mmio_read_32(base + GICD_ISENABLER + (n << 2));
}

unsigned int gicd_read_icenabler(uintptr_t base, unsigned int id)
{
	unsigned n = id >> ICENABLER_SHIFT;
	return mmio_read_32(base + GICD_ICENABLER + (n << 2));
}

unsigned int gicd_read_ispendr(uintptr_t base, unsigned int id)
{
	unsigned n = id >> ISPENDR_SHIFT;
	return mmio_read_32(base + GICD_ISPENDR + (n << 2));
}

unsigned int gicd_read_icpendr(uintptr_t base, unsigned int id)
{
	unsigned n = id >> ICPENDR_SHIFT;
	return mmio_read_32(base + GICD_ICPENDR + (n << 2));
}

unsigned int gicd_read_isactiver(uintptr_t base, unsigned int id)
{
	unsigned n = id >> ISACTIVER_SHIFT;
	return mmio_read_32(base + GICD_ISACTIVER + (n << 2));
}

unsigned int gicd_read_icactiver(uintptr_t base, unsigned int id)
{
	unsigned n = id >> ICACTIVER_SHIFT;
	return mmio_read_32(base + GICD_ICACTIVER + (n << 2));
}

unsigned int gicd_read_ipriorityr(uintptr_t base, unsigned int id)
{
	unsigned n = id >> IPRIORITYR_SHIFT;
	return mmio_read_32(base + GICD_IPRIORITYR + (n << 2));
}

unsigned int gicd_read_itargetsr(uintptr_t base, unsigned int id)
{
	unsigned n = id >> ITARGETSR_SHIFT;
	return mmio_read_32(base + GICD_ITARGETSR + (n << 2));
}

unsigned int gicd_read_icfgr(uintptr_t base, unsigned int id)
{
	unsigned n = id >> ICFGR_SHIFT;
	return mmio_read_32(base + GICD_ICFGR + (n << 2));
}

unsigned int gicd_read_cpendsgir(uintptr_t base, unsigned int id)
{
	unsigned n = id >> CPENDSGIR_SHIFT;
	return mmio_read_32(base + GICD_CPENDSGIR + (n << 2));
}

unsigned int gicd_read_spendsgir(uintptr_t base, unsigned int id)
{
	unsigned n = id >> SPENDSGIR_SHIFT;
	return mmio_read_32(base + GICD_SPENDSGIR + (n << 2));
}

/*******************************************************************************
 * GIC Distributor interface accessors for writing entire registers
 ******************************************************************************/

void gicd_write_igroupr(uintptr_t base, unsigned int id, unsigned int val)
{
	unsigned n = id >> IGROUPR_SHIFT;
	mmio_write_32(base + GICD_IGROUPR + (n << 2), val);
}

void gicd_write_isenabler(uintptr_t base, unsigned int id, unsigned int val)
{
	unsigned n = id >> ISENABLER_SHIFT;
	mmio_write_32(base + GICD_ISENABLER + (n << 2), val);
}

void gicd_write_icenabler(uintptr_t base, unsigned int id, unsigned int val)
{
	unsigned n = id >> ICENABLER_SHIFT;
	mmio_write_32(base + GICD_ICENABLER + (n << 2), val);
}

void gicd_write_ispendr(uintptr_t base, unsigned int id, unsigned int val)
{
	unsigned n = id >> ISPENDR_SHIFT;
	mmio_write_32(base + GICD_ISPENDR + (n << 2), val);
}

void gicd_write_icpendr(uintptr_t base, unsigned int id, unsigned int val)
{
	unsigned n = id >> ICPENDR_SHIFT;
	mmio_write_32(base + GICD_ICPENDR + (n << 2), val);
}

void gicd_write_isactiver(uintptr_t base, unsigned int id, unsigned int val)
{
	unsigned n = id >> ISACTIVER_SHIFT;
	mmio_write_32(base + GICD_ISACTIVER + (n << 2), val);
}

void gicd_write_icactiver(uintptr_t base, unsigned int id, unsigned int val)
{
	unsigned n = id >> ICACTIVER_SHIFT;
	mmio_write_32(base + GICD_ICACTIVER + (n << 2), val);
}

void gicd_write_ipriorityr(uintptr_t base, unsigned int id, unsigned int val)
{
	unsigned n = id >> IPRIORITYR_SHIFT;
	mmio_write_32(base + GICD_IPRIORITYR + (n << 2), val);
}

void gicd_write_itargetsr(uintptr_t base, unsigned int id, unsigned int val)
{
	unsigned n = id >> ITARGETSR_SHIFT;
	mmio_write_32(base + GICD_ITARGETSR + (n << 2), val);
}

void gicd_write_icfgr(uintptr_t base, unsigned int id, unsigned int val)
{
	unsigned n = id >> ICFGR_SHIFT;
	mmio_write_32(base + GICD_ICFGR + (n << 2), val);
}

void gicd_write_cpendsgir(uintptr_t base, unsigned int id, unsigned int val)
{
	unsigned n = id >> CPENDSGIR_SHIFT;
	mmio_write_32(base + GICD_CPENDSGIR + (n << 2), val);
}

void gicd_write_spendsgir(uintptr_t base, unsigned int id, unsigned int val)
{
	unsigned n = id >> SPENDSGIR_SHIFT;
	mmio_write_32(base + GICD_SPENDSGIR + (n << 2), val);
}

/*******************************************************************************
 * GIC Distributor interface accessors for individual interrupt manipulation
 ******************************************************************************/
unsigned int gicd_get_igroupr(uintptr_t base, unsigned int id)
{
	unsigned bit_num = id & ((1 << IGROUPR_SHIFT) - 1);
	unsigned int reg_val = gicd_read_igroupr(base, id);

	return (reg_val >> bit_num) & 0x1;
}

void gicd_set_igroupr(uintptr_t base, unsigned int id)
{
	unsigned bit_num = id & ((1 << IGROUPR_SHIFT) - 1);
	unsigned int reg_val = gicd_read_igroupr(base, id);

	gicd_write_igroupr(base, id, reg_val | (1 << bit_num));
}

void gicd_clr_igroupr(uintptr_t base, unsigned int id)
{
	unsigned bit_num = id & ((1 << IGROUPR_SHIFT) - 1);
	unsigned int reg_val = gicd_read_igroupr(base, id);

	gicd_write_igroupr(base, id, reg_val & ~(1 << bit_num));
}

void gicd_set_isenabler(uintptr_t base, unsigned int id)
{
	unsigned bit_num = id & ((1 << ISENABLER_SHIFT) - 1);

	gicd_write_isenabler(base, id, (1 << bit_num));
}

void gicd_set_icenabler(uintptr_t base, unsigned int id)
{
	unsigned bit_num = id & ((1 << ICENABLER_SHIFT) - 1);

	gicd_write_icenabler(base, id, (1 << bit_num));
}

void gicd_set_ispendr(uintptr_t base, unsigned int id)
{
	unsigned bit_num = id & ((1 << ISPENDR_SHIFT) - 1);

	gicd_write_ispendr(base, id, (1 << bit_num));
}

void gicd_set_icpendr(uintptr_t base, unsigned int id)
{
	unsigned bit_num = id & ((1 << ICPENDR_SHIFT) - 1);

	gicd_write_icpendr(base, id, (1 << bit_num));
}

void gicd_set_isactiver(uintptr_t base, unsigned int id)
{
	unsigned bit_num = id & ((1 << ISACTIVER_SHIFT) - 1);

	gicd_write_isactiver(base, id, (1 << bit_num));
}

void gicd_set_icactiver(uintptr_t base, unsigned int id)
{
	unsigned bit_num = id & ((1 << ICACTIVER_SHIFT) - 1);

	gicd_write_icactiver(base, id, (1 << bit_num));
}

/*
 * Make sure that the interrupt's group is set before expecting
 * this function to do its job correctly.
 */
void gicd_set_ipriorityr(uintptr_t base, unsigned int id, unsigned int pri)
{
	/*
	 * Enforce ARM recommendation to manage priority values such
	 * that group1 interrupts always have a lower priority than
	 * group0 interrupts.
	 * Note, lower numerical values are higher priorities so the comparison
	 * checks below are reversed from what might be expected.
	 */
	assert(gicd_get_igroupr(base, id) == GRP1 ?
		pri >= GIC_HIGHEST_NS_PRIORITY &&
			pri <= GIC_LOWEST_NS_PRIORITY :
		pri >= GIC_HIGHEST_SEC_PRIORITY &&
			pri <= GIC_LOWEST_SEC_PRIORITY);

	mmio_write_8(base + GICD_IPRIORITYR + id, pri & GIC_PRI_MASK);
}

void gicd_set_itargetsr(uintptr_t base, unsigned int id, unsigned int target)
{
	mmio_write_8(base + GICD_ITARGETSR + id, target & GIC_TARGET_CPU_MASK);
}

/*******************************************************************************
 * This function allows the interrupt management framework to determine (through
 * the platform) which interrupt line (IRQ/FIQ) to use for an interrupt type to
 * route it to EL3. The interrupt line is represented as the bit position of the
 * IRQ or FIQ bit in the SCR_EL3.
 ******************************************************************************/
uint32_t gicv2_interrupt_type_to_line(uint32_t cpuif_base, uint32_t type)
{
	uint32_t gicc_ctlr;

	/* Non-secure interrupts are signalled on the IRQ line always */
	if (type == INTR_TYPE_NS)
		return __builtin_ctz(SCR_IRQ_BIT);

	/*
	 * Secure interrupts are signalled using the IRQ line if the FIQ_EN
	 * bit is not set else they are signalled using the FIQ line.
	 */
	gicc_ctlr = gicc_read_ctlr(cpuif_base);
	if (gicc_ctlr & FIQ_EN)
		return __builtin_ctz(SCR_FIQ_BIT);
	else
		return __builtin_ctz(SCR_IRQ_BIT);
}
