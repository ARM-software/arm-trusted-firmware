/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <plat_arm.h>
#include <sgi_plat_config.h>

/* Topology */
/*
 * The power domain tree descriptor. The cluster power domains are
 * arranged so that when the PSCI generic code creates the power domain tree,
 * the indices of the CPU power domain nodes it allocates match the linear
 * indices returned by plat_core_pos_by_mpidr().
 */
const unsigned char sgi_pd_tree_desc[] = {
	PLAT_ARM_CLUSTER_COUNT,
	CSS_SGI_MAX_CPUS_PER_CLUSTER,
	CSS_SGI_MAX_CPUS_PER_CLUSTER
};

/* Topology configuration for sgi platform */
const css_topology_t sgi_topology = {
	.power_tree = sgi_pd_tree_desc,
	.plat_cluster_core_count = CSS_SGI_MAX_CPUS_PER_CLUSTER
};

/*******************************************************************************
 * This function returns the topology tree information.
 ******************************************************************************/
const unsigned char *plat_get_power_domain_tree_desc(void)
{
	return sgi_topology.power_tree;
}

/*******************************************************************************
 * This function returns the core count within the cluster corresponding to
 * `mpidr`.
 ******************************************************************************/
unsigned int plat_arm_get_cluster_core_count(u_register_t mpidr)
{
	return sgi_topology.plat_cluster_core_count;
}

/*******************************************************************************
 * The array mapping platform core position (implemented by plat_my_core_pos())
 * to the SCMI power domain ID implemented by SCP.
 ******************************************************************************/
const uint32_t plat_css_core_pos_to_scmi_dmn_id_map[32] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,		\
	16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31
};
