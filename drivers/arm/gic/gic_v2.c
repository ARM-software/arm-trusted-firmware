/*
 * Copyright (c) 2013-2014, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <arch.h>
#include <assert.h>
#include <gic_v2.h>
#include <interrupt_mgmt.h>
#include <mmio.h>

/*******************************************************************************
 * GIC Distributor interface accessors for reading entire registers
 ******************************************************************************/

unsigned int gicd_read_igroupr(unsigned int base, unsigned int id)
{
	unsigned n = id >> IGROUPR_SHIFT;
	return mmio_read_32(base + GICD_IGROUPR + (n << 2));
}

unsigned int gicd_read_isenabler(unsigned int base, unsigned int id)
{
	unsigned n = id >> ISENABLER_SHIFT;
	return mmio_read_32(base + GICD_ISENABLER + (n << 2));
}

unsigned int gicd_read_icenabler(unsigned int base, unsigned int id)
{
	unsigned n = id >> ICENABLER_SHIFT;
	return mmio_read_32(base + GICD_ICENABLER + (n << 2));
}

unsigned int gicd_read_ispendr(unsigned int base, unsigned int id)
{
	unsigned n = id >> ISPENDR_SHIFT;
	return mmio_read_32(base + GICD_ISPENDR + (n << 2));
}

unsigned int gicd_read_icpendr(unsigned int base, unsigned int id)
{
	unsigned n = id >> ICPENDR_SHIFT;
	return mmio_read_32(base + GICD_ICPENDR + (n << 2));
}

unsigned int gicd_read_isactiver(unsigned int base, unsigned int id)
{
	unsigned n = id >> ISACTIVER_SHIFT;
	return mmio_read_32(base + GICD_ISACTIVER + (n << 2));
}

unsigned int gicd_read_icactiver(unsigned int base, unsigned int id)
{
	unsigned n = id >> ICACTIVER_SHIFT;
	return mmio_read_32(base + GICD_ICACTIVER + (n << 2));
}

unsigned int gicd_read_ipriorityr(unsigned int base, unsigned int id)
{
	unsigned n = id >> IPRIORITYR_SHIFT;
	return mmio_read_32(base + GICD_IPRIORITYR + (n << 2));
}

unsigned int gicd_read_itargetsr(unsigned int base, unsigned int id)
{
	unsigned n = id >> ITARGETSR_SHIFT;
	return mmio_read_32(base + GICD_ITARGETSR + (n << 2));
}

unsigned int gicd_read_icfgr(unsigned int base, unsigned int id)
{
	unsigned n = id >> ICFGR_SHIFT;
	return mmio_read_32(base + GICD_ICFGR + (n << 2));
}

unsigned int gicd_read_cpendsgir(unsigned int base, unsigned int id)
{
	unsigned n = id >> CPENDSGIR_SHIFT;
	return mmio_read_32(base + GICD_CPENDSGIR + (n << 2));
}

unsigned int gicd_read_spendsgir(unsigned int base, unsigned int id)
{
	unsigned n = id >> SPENDSGIR_SHIFT;
	return mmio_read_32(base + GICD_SPENDSGIR + (n << 2));
}

/*******************************************************************************
 * GIC Distributor interface accessors for writing entire registers
 ******************************************************************************/

void gicd_write_igroupr(unsigned int base, unsigned int id, unsigned int val)
{
	unsigned n = id >> IGROUPR_SHIFT;
	mmio_write_32(base + GICD_IGROUPR + (n << 2), val);
}

void gicd_write_isenabler(unsigned int base, unsigned int id, unsigned int val)
{
	unsigned n = id >> ISENABLER_SHIFT;
	mmio_write_32(base + GICD_ISENABLER + (n << 2), val);
}

void gicd_write_icenabler(unsigned int base, unsigned int id, unsigned int val)
{
	unsigned n = id >> ICENABLER_SHIFT;
	mmio_write_32(base + GICD_ICENABLER + (n << 2), val);
}

void gicd_write_ispendr(unsigned int base, unsigned int id, unsigned int val)
{
	unsigned n = id >> ISPENDR_SHIFT;
	mmio_write_32(base + GICD_ISPENDR + (n << 2), val);
}

void gicd_write_icpendr(unsigned int base, unsigned int id, unsigned int val)
{
	unsigned n = id >> ICPENDR_SHIFT;
	mmio_write_32(base + GICD_ICPENDR + (n << 2), val);
}

void gicd_write_isactiver(unsigned int base, unsigned int id, unsigned int val)
{
	unsigned n = id >> ISACTIVER_SHIFT;
	mmio_write_32(base + GICD_ISACTIVER + (n << 2), val);
}

void gicd_write_icactiver(unsigned int base, unsigned int id, unsigned int val)
{
	unsigned n = id >> ICACTIVER_SHIFT;
	mmio_write_32(base + GICD_ICACTIVER + (n << 2), val);
}

void gicd_write_ipriorityr(unsigned int base, unsigned int id, unsigned int val)
{
	unsigned n = id >> IPRIORITYR_SHIFT;
	mmio_write_32(base + GICD_IPRIORITYR + (n << 2), val);
}

void gicd_write_itargetsr(unsigned int base, unsigned int id, unsigned int val)
{
	unsigned n = id >> ITARGETSR_SHIFT;
	mmio_write_32(base + GICD_ITARGETSR + (n << 2), val);
}

void gicd_write_icfgr(unsigned int base, unsigned int id, unsigned int val)
{
	unsigned n = id >> ICFGR_SHIFT;
	mmio_write_32(base + GICD_ICFGR + (n << 2), val);
}

void gicd_write_cpendsgir(unsigned int base, unsigned int id, unsigned int val)
{
	unsigned n = id >> CPENDSGIR_SHIFT;
	mmio_write_32(base + GICD_CPENDSGIR + (n << 2), val);
}

void gicd_write_spendsgir(unsigned int base, unsigned int id, unsigned int val)
{
	unsigned n = id >> SPENDSGIR_SHIFT;
	mmio_write_32(base + GICD_SPENDSGIR + (n << 2), val);
}

/*******************************************************************************
 * GIC Distributor interface accessors for individual interrupt manipulation
 ******************************************************************************/
unsigned int gicd_get_igroupr(unsigned int base, unsigned int id)
{
	unsigned bit_num = id & ((1 << IGROUPR_SHIFT) - 1);
	unsigned int reg_val = gicd_read_igroupr(base, id);

	return (reg_val >> bit_num) & 0x1;
}

void gicd_set_igroupr(unsigned int base, unsigned int id)
{
	unsigned bit_num = id & ((1 << IGROUPR_SHIFT) - 1);
	unsigned int reg_val = gicd_read_igroupr(base, id);

	gicd_write_igroupr(base, id, reg_val | (1 << bit_num));
}

void gicd_clr_igroupr(unsigned int base, unsigned int id)
{
	unsigned bit_num = id & ((1 << IGROUPR_SHIFT) - 1);
	unsigned int reg_val = gicd_read_igroupr(base, id);

	gicd_write_igroupr(base, id, reg_val & ~(1 << bit_num));
}

void gicd_set_isenabler(unsigned int base, unsigned int id)
{
	unsigned bit_num = id & ((1 << ISENABLER_SHIFT) - 1);

	gicd_write_isenabler(base, id, (1 << bit_num));
}

void gicd_set_icenabler(unsigned int base, unsigned int id)
{
	unsigned bit_num = id & ((1 << ICENABLER_SHIFT) - 1);

	gicd_write_icenabler(base, id, (1 << bit_num));
}

void gicd_set_ispendr(unsigned int base, unsigned int id)
{
	unsigned bit_num = id & ((1 << ISPENDR_SHIFT) - 1);

	gicd_write_ispendr(base, id, (1 << bit_num));
}

void gicd_set_icpendr(unsigned int base, unsigned int id)
{
	unsigned bit_num = id & ((1 << ICPENDR_SHIFT) - 1);

	gicd_write_icpendr(base, id, (1 << bit_num));
}

void gicd_set_isactiver(unsigned int base, unsigned int id)
{
	unsigned bit_num = id & ((1 << ISACTIVER_SHIFT) - 1);

	gicd_write_isactiver(base, id, (1 << bit_num));
}

void gicd_set_icactiver(unsigned int base, unsigned int id)
{
	unsigned bit_num = id & ((1 << ICACTIVER_SHIFT) - 1);

	gicd_write_icactiver(base, id, (1 << bit_num));
}

/*
 * Make sure that the interrupt's group is set before expecting
 * this function to do its job correctly.
 */
void gicd_set_ipriorityr(unsigned int base, unsigned int id, unsigned int pri)
{
	unsigned int reg = base + GICD_IPRIORITYR + (id & ~3);
	unsigned int shift = (id & 3) << 3;
	unsigned int reg_val = mmio_read_32(reg);

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

	reg_val &= ~(GIC_PRI_MASK << shift);
	reg_val |= (pri & GIC_PRI_MASK) << shift;
	mmio_write_32(reg, reg_val);
}

void gicd_set_itargetsr(unsigned int base, unsigned int id, unsigned int iface)
{
	unsigned byte_off = id & ((1 << ITARGETSR_SHIFT) - 1);
	unsigned int reg_val = gicd_read_itargetsr(base, id);

	gicd_write_itargetsr(base, id, reg_val |
			     (1 << iface) << (byte_off << 3));
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
