/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>

#include <lib/psci/psci.h>

/* The power domain tree descriptor */
static unsigned char power_domain_tree_desc[] = {
	PLATFORM_SYSTEM_COUNT,
	PLATFORM_CLUSTER_COUNT,
	K3_CLUSTER0_CORE_COUNT,
	K3_CLUSTER1_CORE_COUNT,
	K3_CLUSTER2_CORE_COUNT,
	K3_CLUSTER3_CORE_COUNT,
};

const unsigned char *plat_get_power_domain_tree_desc(void)
{
	return power_domain_tree_desc;
}

int plat_core_pos_by_mpidr(u_register_t mpidr)
{
	unsigned int cluster = MPIDR_AFFLVL1_VAL(mpidr);
	unsigned int core = MPIDR_AFFLVL0_VAL(mpidr);

	if (MPIDR_AFFLVL3_VAL(mpidr) > 0 ||
	    MPIDR_AFFLVL2_VAL(mpidr) > 0) {
		return -1;
	}

	if (cluster > 0)
		core += K3_CLUSTER0_CORE_COUNT;
	if (cluster > 1)
		core += K3_CLUSTER1_CORE_COUNT;
	if (cluster > 2)
		core += K3_CLUSTER2_CORE_COUNT;
	if (cluster > 3)
		return -1;

	return core;
}
