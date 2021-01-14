/*
 * Copyright (c) 2018, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>

#include <common/debug.h>
#include <lib/psci/psci.h>

static const unsigned char rcar_power_domain_tree_desc[] = {
	1,
	PLATFORM_CLUSTER_COUNT,
	PLATFORM_CLUSTER0_CORE_COUNT,
	PLATFORM_CLUSTER1_CORE_COUNT
};

const unsigned char *plat_get_power_domain_tree_desc(void)
{
	return rcar_power_domain_tree_desc;
}

int plat_core_pos_by_mpidr(u_register_t mpidr)
{
	unsigned int cluster_id, cpu_id;

	mpidr &= MPIDR_AFFINITY_MASK;

	if (mpidr & ~(MPIDR_CLUSTER_MASK | MPIDR_CPU_MASK))
		return -1;

	cluster_id = (mpidr >> MPIDR_AFF1_SHIFT) & MPIDR_AFFLVL_MASK;
	cpu_id = (mpidr >> MPIDR_AFF0_SHIFT) & MPIDR_AFFLVL_MASK;

	if (cluster_id >= PLATFORM_CLUSTER_COUNT)
		return -1;

	if (cluster_id == 0 && cpu_id >= PLATFORM_CLUSTER0_CORE_COUNT)
		return -1;

	if (cluster_id == 1 && cpu_id >= PLATFORM_CLUSTER1_CORE_COUNT)
		return -1;

	return (cpu_id + cluster_id * PLATFORM_CLUSTER0_CORE_COUNT);
}

