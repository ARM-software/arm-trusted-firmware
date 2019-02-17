/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>

#include <arch.h>
#include <plat/common/platform.h>

static const unsigned char plat_power_domain_tree_desc[PLAT_MAX_PWR_LVL + 1] = {
	/* One root node for the SoC */
	1,
	/* One node for each cluster */
	PLATFORM_CLUSTER_COUNT,
	/* One set of CPUs per cluster */
	PLATFORM_MAX_CPUS_PER_CLUSTER,
};

int plat_core_pos_by_mpidr(u_register_t mpidr)
{
	unsigned int cluster = MPIDR_AFFLVL1_VAL(mpidr);
	unsigned int core = MPIDR_AFFLVL0_VAL(mpidr);

	if (MPIDR_AFFLVL3_VAL(mpidr) > 0 ||
	    MPIDR_AFFLVL2_VAL(mpidr) > 0 ||
	    cluster >= PLATFORM_CLUSTER_COUNT ||
	    core >= PLATFORM_MAX_CPUS_PER_CLUSTER) {
		return -1;
	}

	return cluster * PLATFORM_MAX_CPUS_PER_CLUSTER + core;
}

const unsigned char *plat_get_power_domain_tree_desc(void)
{
	return plat_power_domain_tree_desc;
}
