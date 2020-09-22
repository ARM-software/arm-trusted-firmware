/*
 * Copyright (c) 2020, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/cassert.h>
#include <plat/arm/common/plat_arm.h>

/* Compile time assertion to ensure the core count is 4 */
CASSERT(PLATFORM_CORE_COUNT == 4U, assert_invalid_platform_core_count);

/* Topology */
typedef struct morello_topology {
	const unsigned char *power_tree;
	unsigned int plat_cluster_core_count;
} morello_topology_t;

/*
 * The power domain tree descriptor. The cluster power domains are
 * arranged so that when the PSCI generic code creates the power domain tree,
 * the indices of the CPU power domain nodes it allocates match the linear
 * indices returned by plat_core_pos_by_mpidr().
 */
const unsigned char morello_pd_tree_desc[] = {
	PLAT_MORELLO_CHIP_COUNT,
	PLAT_ARM_CLUSTER_COUNT,
	MORELLO_MAX_CPUS_PER_CLUSTER,
	MORELLO_MAX_CPUS_PER_CLUSTER,
};

/* Topology configuration for morello */
const morello_topology_t morello_topology = {
	.power_tree = morello_pd_tree_desc,
	.plat_cluster_core_count = MORELLO_MAX_CPUS_PER_CLUSTER
};

/*******************************************************************************
 * This function returns the topology tree information.
 ******************************************************************************/
const unsigned char *plat_get_power_domain_tree_desc(void)
{
	return morello_topology.power_tree;
}

/*******************************************************************************
 * This function returns the core count within the cluster corresponding to
 * `mpidr`.
 ******************************************************************************/
unsigned int plat_arm_get_cluster_core_count(u_register_t mpidr)
{
	return morello_topology.plat_cluster_core_count;
}

/*******************************************************************************
 * The array mapping platform core position (implemented by plat_my_core_pos())
 * to the SCMI power domain ID implemented by SCP.
 ******************************************************************************/
const uint32_t plat_css_core_pos_to_scmi_dmn_id_map[PLATFORM_CORE_COUNT] = {
	0, 1, 2, 3};
