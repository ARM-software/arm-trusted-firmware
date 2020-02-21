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

#include "../common/gic_common_private.h"
#include "gicv3_private.h"

/******************************************************************************
 * This function marks the core as awake in the re-distributor and
 * ensures that the interface is active.
 *****************************************************************************/
void gicv3_rdistif_mark_core_awake(uintptr_t gicr_base)
{
	/*
	 * The WAKER_PS_BIT should be changed to 0
	 * only when WAKER_CA_BIT is 1.
	 */
	assert((gicr_read_waker(gicr_base) & WAKER_CA_BIT) != 0U);

	/* Mark the connected core as awake */
	gicr_write_waker(gicr_base, gicr_read_waker(gicr_base) & ~WAKER_PS_BIT);

	/* Wait till the WAKER_CA_BIT changes to 0 */
	while ((gicr_read_waker(gicr_base) & WAKER_CA_BIT) != 0U)
		;
}

/******************************************************************************
 * This function marks the core as asleep in the re-distributor and ensures
 * that the interface is quiescent.
 *****************************************************************************/
void gicv3_rdistif_mark_core_asleep(uintptr_t gicr_base)
{
	/* Mark the connected core as asleep */
	gicr_write_waker(gicr_base, gicr_read_waker(gicr_base) | WAKER_PS_BIT);

	/* Wait till the WAKER_CA_BIT changes to 1 */
	while ((gicr_read_waker(gicr_base) & WAKER_CA_BIT) == 0U)
		;
}


/*******************************************************************************
 * This function probes the Redistributor frames when the driver is initialised
 * and saves their base addresses. These base addresses are used later to
 * initialise each Redistributor interface.
 ******************************************************************************/
void gicv3_rdistif_base_addrs_probe(uintptr_t *rdistif_base_addrs,
					unsigned int rdistif_num,
					uintptr_t gicr_base,
					mpidr_hash_fn mpidr_to_core_pos)
{
	u_register_t mpidr;
	unsigned int proc_num;
	uint64_t typer_val;
	uintptr_t rdistif_base = gicr_base;

	assert(rdistif_base_addrs != NULL);

	/*
	 * Iterate over the Redistributor frames. Store the base address of each
	 * frame in the platform provided array. Use the "Processor Number"
	 * field to index into the array if the platform has not provided a hash
	 * function to convert an MPIDR (obtained from the "Affinity Value"
	 * field into a linear index.
	 */
	do {
		typer_val = gicr_read_typer(rdistif_base);
		if (mpidr_to_core_pos != NULL) {
			mpidr = mpidr_from_gicr_typer(typer_val);
			proc_num = mpidr_to_core_pos(mpidr);
		} else {
			proc_num = (typer_val >> TYPER_PROC_NUM_SHIFT) &
				TYPER_PROC_NUM_MASK;
		}

		if (proc_num < rdistif_num)
			rdistif_base_addrs[proc_num] = rdistif_base;

		rdistif_base += (1U << GICR_PCPUBASE_SHIFT);
	} while ((typer_val & TYPER_LAST_BIT) == 0U);
}

/*******************************************************************************
 * Helper function to configure the default attributes of SPIs.
 ******************************************************************************/
void gicv3_spis_config_defaults(uintptr_t gicd_base)
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

	/*
	 * Treat all SPIs as level triggered by default, write 16 at
	 * a time
	 */
	for (index = MIN_SPI_ID; index < num_ints; index += 16U)
		gicd_write_icfgr(gicd_base, index, 0U);
}

/*******************************************************************************
 * Helper function to configure properties of secure SPIs
 ******************************************************************************/
unsigned int gicv3_secure_spis_config_props(uintptr_t gicd_base,
		const interrupt_prop_t *interrupt_props,
		unsigned int interrupt_props_num)
{
	unsigned int i;
	const interrupt_prop_t *current_prop;
	unsigned long long gic_affinity_val;
	unsigned int ctlr_enable = 0U;

	/* Make sure there's a valid property array */
	if (interrupt_props_num > 0U)
		assert(interrupt_props != NULL);

	for (i = 0U; i < interrupt_props_num; i++) {
		current_prop = &interrupt_props[i];

		if (current_prop->intr_num < MIN_SPI_ID)
			continue;

		/* Configure this interrupt as a secure interrupt */
		gicd_clr_igroupr(gicd_base, current_prop->intr_num);

		/* Configure this interrupt as G0 or a G1S interrupt */
		assert((current_prop->intr_grp == INTR_GROUP0) ||
				(current_prop->intr_grp == INTR_GROUP1S));
		if (current_prop->intr_grp == INTR_GROUP1S) {
			gicd_set_igrpmodr(gicd_base, current_prop->intr_num);
			ctlr_enable |= CTLR_ENABLE_G1S_BIT;
		} else {
			gicd_clr_igrpmodr(gicd_base, current_prop->intr_num);
			ctlr_enable |= CTLR_ENABLE_G0_BIT;
		}

		/* Set interrupt configuration */
		gicd_set_icfgr(gicd_base, current_prop->intr_num,
				current_prop->intr_cfg);

		/* Set the priority of this interrupt */
		gicd_set_ipriorityr(gicd_base, current_prop->intr_num,
				current_prop->intr_pri);

		/* Target SPIs to the primary CPU */
		gic_affinity_val =
			gicd_irouter_val_from_mpidr(read_mpidr(), 0U);
		gicd_write_irouter(gicd_base, current_prop->intr_num,
				gic_affinity_val);

		/* Enable this interrupt */
		gicd_set_isenabler(gicd_base, current_prop->intr_num);
	}

	return ctlr_enable;
}

/*******************************************************************************
 * Helper function to configure the default attributes of SPIs.
 ******************************************************************************/
void gicv3_ppi_sgi_config_defaults(uintptr_t gicr_base)
{
	unsigned int index;

	/*
	 * Disable all SGIs (imp. def.)/PPIs before configuring them. This is a
	 * more scalable approach as it avoids clearing the enable bits in the
	 * GICD_CTLR
	 */
	gicr_write_icenabler0(gicr_base, ~0U);
	gicr_wait_for_pending_write(gicr_base);

	/* Treat all SGIs/PPIs as G1NS by default. */
	gicr_write_igroupr0(gicr_base, ~0U);

	/* Setup the default PPI/SGI priorities doing four at a time */
	for (index = 0U; index < MIN_SPI_ID; index += 4U)
		gicr_write_ipriorityr(gicr_base,
				      index,
				      GICD_IPRIORITYR_DEF_VAL);

	/* Configure all PPIs as level triggered by default */
	gicr_write_icfgr1(gicr_base, 0U);
}

/*******************************************************************************
 * Helper function to configure properties of secure G0 and G1S PPIs and SGIs.
 ******************************************************************************/
unsigned int gicv3_secure_ppi_sgi_config_props(uintptr_t gicr_base,
		const interrupt_prop_t *interrupt_props,
		unsigned int interrupt_props_num)
{
	unsigned int i;
	const interrupt_prop_t *current_prop;
	unsigned int ctlr_enable = 0U;

	/* Make sure there's a valid property array */
	if (interrupt_props_num > 0U)
		assert(interrupt_props != NULL);

	for (i = 0U; i < interrupt_props_num; i++) {
		current_prop = &interrupt_props[i];

		if (current_prop->intr_num >= MIN_SPI_ID)
			continue;

		/* Configure this interrupt as a secure interrupt */
		gicr_clr_igroupr0(gicr_base, current_prop->intr_num);

		/* Configure this interrupt as G0 or a G1S interrupt */
		assert((current_prop->intr_grp == INTR_GROUP0) ||
				(current_prop->intr_grp == INTR_GROUP1S));
		if (current_prop->intr_grp == INTR_GROUP1S) {
			gicr_set_igrpmodr0(gicr_base, current_prop->intr_num);
			ctlr_enable |= CTLR_ENABLE_G1S_BIT;
		} else {
			gicr_clr_igrpmodr0(gicr_base, current_prop->intr_num);
			ctlr_enable |= CTLR_ENABLE_G0_BIT;
		}

		/* Set the priority of this interrupt */
		gicr_set_ipriorityr(gicr_base, current_prop->intr_num,
				current_prop->intr_pri);

		/*
		 * Set interrupt configuration for PPIs. Configuration for SGIs
		 * are ignored.
		 */
		if ((current_prop->intr_num >= MIN_PPI_ID) &&
				(current_prop->intr_num < MIN_SPI_ID)) {
			gicr_set_icfgr1(gicr_base, current_prop->intr_num,
					current_prop->intr_cfg);
		}

		/* Enable this interrupt */
		gicr_set_isenabler0(gicr_base, current_prop->intr_num);
	}

	return ctlr_enable;
}
