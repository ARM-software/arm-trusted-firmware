/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
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
#include <arch_helpers.h>
#include <assert.h>
#include <debug.h>
#include <gic_common.h>
#include "gicv2_private.h"

/*
 * Accessor to read the GIC Distributor ITARGETSR corresponding to the
 * interrupt `id`, 4 interrupt IDs at a time.
 */
unsigned int gicd_read_itargetsr(uintptr_t base, unsigned int id)
{
	unsigned n = id >> ITARGETSR_SHIFT;
	return mmio_read_32(base + GICD_ITARGETSR + (n << 2));
}

/*
 * Accessor to read the GIC Distributor CPENDSGIR corresponding to the
 * interrupt `id`, 4 interrupt IDs at a time.
 */
unsigned int gicd_read_cpendsgir(uintptr_t base, unsigned int id)
{
	unsigned n = id >> CPENDSGIR_SHIFT;
	return mmio_read_32(base + GICD_CPENDSGIR + (n << 2));
}

/*
 * Accessor to read the GIC Distributor SPENDSGIR corresponding to the
 * interrupt `id`, 4 interrupt IDs at a time.
 */
unsigned int gicd_read_spendsgir(uintptr_t base, unsigned int id)
{
	unsigned n = id >> SPENDSGIR_SHIFT;
	return mmio_read_32(base + GICD_SPENDSGIR + (n << 2));
}

/*
 * Accessor to write the GIC Distributor ITARGETSR corresponding to the
 * interrupt `id`, 4 interrupt IDs at a time.
 */
void gicd_write_itargetsr(uintptr_t base, unsigned int id, unsigned int val)
{
	unsigned n = id >> ITARGETSR_SHIFT;
	mmio_write_32(base + GICD_ITARGETSR + (n << 2), val);
}

/*
 * Accessor to write the GIC Distributor CPENDSGIR corresponding to the
 * interrupt `id`, 4 interrupt IDs at a time.
 */
void gicd_write_cpendsgir(uintptr_t base, unsigned int id, unsigned int val)
{
	unsigned n = id >> CPENDSGIR_SHIFT;
	mmio_write_32(base + GICD_CPENDSGIR + (n << 2), val);
}

/*
 * Accessor to write the GIC Distributor SPENDSGIR corresponding to the
 * interrupt `id`, 4 interrupt IDs at a time.
 */
void gicd_write_spendsgir(uintptr_t base, unsigned int id, unsigned int val)
{
	unsigned n = id >> SPENDSGIR_SHIFT;
	mmio_write_32(base + GICD_SPENDSGIR + (n << 2), val);
}

/*
 * Accessor to write the GIC Distributor ITARGETSR corresponding to the
 * interrupt `id`.
 */
void gicd_set_itargetsr(uintptr_t base, unsigned int id, unsigned int target)
{
	unsigned byte_off = id & ((1 << ITARGETSR_SHIFT) - 1);
	unsigned int reg_val = gicd_read_itargetsr(base, id);

	gicd_write_itargetsr(base, id, reg_val | (target << (byte_off << 3)));
}

/*******************************************************************************
 * Get the current CPU bit mask from GICD_ITARGETSR0
 ******************************************************************************/
unsigned int gicv2_get_cpuif_id(uintptr_t base)
{
	unsigned int val;

	val = gicd_read_itargetsr(base, 0);
	return val & GIC_TARGET_CPU_MASK;
}

/*******************************************************************************
 * Helper function to configure the default attributes of SPIs.
 ******************************************************************************/
void gicv2_spis_configure_defaults(uintptr_t gicd_base)
{
	unsigned int index, num_ints;

	num_ints = gicd_read_typer(gicd_base);
	num_ints &= TYPER_IT_LINES_NO_MASK;
	num_ints = (num_ints + 1) << 5;

	/*
	 * Treat all SPIs as G1NS by default. The number of interrupts is
	 * calculated as 32 * (IT_LINES + 1). We do 32 at a time.
	 */
	for (index = MIN_SPI_ID; index < num_ints; index += 32)
		gicd_write_igroupr(gicd_base, index, ~0U);

	/* Setup the default SPI priorities doing four at a time */
	for (index = MIN_SPI_ID; index < num_ints; index += 4)
		gicd_write_ipriorityr(gicd_base,
				      index,
				      GICD_IPRIORITYR_DEF_VAL);

	/* Treat all SPIs as level triggered by default, 16 at a time */
	for (index = MIN_SPI_ID; index < num_ints; index += 16)
		gicd_write_icfgr(gicd_base, index, 0);
}

/*******************************************************************************
 * Helper function to configure secure G0 SPIs.
 ******************************************************************************/
void gicv2_secure_spis_configure(uintptr_t gicd_base,
				     unsigned int num_ints,
				     const unsigned int *sec_intr_list)
{
	unsigned int index, irq_num;

	/* If `num_ints` is not 0, ensure that `sec_intr_list` is not NULL */
	assert(num_ints ? (uintptr_t)sec_intr_list : 1);

	for (index = 0; index < num_ints; index++) {
		irq_num = sec_intr_list[index];
		if (irq_num >= MIN_SPI_ID) {
			/* Configure this interrupt as a secure interrupt */
			gicd_clr_igroupr(gicd_base, irq_num);

			/* Set the priority of this interrupt */
			gicd_write_ipriorityr(gicd_base,
					      irq_num,
					      GIC_HIGHEST_SEC_PRIORITY);

			/* Target the secure interrupts to primary CPU */
			gicd_set_itargetsr(gicd_base, irq_num,
					gicv2_get_cpuif_id(gicd_base));

			/* Enable this interrupt */
			gicd_set_isenabler(gicd_base, irq_num);
		}
	}

}

/*******************************************************************************
 * Helper function to configure secure G0 SGIs and PPIs.
 ******************************************************************************/
void gicv2_secure_ppi_sgi_setup(uintptr_t gicd_base,
					unsigned int num_ints,
					const unsigned int *sec_intr_list)
{
	unsigned int index, irq_num, sec_ppi_sgi_mask = 0;

	/* If `num_ints` is not 0, ensure that `sec_intr_list` is not NULL */
	assert(num_ints ? (uintptr_t)sec_intr_list : 1);

	/*
	 * Disable all SGIs (imp. def.)/PPIs before configuring them. This is a
	 * more scalable approach as it avoids clearing the enable bits in the
	 * GICD_CTLR.
	 */
	gicd_write_icenabler(gicd_base, 0, ~0);

	/* Setup the default PPI/SGI priorities doing four at a time */
	for (index = 0; index < MIN_SPI_ID; index += 4)
		gicd_write_ipriorityr(gicd_base,
				      index,
				      GICD_IPRIORITYR_DEF_VAL);

	for (index = 0; index < num_ints; index++) {
		irq_num = sec_intr_list[index];
		if (irq_num < MIN_SPI_ID) {
			/* We have an SGI or a PPI. They are Group0 at reset */
			sec_ppi_sgi_mask |= 1U << irq_num;

			/* Set the priority of this interrupt */
			gicd_write_ipriorityr(gicd_base,
					    irq_num,
					    GIC_HIGHEST_SEC_PRIORITY);
		}
	}

	/*
	 * Invert the bitmask to create a mask for non-secure PPIs and
	 * SGIs. Program the GICD_IGROUPR0 with this bit mask.
	 */
	gicd_write_igroupr(gicd_base, 0, ~sec_ppi_sgi_mask);

	/* Enable the Group 0 SGIs and PPIs */
	gicd_write_isenabler(gicd_base, 0, sec_ppi_sgi_mask);
}
