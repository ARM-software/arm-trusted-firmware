/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>
#include <psci.h>

/* The power domain tree descriptor */
static unsigned char power_domain_tree_desc[] = {
	PLATFORM_SYSTEM_COUNT,
	PLATFORM_CLUSTER_COUNT,
	K3_CLUSTER0_CORE_COUNT,
#if K3_CLUSTER1_MSMC_PORT != UNUSED
	K3_CLUSTER1_CORE_COUNT,
#endif /* K3_CLUSTER1_MSMC_PORT != UNUSED */
#if K3_CLUSTER2_MSMC_PORT != UNUSED
	K3_CLUSTER2_CORE_COUNT,
#endif /* K3_CLUSTER2_MSMC_PORT != UNUSED */
#if K3_CLUSTER3_MSMC_PORT != UNUSED
	K3_CLUSTER3_CORE_COUNT,
#endif /* K3_CLUSTER3_MSMC_PORT != UNUSED */
};

const unsigned char *plat_get_power_domain_tree_desc(void)
{
	return power_domain_tree_desc;
}

int plat_core_pos_by_mpidr(u_register_t mpidr)
{
	unsigned int cpu_id;

	mpidr &= MPIDR_AFFINITY_MASK;

	if (mpidr & ~(MPIDR_CLUSTER_MASK | MPIDR_CPU_MASK))
		return -1;

	cpu_id = MPIDR_AFFLVL0_VAL(mpidr);

	switch (MPIDR_AFFLVL1_VAL(mpidr)) {
	case K3_CLUSTER0_MSMC_PORT:
		if (cpu_id < K3_CLUSTER0_CORE_COUNT)
			return cpu_id;
		return -1;
#if K3_CLUSTER1_MSMC_PORT != UNUSED
	case K3_CLUSTER1_MSMC_PORT:
		if (cpu_id < K3_CLUSTER1_CORE_COUNT)
			return K3_CLUSTER0_CORE_COUNT + cpu_id;
		return -1;
#endif /* K3_CLUSTER1_MSMC_PORT != UNUSED */
#if K3_CLUSTER2_MSMC_PORT != UNUSED
	case K3_CLUSTER2_MSMC_PORT:
		if (cpu_id < K3_CLUSTER2_CORE_COUNT)
			return K3_CLUSTER0_CORE_COUNT +
			       K3_CLUSTER1_CORE_COUNT + cpu_id;
		return -1;
#endif /* K3_CLUSTER2_MSMC_PORT != UNUSED */
#if K3_CLUSTER3_MSMC_PORT != UNUSED
	case K3_CLUSTER3_MSMC_PORT:
		if (cpu_id < K3_CLUSTER3_CORE_COUNT)
			return K3_CLUSTER0_CORE_COUNT +
			       K3_CLUSTER1_CORE_COUNT +
			       K3_CLUSTER2_CORE_COUNT + cpu_id;
		return -1;
#endif /* K3_CLUSTER3_MSMC_PORT != UNUSED */
	default:
		return -1;
	}
}
