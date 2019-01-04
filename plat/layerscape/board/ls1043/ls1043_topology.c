/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>

#include <lib/cassert.h>

#include "plat_ls.h"
#include "platform_def.h"

unsigned char ls1043_power_domain_tree_desc[LS1043_CLUSTER_COUNT + 1];


CASSERT(LS1043_CLUSTER_COUNT && LS1043_CLUSTER_COUNT <= 256,
		assert_invalid_ls1043_cluster_count);

/*******************************************************************************
 * This function dynamically constructs the topology according to
 * LS1043_CLUSTER_COUNT and returns it.
 ******************************************************************************/
const unsigned char *plat_get_power_domain_tree_desc(void)
{
	int i;

	ls1043_power_domain_tree_desc[0] = LS1043_CLUSTER_COUNT;

	for (i = 0; i < LS1043_CLUSTER_COUNT; i++)
		ls1043_power_domain_tree_desc[i + 1] =
						LS1043_MAX_CPUS_PER_CLUSTER;

	return ls1043_power_domain_tree_desc;
}

/*******************************************************************************
 * This function returns the core count within the cluster corresponding to
 * `mpidr`.
 ******************************************************************************/
unsigned int plat_ls_get_cluster_core_count(u_register_t mpidr)
{
	return LS1043_MAX_CPUS_PER_CLUSTER;
}

/*******************************************************************************
 * This function implements a part of the critical interface between the psci
 * generic layer and the platform that allows the former to query the platform
 * to convert an MPIDR to a unique linear index. An error code (-1) is returned
 * in case the MPIDR is invalid.
 ******************************************************************************/
int plat_core_pos_by_mpidr(u_register_t mpidr)
{
	if (ls_check_mpidr(mpidr) == -1)
		return -1;

	return plat_ls_calc_core_pos(mpidr);
}
