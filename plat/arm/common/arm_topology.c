/*
 * Copyright (c) 2015-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>

#include <arch.h>
#include <plat/arm/common/plat_arm.h>

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
	cluster_id = (unsigned int) ((mpidr >> MPIDR_AFF1_SHIFT) &
						MPIDR_AFFLVL_MASK);
	cpu_id = (unsigned int) ((mpidr >> MPIDR_AFF0_SHIFT) &
						MPIDR_AFFLVL_MASK);
#endif /* ARM_PLAT_MT */

	mpidr &= MPIDR_AFFINITY_MASK;
	if ((mpidr & valid_mask) != 0U)
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
