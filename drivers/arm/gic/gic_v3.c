/*
 * Copyright (c) 2014, ARM Limited and Contributors. All rights reserved.
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
