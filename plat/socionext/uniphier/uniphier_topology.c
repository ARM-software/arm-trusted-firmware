/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <platform.h>

#include "uniphier.h"

static unsigned char uniphier_power_domain_tree_desc[UNIPHIER_CLUSTER_COUNT + 1];

const unsigned char *plat_get_power_domain_tree_desc(void)
{
	int i;

	uniphier_power_domain_tree_desc[0] = UNIPHIER_CLUSTER_COUNT;

	for (i = 0; i < UNIPHIER_CLUSTER_COUNT; i++)
		uniphier_power_domain_tree_desc[i + 1] =
						UNIPHIER_MAX_CPUS_PER_CLUSTER;

	return uniphier_power_domain_tree_desc;
}

int plat_core_pos_by_mpidr(u_register_t mpidr)
{
	unsigned int cluster_id, cpu_id;

	cluster_id = (mpidr >> MPIDR_AFF1_SHIFT) & MPIDR_AFFLVL_MASK;
	if (cluster_id >= UNIPHIER_CLUSTER_COUNT)
		return -1;

	cpu_id = (mpidr >> MPIDR_AFF0_SHIFT) & MPIDR_AFFLVL_MASK;
	if (cpu_id >= UNIPHIER_MAX_CPUS_PER_CLUSTER)
		return -1;

	return uniphier_calc_core_pos(mpidr);
}
