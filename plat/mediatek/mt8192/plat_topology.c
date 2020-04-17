/*
 * Copyright (c) 2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Project Includes */
#include <arch.h>
#include <arch_helpers.h>
#include <lib/psci/psci.h>

/* Platform Includes */
#include <plat_helpers.h>
#include <platform_def.h>

const unsigned char mtk_power_domain_tree_desc[] = {
	/* Number of root nodes */
	PLATFORM_SYSTEM_COUNT,
	/* Number of children for the root node */
	PLATFORM_CLUSTER_COUNT,
	/* Number of children for the first cluster node */
	PLATFORM_CLUSTER0_CORE_COUNT,
};

/*******************************************************************************
 * This function returns the MT8192 default topology tree information.
 ******************************************************************************/
const unsigned char *plat_get_power_domain_tree_desc(void)
{
	return mtk_power_domain_tree_desc;
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

	if (read_mpidr() & MPIDR_MT_MASK) {
		/* ARMv8.2 arch */
		if (mpidr & (MPIDR_AFFLVL_MASK << MPIDR_AFF0_SHIFT)) {
			return -1;
		}
		return plat_mediatek_calc_core_pos(mpidr);
	}

	mpidr &= MPIDR_AFFINITY_MASK;

	if (mpidr & ~(MPIDR_CLUSTER_MASK | MPIDR_CPU_MASK)) {
		return -1;
	}

	cluster_id = (mpidr >> MPIDR_AFF1_SHIFT) & MPIDR_AFFLVL_MASK;
	cpu_id = (mpidr >> MPIDR_AFF0_SHIFT) & MPIDR_AFFLVL_MASK;

	if (cluster_id >= PLATFORM_CLUSTER_COUNT) {
		return -1;
	}

	/*
	 * Validate cpu_id by checking whether it represents a CPU in
	 * one of the two clusters present on the platform.
	 */
	if (cpu_id >= PLATFORM_MAX_CPUS_PER_CLUSTER) {
		return -1;
	}

	return (cpu_id + (cluster_id * 8));
}
