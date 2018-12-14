/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_helpers.h>
#include <plat/common/platform.h>

const unsigned char imx_power_domain_tree_desc[] = {
	PWR_DOMAIN_AT_MAX_LVL,
	PLATFORM_CLUSTER_COUNT,
	PLATFORM_CLUSTER0_CORE_COUNT,
	PLATFORM_CLUSTER1_CORE_COUNT,
};

const unsigned char *plat_get_power_domain_tree_desc(void)
{
	return imx_power_domain_tree_desc;
}

int plat_core_pos_by_mpidr(u_register_t mpidr)
{
	unsigned int cluster_id, cpu_id;

	mpidr &= MPIDR_AFFINITY_MASK;

	if (mpidr & ~(MPIDR_CLUSTER_MASK | MPIDR_CPU_MASK))
		return -1;

	cluster_id = MPIDR_AFFLVL1_VAL(mpidr);
	cpu_id = MPIDR_AFFLVL0_VAL(mpidr);

	if (cluster_id > PLATFORM_CLUSTER_COUNT ||
		cpu_id > PLATFORM_MAX_CPU_PER_CLUSTER)
		return -1;

	return (cpu_id + (cluster_id * 4));
}
