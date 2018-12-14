/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <arch.h>
#include <arch_helpers.h>
#include <common/debug.h>
#include <common/interrupt_props.h>
#include <drivers/arm/gic_common.h>
#include <drivers/arm/gicv2.h>

#include "../common/gic_common_private.h"
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
	num_ints = (num_ints + 1U) << 5;

	/*
	 * Treat all SPIs as G1NS by default. The number of interrupts is
	 * calculated as 32 * (IT_LINES + 1). We do 32 at a time.
	 */
	for (index = MIN_SPI_ID; index < num_ints; index += 32U)
		gicd_write_igroupr(gicd_base, index, ~0U);

	/* Setup the default SPI priorities doing four at a time */
	for (index = MIN_SPI_ID; index < num_ints; index += 4U)
		gicd_write_ipriorityr(gicd_base,
				      index,
				      GICD_IPRIORITYR_DEF_VAL);

	/* Treat all SPIs as level triggered by default, 16 at a time */
	for (index = MIN_SPI_ID; index < num_ints; index += 16U)
		gicd_write_icfgr(gicd_base, index, 0U);
}

/*******************************************************************************
 * Helper function to configure properties of secure G0 SPIs.
 ******************************************************************************/
void gicv2_secure_spis_configure_props(uintptr_t gicd_base,
		const interrupt_prop_t *interrupt_props,
		unsigned int interrupt_props_num)
{
	unsigned int i;
	const interrupt_prop_t *prop_desc;

	/* Make sure there's a valid property array */
	if (interrupt_props_num != 0U)
		assert(interrupt_props != NULL);

	for (i = 0; i < interrupt_props_num; i++) {
		prop_desc = &interrupt_props[i];

		if (prop_desc->intr_num < MIN_SPI_ID)
			continue;

		/* Configure this interrupt as a secure interrupt */
		assert(prop_desc->intr_grp == GICV2_INTR_GROUP0);
		gicd_clr_igroupr(gicd_base, prop_desc->intr_num);

		/* Set the priority of this interrupt */
		gicd_set_ipriorityr(gicd_base, prop_desc->intr_num,
				prop_desc->intr_pri);

		/* Target the secure interrupts to primary CPU */
		gicd_set_itargetsr(gicd_base, prop_desc->intr_num,
				gicv2_get_cpuif_id(gicd_base));

		/* Set interrupt configuration */
		gicd_set_icfgr(gicd_base, prop_desc->intr_num,
				prop_desc->intr_cfg);

		/* Enable this interrupt */
		gicd_set_isenabler(gicd_base, prop_desc->intr_num);
	}
}

/*******************************************************************************
 * Helper function to configure properties of secure G0 SGIs and PPIs.
 ******************************************************************************/
void gicv2_secure_ppi_sgi_setup_props(uintptr_t gicd_base,
		const interrupt_prop_t *interrupt_props,
		unsigned int interrupt_props_num)
{
	unsigned int i;
	uint32_t sec_ppi_sgi_mask = 0;
	const interrupt_prop_t *prop_desc;

	/* Make sure there's a valid property array */
	if (interrupt_props_num != 0U)
		assert(interrupt_props != NULL);

	/*
	 * Disable all SGIs (imp. def.)/PPIs before configuring them. This is a
	 * more scalable approach as it avoids clearing the enable bits in the
	 * GICD_CTLR.
	 */
	gicd_write_icenabler(gicd_base, 0U, ~0U);

	/* Setup the default PPI/SGI priorities doing four at a time */
	for (i = 0U; i < MIN_SPI_ID; i += 4U)
		gicd_write_ipriorityr(gicd_base, i, GICD_IPRIORITYR_DEF_VAL);

	for (i = 0U; i < interrupt_props_num; i++) {
		prop_desc = &interrupt_props[i];

		if (prop_desc->intr_num >= MIN_SPI_ID)
			continue;

		/* Configure this interrupt as a secure interrupt */
		assert(prop_desc->intr_grp == GICV2_INTR_GROUP0);

		/*
		 * Set interrupt configuration for PPIs. Configuration for SGIs
		 * are ignored.
		 */
		if ((prop_desc->intr_num >= MIN_PPI_ID) &&
				(prop_desc->intr_num < MIN_SPI_ID)) {
			gicd_set_icfgr(gicd_base, prop_desc->intr_num,
					prop_desc->intr_cfg);
		}

		/* We have an SGI or a PPI. They are Group0 at reset */
		sec_ppi_sgi_mask |= (1u << prop_desc->intr_num);

		/* Set the priority of this interrupt */
		gicd_set_ipriorityr(gicd_base, prop_desc->intr_num,
				prop_desc->intr_pri);
	}

	/*
	 * Invert the bitmask to create a mask for non-secure PPIs and SGIs.
	 * Program the GICD_IGROUPR0 with this bit mask.
	 */
	gicd_write_igroupr(gicd_base, 0, ~sec_ppi_sgi_mask);

	/* Enable the Group 0 SGIs and PPIs */
	gicd_write_isenabler(gicd_base, 0, sec_ppi_sgi_mask);
}
