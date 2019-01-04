/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>

#include <arch.h>

#include <sq_common.h>

unsigned char sq_pd_tree_desc[PLAT_CLUSTER_COUNT + 1];

int plat_core_pos_by_mpidr(u_register_t mpidr)
{
	unsigned int cluster_id, cpu_id;

	cluster_id = (mpidr >> MPIDR_AFF1_SHIFT) & MPIDR_AFFLVL_MASK;
	if (cluster_id >= PLAT_CLUSTER_COUNT)
		return -1;

	cpu_id = (mpidr >> MPIDR_AFF0_SHIFT) & MPIDR_AFFLVL_MASK;
	if (cpu_id >= PLAT_MAX_CORES_PER_CLUSTER)
		return -1;

	return sq_calc_core_pos(mpidr);
}

const unsigned char *plat_get_power_domain_tree_desc(void)
{
	int i;

	sq_pd_tree_desc[0] = PLAT_CLUSTER_COUNT;

	for (i = 0; i < PLAT_CLUSTER_COUNT; i++)
		sq_pd_tree_desc[i + 1] = PLAT_MAX_CORES_PER_CLUSTER;

	return sq_pd_tree_desc;
}
