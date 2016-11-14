/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arm_def.h>
#include <plat_arm.h>
#include "juno_def.h"

/*
 * On Juno, the system power level is the highest power level.
 * The first entry in the power domain descriptor specifies the
 * number of system power domains i.e. 1.
 */
#define JUNO_PWR_DOMAINS_AT_MAX_PWR_LVL	 ARM_SYSTEM_COUNT

/*
 * The Juno power domain tree descriptor. The cluster power domains
 * are arranged so that when the PSCI generic code creates the power
 * domain tree, the indices of the CPU power domain nodes it allocates
 * match the linear indices returned by plat_core_pos_by_mpidr()
 * i.e. CLUSTER1 CPUs are allocated indices from 0 to 3 and the higher
 * indices for CLUSTER0 CPUs.
 */
const unsigned char juno_power_domain_tree_desc[] = {
	/* No of root nodes */
	JUNO_PWR_DOMAINS_AT_MAX_PWR_LVL,
	/* No of children for the root node */
	JUNO_CLUSTER_COUNT,
	/* No of children for the first cluster node */
	JUNO_CLUSTER1_CORE_COUNT,
	/* No of children for the second cluster node */
	JUNO_CLUSTER0_CORE_COUNT
};

/*******************************************************************************
 * This function returns the Juno topology tree information.
 ******************************************************************************/
const unsigned char *plat_get_power_domain_tree_desc(void)
{
	return juno_power_domain_tree_desc;
}

/*******************************************************************************
 * This function returns the core count within the cluster corresponding to
 * `mpidr`.
 ******************************************************************************/
unsigned int plat_arm_get_cluster_core_count(u_register_t mpidr)
{
	return (((mpidr) & 0x100) ? JUNO_CLUSTER1_CORE_COUNT :\
				JUNO_CLUSTER0_CORE_COUNT);
}

/*
 * The array mapping platform core position (implemented by plat_my_core_pos())
 * to the SCMI power domain ID implemented by SCP.
 */
const uint32_t plat_css_core_pos_to_scmi_dmn_id_map[PLATFORM_CORE_COUNT] = {
			2, 3, 4, 5, 0, 1 };
