/*
 * Copyright (c) 2015-2023, ARM Limited and Contributors. All rights reserved.
 *
 * Copyright (C) 2022-2023 Nuvoton Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_helpers.h>
#include <common/debug.h>
#include <lib/psci/psci.h>
#include <lib/semihosting.h>
#include <plat/common/platform.h>

/*
 * Since NPCM845 have only powered/non-powered state,
 * the tree is structure of level 0
 * (Single cluster == 0) and 4 representing a "leaf" for every CPU
 */
const unsigned char npcm845x_power_domain_tree_desc[] = {
	PLATFORM_CLUSTER_COUNT,
	PLATFORM_MAX_CPU_PER_CLUSTER
};

const unsigned char *plat_get_power_domain_tree_desc(void)
{
	/* A single cluster with 4 CPUs */
	return npcm845x_power_domain_tree_desc;
}

int plat_core_pos_by_mpidr(u_register_t mpidr)
{
	unsigned int cluster_id, cpu_id;

	mpidr &= MPIDR_AFFINITY_MASK;

	if (mpidr & ~(MPIDR_CLUSTER_MASK | MPIDR_CPU_MASK)) {
		return -1;
	}

	cluster_id = (unsigned int)MPIDR_AFFLVL1_VAL(mpidr);
	cpu_id = (unsigned int)MPIDR_AFFLVL0_VAL(mpidr);

	if (cluster_id > PLATFORM_CLUSTER_COUNT ||
		cpu_id > PLATFORM_MAX_CPU_PER_CLUSTER) {
		return -1;
	}

	return (int)(cpu_id + (cluster_id * 4));
}
