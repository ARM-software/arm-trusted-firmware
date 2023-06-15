/*
 * Copyright (c) 2023, Aspeed Technology Inc.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <lib/psci/psci.h>
#include <platform_def.h>

static const unsigned char ast2700_power_domain_tree_desc[] = {
	PLATFORM_SYSTEM_COUNT,
	PLATFORM_CORE_COUNT_PER_CLUSTER,
};

const unsigned char *plat_get_power_domain_tree_desc(void)
{
	return ast2700_power_domain_tree_desc;
}

unsigned int plat_core_pos_by_mpidr(u_register_t mpidr)
{
	unsigned int cluster_id, cpu_id;

	mpidr &= MPIDR_AFFINITY_MASK;

	if (mpidr & ~(MPIDR_CLUSTER_MASK | MPIDR_CPU_MASK)) {
		return -1;
	}

	cluster_id = (mpidr >> MPIDR_AFF1_SHIFT) & MPIDR_AFFLVL_MASK;
	cpu_id = (mpidr >> MPIDR_AFF0_SHIFT) & MPIDR_AFFLVL_MASK;

	if (cluster_id >= PLATFORM_CLUSTER_COUNT) {
		return -1;
	}

	if (cpu_id >= PLATFORM_CORE_COUNT_PER_CLUSTER) {
		return -1;
	}

	return (cluster_id * PLATFORM_CORE_COUNT_PER_CLUSTER) + cpu_id;
}
