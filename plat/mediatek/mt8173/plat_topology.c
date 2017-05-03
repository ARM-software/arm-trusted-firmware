/*
 * Copyright (c) 2013-2015, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <arch.h>
#include <platform_def.h>
#include <psci.h>

#if ENABLE_PLAT_COMPAT
unsigned int plat_get_aff_count(unsigned int aff_lvl, unsigned long mpidr)
{
	/* Report 1 (absent) instance at levels higher that the cluster level */
	if (aff_lvl > MPIDR_AFFLVL1)
		return PLATFORM_SYSTEM_COUNT;

	if (aff_lvl == MPIDR_AFFLVL1)
		return PLATFORM_CLUSTER_COUNT;

	return mpidr & 0x100 ? PLATFORM_CLUSTER1_CORE_COUNT :
			       PLATFORM_CLUSTER0_CORE_COUNT;
}

unsigned int plat_get_aff_state(unsigned int aff_lvl, unsigned long mpidr)
{
	return aff_lvl <= MPIDR_AFFLVL2 ? PSCI_AFF_PRESENT : PSCI_AFF_ABSENT;
}

int mt_setup_topology(void)
{
	/* [TODO] Make topology configurable via SCC */
	return 0;
}
#else

const unsigned char mtk_power_domain_tree_desc[] = {
	/* No of root nodes */
	PLATFORM_SYSTEM_COUNT,
	/* No of children for the root node */
	PLATFORM_CLUSTER_COUNT,
	/* No of children for the first cluster node */
	PLATFORM_CLUSTER0_CORE_COUNT,
	/* No of children for the second cluster node */
	PLATFORM_CLUSTER1_CORE_COUNT
};

/*******************************************************************************
 * This function returns the MT8173 default topology tree information.
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

	mpidr &= MPIDR_AFFINITY_MASK;

	if (mpidr & ~(MPIDR_CLUSTER_MASK | MPIDR_CPU_MASK))
		return -1;

	cluster_id = (mpidr >> MPIDR_AFF1_SHIFT) & MPIDR_AFFLVL_MASK;
	cpu_id = (mpidr >> MPIDR_AFF0_SHIFT) & MPIDR_AFFLVL_MASK;

	if (cluster_id >= PLATFORM_CLUSTER_COUNT)
		return -1;

	/*
	 * Validate cpu_id by checking whether it represents a CPU in
	 * one of the two clusters present on the platform.
	 */
	if (cpu_id >= PLATFORM_MAX_CPUS_PER_CLUSTER)
		return -1;

	return (cpu_id + (cluster_id * 4));
}
#endif
