/*
 * Copyright (c) 2019, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>

/* The A5DS power domain tree descriptor */
static const unsigned char a5ds_power_domain_tree_desc[] = {
	1,
	/* No of children for the root node */
	A5DS_CLUSTER_COUNT,
	/* No of children for the first cluster node */
	A5DS_CORE_COUNT,
};

/*******************************************************************************
 * This function returns the topology according to A5DS_CLUSTER_COUNT.
 ******************************************************************************/
const unsigned char *plat_get_power_domain_tree_desc(void)
{
	return a5ds_power_domain_tree_desc;
}

/*******************************************************************************
 * Get core position using mpidr
 ******************************************************************************/
int plat_core_pos_by_mpidr(u_register_t mpidr)
{
	unsigned int cluster_id, cpu_id;

	mpidr &= MPIDR_AFFINITY_MASK;

	if (mpidr & ~(MPIDR_CLUSTER_MASK | MPIDR_CPU_MASK))
		return -1;

	cluster_id = (mpidr >> MPIDR_AFF1_SHIFT) & MPIDR_AFFLVL_MASK;
	cpu_id = (mpidr >> MPIDR_AFF0_SHIFT) & MPIDR_AFFLVL_MASK;

	if (cluster_id >= A5DS_CLUSTER_COUNT)
		return -1;

	/*
	 * Validate cpu_id by checking whether it represents a CPU in
	 * one of the two clusters present on the platform.
	 */
	if (cpu_id >= A5DS_MAX_CPUS_PER_CLUSTER)
		return -1;

	return (cpu_id + (cluster_id * 4));

}
