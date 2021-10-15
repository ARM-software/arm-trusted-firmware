/*
 * Copyright (c) 2021-2025, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <lib/psci/psci.h>
#include <plat_helpers.h>
#include <platform_def.h>

#include "rcar_private.h"

const unsigned char *plat_get_power_domain_tree_desc(void)
{
	static const unsigned char rcar_power_domain_tree_desc[] = {
		1,
		PLATFORM_CLUSTER_COUNT,
		PLATFORM_CLUSTER0_CORE_COUNT,
		PLATFORM_CLUSTER1_CORE_COUNT,
		PLATFORM_CLUSTER2_CORE_COUNT,
		PLATFORM_CLUSTER3_CORE_COUNT
	};

	return rcar_power_domain_tree_desc;
}

int plat_core_pos_by_mpidr(u_register_t mpidr)
{
	u_register_t cpu;

	/* ARMv8.2 arch */
	if ((mpidr & (MPIDR_AFFLVL_MASK << MPIDR_AFF0_SHIFT)) != 0)
		return -1;

	cpu = plat_renesas_calc_core_pos(mpidr);
	if (cpu >= PLATFORM_CORE_COUNT)
		return -1;

	return (int)cpu;
}

int32_t rcar_cluster_pos_by_mpidr(u_register_t mpidr)
{
	u_register_t cluster;

	/* ARMv8.2 arch */
	if ((mpidr & (MPIDR_AFFLVL_MASK << MPIDR_AFF0_SHIFT)) != 0)
		return -1;

	cluster = MPIDR_AFFLVL2_VAL(mpidr);
	if (cluster >= PLATFORM_CLUSTER_COUNT)
		return -1;

	return (int32_t)cluster;
}
