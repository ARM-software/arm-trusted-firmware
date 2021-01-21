/*
 * Copyright (c) 2015-2021, ARM Limited and Contributors. All rights reserved.
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
	while ((gicr_read_waker(gicr_base) & WAKER_CA_BIT) != 0U) {
	}
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
	while ((gicr_read_waker(gicr_base) & WAKER_CA_BIT) == 0U) {
	}
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

		if (proc_num < rdistif_num) {
			rdistif_base_addrs[proc_num] = rdistif_base;
		}

		rdistif_base += (1U << GICR_PCPUBASE_SHIFT);
	} while ((typer_val & TYPER_LAST_BIT) == 0U);
}

/*******************************************************************************
 * Helper function to configure the default attributes of (E)SPIs.
 ******************************************************************************/
void gicv3_spis_config_defaults(uintptr_t gicd_base)
{
	unsigned int i, num_ints;
#if GIC_EXT_INTID
	unsigned int num_eints;
#endif
	unsigned int typer_reg = gicd_read_typer(gicd_base);

	/* Maximum SPI INTID is 32 * (GICD_TYPER.ITLinesNumber + 1) - 1 */
	num_ints = ((typer_reg & TYPER_IT_LINES_NO_MASK) + 1U) << 5;

	/*
	 * The GICv3 architecture allows GICD_TYPER.ITLinesNumber to be 31, so
	 * the maximum possible value for num_ints is 1024. Limit the value to
	 * MAX_SPI_ID + 1 to avoid getting wrong address in GICD_OFFSET() macro.
	 */
	if (num_ints > MAX_SPI_ID + 1U) {
		num_ints = MAX_SPI_ID + 1U;
	}
	INFO("Maximum SPI INTID supported: %u\n", num_ints - 1);

	/* Treat all (E)SPIs as G1NS by default. We do 32 at a time. */
	for (i = MIN_SPI_ID; i < num_ints; i += (1U << IGROUPR_SHIFT)) {
		gicd_write_igroupr(gicd_base, i, ~0U);
	}

#if GIC_EXT_INTID
	/* Check if extended SPI range is implemented */
	if ((typer_reg & TYPER_ESPI) != 0U) {
		/*
		 * Maximum ESPI INTID is 32 * (GICD_TYPER.ESPI_range + 1) + 4095
		 */
		num_eints = ((((typer_reg >> TYPER_ESPI_RANGE_SHIFT) &
			TYPER_ESPI_RANGE_MASK) + 1U) << 5) + MIN_ESPI_ID;
		INFO("Maximum ESPI INTID supported: %u\n", num_eints - 1);

		for (i = MIN_ESPI_ID; i < num_eints;
					i += (1U << IGROUPR_SHIFT)) {
			gicd_write_igroupr(gicd_base, i, ~0U);
		}
	} else {
		num_eints = 0U;
		INFO("ESPI range is not implemented.\n");
	}
#endif

	/* Setup the default (E)SPI priorities doing four at a time */
	for (i = MIN_SPI_ID; i < num_ints; i += (1U << IPRIORITYR_SHIFT)) {
		gicd_write_ipriorityr(gicd_base, i, GICD_IPRIORITYR_DEF_VAL);
	}

#if GIC_EXT_INTID
	for (i = MIN_ESPI_ID; i < num_eints;
					i += (1U << IPRIORITYR_SHIFT)) {
		gicd_write_ipriorityr(gicd_base, i, GICD_IPRIORITYR_DEF_VAL);
	}
#endif
	/*
	 * Treat all (E)SPIs as level triggered by default, write 16 at a time
	 */
	for (i = MIN_SPI_ID; i < num_ints; i += (1U << ICFGR_SHIFT)) {
		gicd_write_icfgr(gicd_base, i, 0U);
	}

#if GIC_EXT_INTID
	for (i = MIN_ESPI_ID; i < num_eints; i += (1U << ICFGR_SHIFT)) {
		gicd_write_icfgr(gicd_base, i, 0U);
	}
#endif
}

/*******************************************************************************
 * Helper function to configure properties of secure (E)SPIs
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
	if (interrupt_props_num > 0U) {
		assert(interrupt_props != NULL);
	}

	for (i = 0U; i < interrupt_props_num; i++) {
		current_prop = &interrupt_props[i];

		unsigned int intr_num = current_prop->intr_num;

		/* Skip SGI, (E)PPI and LPI interrupts */
		if (!IS_SPI(intr_num)) {
			continue;
		}

		/* Configure this interrupt as a secure interrupt */
		gicd_clr_igroupr(gicd_base, intr_num);

		/* Configure this interrupt as G0 or a G1S interrupt */
		assert((current_prop->intr_grp == INTR_GROUP0) ||
				(current_prop->intr_grp == INTR_GROUP1S));

		if (current_prop->intr_grp == INTR_GROUP1S) {
			gicd_set_igrpmodr(gicd_base, intr_num);
			ctlr_enable |= CTLR_ENABLE_G1S_BIT;
		} else {
			gicd_clr_igrpmodr(gicd_base, intr_num);
			ctlr_enable |= CTLR_ENABLE_G0_BIT;
		}

		/* Set interrupt configuration */
		gicd_set_icfgr(gicd_base, intr_num, current_prop->intr_cfg);

		/* Set the priority of this interrupt */
		gicd_set_ipriorityr(gicd_base, intr_num,
					current_prop->intr_pri);

		/* Target (E)SPIs to the primary CPU */
		gic_affinity_val =
			gicd_irouter_val_from_mpidr(read_mpidr(), 0U);
		gicd_write_irouter(gicd_base, intr_num,
					gic_affinity_val);

		/* Enable this interrupt */
		gicd_set_isenabler(gicd_base, intr_num);
	}

	return ctlr_enable;
}

/*******************************************************************************
 * Helper function to configure the default attributes of (E)SPIs
 ******************************************************************************/
void gicv3_ppi_sgi_config_defaults(uintptr_t gicr_base)
{
	unsigned int i, ppi_regs_num, regs_num;

#if GIC_EXT_INTID
	/* Calculate number of PPI registers */
	ppi_regs_num = (unsigned int)((gicr_read_typer(gicr_base) >>
			TYPER_PPI_NUM_SHIFT) & TYPER_PPI_NUM_MASK) + 1;
	/* All other values except PPInum [0-2] are reserved */
	if (ppi_regs_num > 3U) {
		ppi_regs_num = 1U;
	}
#else
	ppi_regs_num = 1U;
#endif
	/*
	 * Disable all SGIs (imp. def.)/(E)PPIs before configuring them.
	 * This is a more scalable approach as it avoids clearing
	 * the enable bits in the GICD_CTLR.
	 */
	for (i = 0U; i < ppi_regs_num; ++i) {
		gicr_write_icenabler(gicr_base, i, ~0U);
	}

	/* Wait for pending writes to GICR_ICENABLER */
	gicr_wait_for_pending_write(gicr_base);

	/* 32 interrupt IDs per GICR_IGROUPR register */
	for (i = 0U; i < ppi_regs_num; ++i) {
		/* Treat all SGIs/(E)PPIs as G1NS by default */
		gicr_write_igroupr(gicr_base, i, ~0U);
	}

	/* 4 interrupt IDs per GICR_IPRIORITYR register */
	regs_num = ppi_regs_num << 3;
	for (i = 0U; i < regs_num; ++i) {
		/* Setup the default (E)PPI/SGI priorities doing 4 at a time */
		gicr_write_ipriorityr(gicr_base, i, GICD_IPRIORITYR_DEF_VAL);
	}

	/* 16 interrupt IDs per GICR_ICFGR register */
	regs_num = ppi_regs_num << 1;
	for (i = (MIN_PPI_ID >> ICFGR_SHIFT); i < regs_num; ++i) {
		/* Configure all (E)PPIs as level triggered by default */
		gicr_write_icfgr(gicr_base, i, 0U);
	}
}

/*******************************************************************************
 * Helper function to configure properties of secure G0 and G1S (E)PPIs and SGIs
 ******************************************************************************/
unsigned int gicv3_secure_ppi_sgi_config_props(uintptr_t gicr_base,
		const interrupt_prop_t *interrupt_props,
		unsigned int interrupt_props_num)
{
	unsigned int i;
	const interrupt_prop_t *current_prop;
	unsigned int ctlr_enable = 0U;

	/* Make sure there's a valid property array */
	if (interrupt_props_num > 0U) {
		assert(interrupt_props != NULL);
	}

	for (i = 0U; i < interrupt_props_num; i++) {
		current_prop = &interrupt_props[i];

		unsigned int intr_num = current_prop->intr_num;

		/* Skip (E)SPI interrupt */
		if (!IS_SGI_PPI(intr_num)) {
			continue;
		}

		/* Configure this interrupt as a secure interrupt */
		gicr_clr_igroupr(gicr_base, intr_num);

		/* Configure this interrupt as G0 or a G1S interrupt */
		assert((current_prop->intr_grp == INTR_GROUP0) ||
			(current_prop->intr_grp == INTR_GROUP1S));

		if (current_prop->intr_grp == INTR_GROUP1S) {
			gicr_set_igrpmodr(gicr_base, intr_num);
			ctlr_enable |= CTLR_ENABLE_G1S_BIT;
		} else {
			gicr_clr_igrpmodr(gicr_base, intr_num);
			ctlr_enable |= CTLR_ENABLE_G0_BIT;
		}

		/* Set the priority of this interrupt */
		gicr_set_ipriorityr(gicr_base, intr_num,
					current_prop->intr_pri);

		/*
		 * Set interrupt configuration for (E)PPIs.
		 * Configurations for SGIs 0-15 are ignored.
		 */
		if (intr_num >= MIN_PPI_ID) {
			gicr_set_icfgr(gicr_base, intr_num,
					current_prop->intr_cfg);
		}

		/* Enable this interrupt */
		gicr_set_isenabler(gicr_base, intr_num);
	}

	return ctlr_enable;
}

/**
 * gicv3_rdistif_get_number_frames() - determine size of GICv3 GICR region
 * @gicr_frame: base address of the GICR region to check
 *
 * This iterates over the GICR_TYPER registers of multiple GICR frames in
 * a GICR region, to find the instance which has the LAST bit set. For most
 * systems this corresponds to the number of cores handled by a redistributor,
 * but there could be disabled cores among them.
 * It assumes that each GICR region is fully accessible (till the LAST bit
 * marks the end of the region).
 * If a platform has multiple GICR regions, this function would need to be
 * called multiple times, providing the respective GICR base address each time.
 *
 * Return: number of valid GICR frames (at least 1, up to PLATFORM_CORE_COUNT)
 ******************************************************************************/
unsigned int gicv3_rdistif_get_number_frames(const uintptr_t gicr_frame)
{
	uintptr_t rdistif_base = gicr_frame;
	unsigned int count;

	for (count = 1; count < PLATFORM_CORE_COUNT; count++) {
		if ((gicr_read_typer(rdistif_base) & TYPER_LAST_BIT) != 0U) {
			break;
		}
		rdistif_base += (1U << GICR_PCPUBASE_SHIFT);
	}

	return count;
}
