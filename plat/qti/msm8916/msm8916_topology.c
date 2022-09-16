/*
 * Copyright (c) 2022, Stephan Gerhold <stephan@gerhold.net>
 * Copyright (c) 2017-2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <plat/common/platform.h>

#include <platform_def.h>

static const unsigned char plat_power_domain_tree_desc[] = {
	PLATFORM_SYSTEM_COUNT,
	PLATFORM_CLUSTER_COUNT,
	PLATFORM_CPUS_PER_CLUSTER,
#if PLATFORM_CLUSTER_COUNT > 1
	PLATFORM_CPUS_PER_CLUSTER,
#endif
};

int plat_core_pos_by_mpidr(u_register_t mpidr)
{
	unsigned int cluster = MPIDR_AFFLVL1_VAL(mpidr);
	unsigned int core = MPIDR_AFFLVL0_VAL(mpidr);

	if (MPIDR_AFFLVL3_VAL(mpidr) > 0 ||
	    MPIDR_AFFLVL2_VAL(mpidr) > 0 ||
	    cluster >= PLATFORM_CLUSTER_COUNT ||
	    core >= PLATFORM_CPUS_PER_CLUSTER) {
		return -1;
	}
	return core | (cluster << PLATFORM_CPU_PER_CLUSTER_SHIFT);
}

const unsigned char *plat_get_power_domain_tree_desc(void)
{
	return plat_power_domain_tree_desc;
}
