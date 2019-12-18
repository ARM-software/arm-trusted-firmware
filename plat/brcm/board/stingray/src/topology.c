/*
 * Copyright (c) 2019-2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdint.h>

#include <plat_brcm.h>
#include <platform_def.h>

/*
 * On Stingray, the system power level is the highest power level.
 * The first entry in the power domain descriptor specifies the
 * number of system power domains i.e. 1.
 */
#define SR_PWR_DOMAINS_AT_MAX_PWR_LVL	 1

/*
 * The Stingray power domain tree descriptor. The cluster power domains
 * are arranged so that when the PSCI generic code creates the power
 * domain tree, the indices of the CPU power domain nodes it allocates
 * match the linear indices returned by plat_core_pos_by_mpidr()
 * i.e. CLUSTER0 CPUs are allocated indices from 0 to 1 and the higher
 * indices for other Cluster CPUs.
 */
const unsigned char sr_power_domain_tree_desc[] = {
	/* No of root nodes */
	SR_PWR_DOMAINS_AT_MAX_PWR_LVL,
	/* No of children for the root node */
	BRCM_CLUSTER_COUNT,
	/* No of children for the first cluster node */
	PLATFORM_CLUSTER0_CORE_COUNT,
	/* No of children for the second cluster node */
	PLATFORM_CLUSTER1_CORE_COUNT,
	/* No of children for the third cluster node */
	PLATFORM_CLUSTER2_CORE_COUNT,
	/* No of children for the fourth cluster node */
	PLATFORM_CLUSTER3_CORE_COUNT,
};

/*******************************************************************************
 * This function returns the Stingray topology tree information.
 ******************************************************************************/
const unsigned char *plat_get_power_domain_tree_desc(void)
{
	return sr_power_domain_tree_desc;
}

int plat_core_pos_by_mpidr(u_register_t mpidr)
{
	return plat_brcm_calc_core_pos(mpidr);
}
