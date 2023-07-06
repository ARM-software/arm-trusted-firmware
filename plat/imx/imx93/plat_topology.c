/*
 * Copyright 2022-2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_helpers.h>
#include <plat/common/platform.h>

const unsigned char imx_power_domain_tree_desc[] = {
	PWR_DOMAIN_AT_MAX_LVL,
	PLATFORM_CLUSTER_COUNT,
	PLATFORM_CLUSTER0_CORE_COUNT,
};

const unsigned char *plat_get_power_domain_tree_desc(void)
{
	return imx_power_domain_tree_desc;
}

/*
 * Only one cluster is planned for i.MX9 family, no need
 * to consider the cluster id
 */
int plat_core_pos_by_mpidr(u_register_t mpidr)
{
	unsigned int cpu_id;

	mpidr &= MPIDR_AFFINITY_MASK;

	if (mpidr & ~(MPIDR_CLUSTER_MASK | MPIDR_CPU_MASK)) {
		return -1;
	}

	cpu_id = MPIDR_AFFLVL1_VAL(mpidr);

	return cpu_id;
}
