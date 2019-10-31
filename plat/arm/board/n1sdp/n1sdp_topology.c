/*
 * Copyright (c) 2018-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <plat/arm/common/plat_arm.h>

/* Topology */
typedef struct n1sdp_topology {
	const unsigned char *power_tree;
	unsigned int plat_cluster_core_count;
} n1sdp_topology_t;

/*
 * The power domain tree descriptor. The cluster power domains are
 * arranged so that when the PSCI generic code creates the power domain tree,
 * the indices of the CPU power domain nodes it allocates match the linear
 * indices returned by plat_core_pos_by_mpidr().
 */
const unsigned char n1sdp_pd_tree_desc[] = {
	PLAT_N1SDP_CHIP_COUNT,
	PLAT_ARM_CLUSTER_COUNT,
	PLAT_ARM_CLUSTER_COUNT,
	N1SDP_MAX_CPUS_PER_CLUSTER,
	N1SDP_MAX_CPUS_PER_CLUSTER,
	N1SDP_MAX_CPUS_PER_CLUSTER,
	N1SDP_MAX_CPUS_PER_CLUSTER
};

/* Topology configuration for n1sdp */
const n1sdp_topology_t n1sdp_topology = {
	.power_tree = n1sdp_pd_tree_desc,
	.plat_cluster_core_count = N1SDP_MAX_CPUS_PER_CLUSTER
};

/*******************************************************************************
 * This function returns the topology tree information.
 ******************************************************************************/
const unsigned char *plat_get_power_domain_tree_desc(void)
{
	return n1sdp_topology.power_tree;
}

/*******************************************************************************
 * This function returns the core count within the cluster corresponding to
 * `mpidr`.
 ******************************************************************************/
unsigned int plat_arm_get_cluster_core_count(u_register_t mpidr)
{
	return n1sdp_topology.plat_cluster_core_count;
}

/*******************************************************************************
 * The array mapping platform core position (implemented by plat_my_core_pos())
 * to the SCMI power domain ID implemented by SCP.
 ******************************************************************************/
const uint32_t plat_css_core_pos_to_scmi_dmn_id_map[PLATFORM_CORE_COUNT] = {
	0, 1, 2, 3, 4, 5, 6, 7};
