/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>

/* The Corstone700 power domain tree descriptor */
static unsigned char corstone700_power_domain_tree_desc
			[PLAT_ARM_CLUSTER_COUNT + 2];
/*******************************************************************************
 * This function dynamically constructs the topology according to
 * CLUSTER_COUNT and returns it.
 ******************************************************************************/
const unsigned char *plat_get_power_domain_tree_desc(void)
{
	int i;

	/*
	 * The highest level is the system level. The next level is constituted
	 * by clusters and then cores in clusters.
	 */
	corstone700_power_domain_tree_desc[0] = 1;
	corstone700_power_domain_tree_desc[1] = PLAT_ARM_CLUSTER_COUNT;

	for (i = 0; i < PLAT_ARM_CLUSTER_COUNT; i++)
		corstone700_power_domain_tree_desc[i + 2] = PLATFORM_CORE_COUNT;

	return corstone700_power_domain_tree_desc;
}

/******************************************************************************
 * This function implements a part of the critical interface between the PSCI
 * generic layer and the platform that allows the former to query the platform
 * to convert an MPIDR to a unique linear index. An error code (-1) is
 * returned in case the MPIDR is invalid.
 *****************************************************************************/
int plat_core_pos_by_mpidr(u_register_t mpidr)
{
	return plat_arm_calc_core_pos(mpidr);
}
