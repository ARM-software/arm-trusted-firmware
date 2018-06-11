/*
 * Copyright (c) 2016-2017, Cavium Inc. All rights reserved.<BR>
 * Copyright (c) 2018     , Facebook, Inc.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <platform_def.h>
#include <psci.h>

static const unsigned char thunder_power_domain_tree_desc[] = {
	/* No of root nodes */
	PLATFORM_MAX_NODES,
	/* No of clusters */
	PLATFORM_MAX_CLUSTERS_PER_NODE,
	PLATFORM_MAX_CLUSTERS_PER_NODE,
	/* No of CPU cores */
	PLATFORM_MAX_CPUS_PER_CLUSTER,
	PLATFORM_MAX_CPUS_PER_CLUSTER,
	PLATFORM_MAX_CPUS_PER_CLUSTER,
	PLATFORM_MAX_CPUS_PER_CLUSTER,
	PLATFORM_MAX_CPUS_PER_CLUSTER,
	PLATFORM_MAX_CPUS_PER_CLUSTER,
};

/*******************************************************************************
 * This function returns the Thunder default topology tree information.
 ******************************************************************************/
const unsigned char *plat_get_power_domain_tree_desc(void)
{
	return thunder_power_domain_tree_desc;
}

/*******************************************************************************
 * This function implements a part of the critical interface between the psci
 * generic layer and the platform that allows the former to query the platform
 * to convert an MPIDR to a unique linear index. An error code (-1) is returned
 * in case the MPIDR is invalid.
 ******************************************************************************/
int plat_core_pos_by_mpidr(u_register_t mpidr)
{
	unsigned int cluster_id, cpu_id, node_id;

	mpidr &= MPIDR_AFFINITY_MASK;

	if (mpidr & ~(MPIDR_CLUSTER_MASK | MPIDR_CPU_MASK | MPIDR_NODE_MASK))
		return -1;

	node_id = (mpidr >> MPIDR_AFF2_SHIFT) & MPIDR_AFFLVL_MASK;
	cluster_id = (mpidr >> MPIDR_AFF1_SHIFT) & MPIDR_AFFLVL_MASK;
	cpu_id = (mpidr >> MPIDR_AFF0_SHIFT) & MPIDR_AFFLVL_MASK;

	if (node_id >= PLATFORM_MAX_NODES)
		return -1;

	if (cluster_id >= PLATFORM_MAX_CLUSTERS_PER_NODE)
		return -1;

	if (cpu_id >= PLATFORM_MAX_CPUS_PER_CLUSTER)
		return -1;

	return (cpu_id + (cluster_id << 4) + (node_id * PLATFORM_MAX_CLUSTERS_PER_NODE * PLATFORM_MAX_CPUS_PER_CLUSTER));
}
