/*
 * Copyright (c) 2015-2017, ARM Limited and Contributors. All rights reserved.
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
#include <plat_arm.h>
#include <platform_def.h>

/*******************************************************************************
 * This function validates an MPIDR by checking whether it falls within the
 * acceptable bounds. An error code (-1) is returned if an incorrect mpidr
 * is passed.
 ******************************************************************************/
int arm_check_mpidr(u_register_t mpidr)
{
	unsigned int cluster_id, cpu_id;
	uint64_t valid_mask;

#if ARM_PLAT_MT
	unsigned int pe_id;

	valid_mask = ~(MPIDR_AFFLVL_MASK |
			(MPIDR_AFFLVL_MASK << MPIDR_AFF1_SHIFT) |
			(MPIDR_AFFLVL_MASK << MPIDR_AFF2_SHIFT));
	cluster_id = (mpidr >> MPIDR_AFF2_SHIFT) & MPIDR_AFFLVL_MASK;
	cpu_id = (mpidr >> MPIDR_AFF1_SHIFT) & MPIDR_AFFLVL_MASK;
	pe_id = (mpidr >> MPIDR_AFF0_SHIFT) & MPIDR_AFFLVL_MASK;
#else
	valid_mask = ~(MPIDR_CLUSTER_MASK | MPIDR_CPU_MASK);
	cluster_id = (mpidr >> MPIDR_AFF1_SHIFT) & MPIDR_AFFLVL_MASK;
	cpu_id = (mpidr >> MPIDR_AFF0_SHIFT) & MPIDR_AFFLVL_MASK;
#endif /* ARM_PLAT_MT */

	mpidr &= MPIDR_AFFINITY_MASK;
	if (mpidr & valid_mask)
		return -1;

	if (cluster_id >= PLAT_ARM_CLUSTER_COUNT)
		return -1;

	/* Validate cpu_id by checking whether it represents a CPU in
	   one of the two clusters present on the platform. */
	if (cpu_id >= plat_arm_get_cluster_core_count(mpidr))
		return -1;

#if ARM_PLAT_MT
	if (pe_id >= plat_arm_get_cpu_pe_count(mpidr))
		return -1;
#endif /* ARM_PLAT_MT */

	return 0;
}
