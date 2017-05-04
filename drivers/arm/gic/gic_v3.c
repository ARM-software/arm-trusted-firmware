/*
 * Copyright (c) 2014, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <debug.h>
#include <gic_v3.h>

uintptr_t gicv3_get_rdist(uintptr_t gicr_base, uint64_t mpidr)
{
	uint32_t  cpu_aff, gicr_aff;
	uint64_t  gicr_typer;
	uintptr_t addr;

	/* Construct the affinity as used by GICv3. MPIDR and GIC affinity level
	 * mask is the same.
	 */
	cpu_aff  = ((mpidr >> MPIDR_AFF0_SHIFT) & MPIDR_AFFLVL_MASK) <<
			GICV3_AFF0_SHIFT;
	cpu_aff |= ((mpidr >> MPIDR_AFF1_SHIFT) & MPIDR_AFFLVL_MASK) <<
			GICV3_AFF1_SHIFT;
	cpu_aff |= ((mpidr >> MPIDR_AFF2_SHIFT) & MPIDR_AFFLVL_MASK) <<
			GICV3_AFF2_SHIFT;
	cpu_aff |= ((mpidr >> MPIDR_AFF3_SHIFT) & MPIDR_AFFLVL_MASK) <<
			GICV3_AFF3_SHIFT;

	addr = gicr_base;
	do {
		gicr_typer = gicr_read_typer(addr);

		gicr_aff = (gicr_typer >> GICR_TYPER_AFF_SHIFT) &
				GICR_TYPER_AFF_MASK;
		if (cpu_aff == gicr_aff) {
			/* Disable this print for now as it appears every time
			 * when using PSCI CPU_SUSPEND.
			 * TODO: Print this only the first time for each CPU.
			 * INFO("GICv3 - Found RDIST for MPIDR(0x%lx) at %p\n",
			 *	mpidr, (void *) addr);
			 */
			return addr;
		}

		/* TODO:
		 * For GICv4 we need to adjust the Base address based on
		 * GICR_TYPER.VLPIS
		 */
		addr += (1 << GICR_PCPUBASE_SHIFT);

	} while (!(gicr_typer & GICR_TYPER_LAST));

	/* If we get here we did not find a match. */
	ERROR("GICv3 - Did not find RDIST for CPU with MPIDR 0x%lx\n", mpidr);
	return (uintptr_t)NULL;
}
