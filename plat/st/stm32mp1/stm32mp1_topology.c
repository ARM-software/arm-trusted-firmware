/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>
#include <platform.h>
#include <psci.h>

/* 1 cluster, all cores into */
static const unsigned char stm32mp1_power_domain_tree_desc[] = {
	PLATFORM_CLUSTER_COUNT,
	PLATFORM_CORE_COUNT,
};

/* This function returns the platform topology */
const unsigned char *plat_get_power_domain_tree_desc(void)
{
	return stm32mp1_power_domain_tree_desc;
}

/*******************************************************************************
 * This function implements a part of the critical interface between the psci
 * generic layer and the platform that allows the former to query the platform
 * to convert an MPIDR to a unique linear index. An error code (-1) is returned
 * in case the MPIDR is invalid.
 ******************************************************************************/
int plat_core_pos_by_mpidr(u_register_t mpidr)
{
	unsigned int cluster_id, cpu_id;
	u_register_t mpidr_copy = mpidr;

	mpidr_copy &= MPIDR_AFFINITY_MASK;

	if ((mpidr_copy & ~(MPIDR_CLUSTER_MASK | MPIDR_CPU_MASK)) != 0U) {
		return -1;
	}

	cluster_id = (mpidr_copy >> MPIDR_AFF1_SHIFT) & MPIDR_AFFLVL_MASK;
	cpu_id = (mpidr_copy >> MPIDR_AFF0_SHIFT) & MPIDR_AFFLVL_MASK;

	if (cluster_id >= PLATFORM_CLUSTER_COUNT) {
		return -1;
	}

	/*
	 * Validate cpu_id by checking whether it represents a CPU in one
	 * of the two clusters present on the platform.
	 */
	if (cpu_id >= PLATFORM_CORE_COUNT) {
		return -1;
	}

	return (int)cpu_id;
}
