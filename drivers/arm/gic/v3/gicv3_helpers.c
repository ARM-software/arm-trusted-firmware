/*
 * Copyright (c) 2015-2016, ARM Limited and Contributors. All rights reserved.
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
#include "../common/gic_common_private.h"
#include "gicv3_private.h"

/*
 * Accessor to read the GIC Distributor IGRPMODR corresponding to the
 * interrupt `id`, 32 interrupt IDs at a time.
 */
unsigned int gicd_read_igrpmodr(uintptr_t base, unsigned int id)
{
	unsigned n = id >> IGRPMODR_SHIFT;
	return mmio_read_32(base + GICD_IGRPMODR + (n << 2));
}

/*
 * Accessor to write the GIC Distributor IGRPMODR corresponding to the
 * interrupt `id`, 32 interrupt IDs at a time.
 */
void gicd_write_igrpmodr(uintptr_t base, unsigned int id, unsigned int val)
{
	unsigned n = id >> IGRPMODR_SHIFT;
	mmio_write_32(base + GICD_IGRPMODR + (n << 2), val);
}

/*
 * Accessor to get the bit corresponding to interrupt ID
 * in GIC Distributor IGRPMODR.
 */
unsigned int gicd_get_igrpmodr(uintptr_t base, unsigned int id)
{
	unsigned bit_num = id & ((1 << IGRPMODR_SHIFT) - 1);
	unsigned int reg_val = gicd_read_igrpmodr(base, id);

	return (reg_val >> bit_num) & 0x1;
}

/*
 * Accessor to set the bit corresponding to interrupt ID
 * in GIC Distributor IGRPMODR.
 */
void gicd_set_igrpmodr(uintptr_t base, unsigned int id)
{
	unsigned bit_num = id & ((1 << IGRPMODR_SHIFT) - 1);
	unsigned int reg_val = gicd_read_igrpmodr(base, id);

	gicd_write_igrpmodr(base, id, reg_val | (1 << bit_num));
}

/*
 * Accessor to clear the bit corresponding to interrupt ID
 * in GIC Distributor IGRPMODR.
 */
void gicd_clr_igrpmodr(uintptr_t base, unsigned int id)
{
	unsigned bit_num = id & ((1 << IGRPMODR_SHIFT) - 1);
	unsigned int reg_val = gicd_read_igrpmodr(base, id);

	gicd_write_igrpmodr(base, id, reg_val & ~(1 << bit_num));
}

/*
 * Accessor to read the GIC Re-distributor IPRIORITYR corresponding to the
 * interrupt `id`, 4 interrupts IDs at a time.
 */
unsigned int gicr_read_ipriorityr(uintptr_t base, unsigned int id)
{
	unsigned n = id >> IPRIORITYR_SHIFT;
	return mmio_read_32(base + GICR_IPRIORITYR + (n << 2));
}

/*
 * Accessor to write the GIC Re-distributor IPRIORITYR corresponding to the
 * interrupt `id`, 4 interrupts IDs at a time.
 */
void gicr_write_ipriorityr(uintptr_t base, unsigned int id, unsigned int val)
{
	unsigned n = id >> IPRIORITYR_SHIFT;
	mmio_write_32(base + GICR_IPRIORITYR + (n << 2), val);
}

/*
 * Accessor to get the bit corresponding to interrupt ID
 * from GIC Re-distributor IGROUPR0.
 */
unsigned int gicr_get_igroupr0(uintptr_t base, unsigned int id)
{
	unsigned bit_num = id & ((1 << IGROUPR_SHIFT) - 1);
	unsigned int reg_val = gicr_read_igroupr0(base);

	return (reg_val >> bit_num) & 0x1;
}

/*
 * Accessor to set the bit corresponding to interrupt ID
 * in GIC Re-distributor IGROUPR0.
 */
void gicr_set_igroupr0(uintptr_t base, unsigned int id)
{
	unsigned bit_num = id & ((1 << IGROUPR_SHIFT) - 1);
	unsigned int reg_val = gicr_read_igroupr0(base);

	gicr_write_igroupr0(base, reg_val | (1 << bit_num));
}

/*
 * Accessor to clear the bit corresponding to interrupt ID
 * in GIC Re-distributor IGROUPR0.
 */
void gicr_clr_igroupr0(uintptr_t base, unsigned int id)
{
	unsigned bit_num = id & ((1 << IGROUPR_SHIFT) - 1);
	unsigned int reg_val = gicr_read_igroupr0(base);

	gicr_write_igroupr0(base, reg_val & ~(1 << bit_num));
}

/*
 * Accessor to get the bit corresponding to interrupt ID
 * from GIC Re-distributor IGRPMODR0.
 */
unsigned int gicr_get_igrpmodr0(uintptr_t base, unsigned int id)
{
	unsigned bit_num = id & ((1 << IGRPMODR_SHIFT) - 1);
	unsigned int reg_val = gicr_read_igrpmodr0(base);

	return (reg_val >> bit_num) & 0x1;
}

/*
 * Accessor to set the bit corresponding to interrupt ID
 * in GIC Re-distributor IGRPMODR0.
 */
void gicr_set_igrpmodr0(uintptr_t base, unsigned int id)
{
	unsigned bit_num = id & ((1 << IGRPMODR_SHIFT) - 1);
	unsigned int reg_val = gicr_read_igrpmodr0(base);

	gicr_write_igrpmodr0(base, reg_val | (1 << bit_num));
}

/*
 * Accessor to clear the bit corresponding to interrupt ID
 * in GIC Re-distributor IGRPMODR0.
 */
void gicr_clr_igrpmodr0(uintptr_t base, unsigned int id)
{
	unsigned bit_num = id & ((1 << IGRPMODR_SHIFT) - 1);
	unsigned int reg_val = gicr_read_igrpmodr0(base);

	gicr_write_igrpmodr0(base, reg_val & ~(1 << bit_num));
}

/*
 * Accessor to set the bit corresponding to interrupt ID
 * in GIC Re-distributor ISENABLER0.
 */
void gicr_set_isenabler0(uintptr_t base, unsigned int id)
{
	unsigned bit_num = id & ((1 << ISENABLER_SHIFT) - 1);

	gicr_write_isenabler0(base, (1 << bit_num));
}

/*
 * Accessor to set the byte corresponding to interrupt ID
 * in GIC Re-distributor IPRIORITYR.
 */
void gicr_set_ipriorityr(uintptr_t base, unsigned int id, unsigned int pri)
{
	mmio_write_8(base + GICR_IPRIORITYR + id, pri & GIC_PRI_MASK);
}

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
	assert(gicr_read_waker(gicr_base) & WAKER_CA_BIT);

	/* Mark the connected core as awake */
	gicr_write_waker(gicr_base, gicr_read_waker(gicr_base) & ~WAKER_PS_BIT);

	/* Wait till the WAKER_CA_BIT changes to 0 */
	while (gicr_read_waker(gicr_base) & WAKER_CA_BIT)
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
	while (!(gicr_read_waker(gicr_base) & WAKER_CA_BIT))
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
	unsigned long long typer_val;
	uintptr_t rdistif_base = gicr_base;

	assert(rdistif_base_addrs);

	/*
	 * Iterate over the Redistributor frames. Store the base address of each
	 * frame in the platform provided array. Use the "Processor Number"
	 * field to index into the array if the platform has not provided a hash
	 * function to convert an MPIDR (obtained from the "Affinity Value"
	 * field into a linear index.
	 */
	do {
		typer_val = gicr_read_typer(rdistif_base);
		if (mpidr_to_core_pos) {
			mpidr = mpidr_from_gicr_typer(typer_val);
			proc_num = mpidr_to_core_pos(mpidr);
		} else {
			proc_num = (typer_val >> TYPER_PROC_NUM_SHIFT) &
				TYPER_PROC_NUM_MASK;
		}
		assert(proc_num < rdistif_num);
		rdistif_base_addrs[proc_num] = rdistif_base;
		rdistif_base += (1 << GICR_PCPUBASE_SHIFT);
	} while (!(typer_val & TYPER_LAST_BIT));
}

/*******************************************************************************
 * Helper function to configure the default attributes of SPIs.
 ******************************************************************************/
void gicv3_spis_configure_defaults(uintptr_t gicd_base)
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

	/*
	 * Treat all SPIs as level triggered by default, write 16 at
	 * a time
	 */
	for (index = MIN_SPI_ID; index < num_ints; index += 16)
		gicd_write_icfgr(gicd_base, index, 0);
}

/*******************************************************************************
 * Helper function to configure secure G0 and G1S SPIs.
 ******************************************************************************/
void gicv3_secure_spis_configure(uintptr_t gicd_base,
				     unsigned int num_ints,
				     const unsigned int *sec_intr_list,
				     unsigned int int_grp)
{
	unsigned int index, irq_num;
	unsigned long long gic_affinity_val;

	assert((int_grp == INTR_GROUP1S) || (int_grp == INTR_GROUP0));
	/* If `num_ints` is not 0, ensure that `sec_intr_list` is not NULL */
	assert(num_ints ? (uintptr_t)sec_intr_list : 1);

	for (index = 0; index < num_ints; index++) {
		irq_num = sec_intr_list[index];
		if (irq_num >= MIN_SPI_ID) {

			/* Configure this interrupt as a secure interrupt */
			gicd_clr_igroupr(gicd_base, irq_num);

			/* Configure this interrupt as G0 or a G1S interrupt */
			if (int_grp == INTR_GROUP1S)
				gicd_set_igrpmodr(gicd_base, irq_num);
			else
				gicd_clr_igrpmodr(gicd_base, irq_num);

			/* Set the priority of this interrupt */
			gicd_set_ipriorityr(gicd_base,
					      irq_num,
					      GIC_HIGHEST_SEC_PRIORITY);

			/* Target SPIs to the primary CPU */
			gic_affinity_val =
				gicd_irouter_val_from_mpidr(read_mpidr(), 0);
			gicd_write_irouter(gicd_base,
					   irq_num,
					   gic_affinity_val);

			/* Enable this interrupt */
			gicd_set_isenabler(gicd_base, irq_num);
		}
	}

}

/*******************************************************************************
 * Helper function to configure the default attributes of SPIs.
 ******************************************************************************/
void gicv3_ppi_sgi_configure_defaults(uintptr_t gicr_base)
{
	unsigned int index;

	/*
	 * Disable all SGIs (imp. def.)/PPIs before configuring them. This is a
	 * more scalable approach as it avoids clearing the enable bits in the
	 * GICD_CTLR
	 */
	gicr_write_icenabler0(gicr_base, ~0);
	gicr_wait_for_pending_write(gicr_base);

	/* Treat all SGIs/PPIs as G1NS by default. */
	gicr_write_igroupr0(gicr_base, ~0U);

	/* Setup the default PPI/SGI priorities doing four at a time */
	for (index = 0; index < MIN_SPI_ID; index += 4)
		gicr_write_ipriorityr(gicr_base,
				      index,
				      GICD_IPRIORITYR_DEF_VAL);

	/* Configure all PPIs as level triggered by default */
	gicr_write_icfgr1(gicr_base, 0);
}

/*******************************************************************************
 * Helper function to configure secure G0 and G1S SPIs.
 ******************************************************************************/
void gicv3_secure_ppi_sgi_configure(uintptr_t gicr_base,
					unsigned int num_ints,
					const unsigned int *sec_intr_list,
					unsigned int int_grp)
{
	unsigned int index, irq_num;

	assert((int_grp == INTR_GROUP1S) || (int_grp == INTR_GROUP0));
	/* If `num_ints` is not 0, ensure that `sec_intr_list` is not NULL */
	assert(num_ints ? (uintptr_t)sec_intr_list : 1);

	for (index = 0; index < num_ints; index++) {
		irq_num = sec_intr_list[index];
		if (irq_num < MIN_SPI_ID) {

			/* Configure this interrupt as a secure interrupt */
			gicr_clr_igroupr0(gicr_base, irq_num);

			/* Configure this interrupt as G0 or a G1S interrupt */
			if (int_grp == INTR_GROUP1S)
				gicr_set_igrpmodr0(gicr_base, irq_num);
			else
				gicr_clr_igrpmodr0(gicr_base, irq_num);

			/* Set the priority of this interrupt */
			gicr_set_ipriorityr(gicr_base,
					    irq_num,
					    GIC_HIGHEST_SEC_PRIORITY);

			/* Enable this interrupt */
			gicr_set_isenabler0(gicr_base, irq_num);
		}
	}
}
