/*
 * Copyright 2024-2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <plat/common/platform.h>
#include <plat_helpers.h>

const unsigned char *plat_get_power_domain_tree_desc(void)
{
	static const unsigned char s32g_power_domain_tree_desc[] = {
		PLATFORM_SYSTEM_COUNT,
		PLATFORM_CLUSTER_COUNT,
		PLATFORM_CORE_COUNT / U(2),
		PLATFORM_CORE_COUNT / U(2),
	};

	return s32g_power_domain_tree_desc;
}

int plat_core_pos_by_mpidr(u_register_t mpidr)
{
	unsigned int cluster_id, cpu_id, core_id;
	u_register_t mpidr_priv = mpidr;

	mpidr_priv &= MPIDR_AFFINITY_MASK;

	if ((mpidr_priv & ~(MPIDR_CLUSTER_MASK | MPIDR_CPU_MASK)) != 0) {
		return -1;
	}

	cluster_id = MPIDR_AFFLVL1_VAL(mpidr_priv);
	cpu_id = MPIDR_AFFLVL0_VAL(mpidr_priv);

	if ((cluster_id >= PLATFORM_CLUSTER_COUNT) ||
	    (cpu_id >= PLATFORM_MAX_CPUS_PER_CLUSTER)) {
		return -1;
	}

	core_id = s32g2_core_pos_by_mpidr(mpidr_priv);
	if (core_id >= PLATFORM_CORE_COUNT) {
		return -1;
	}

	return (int)core_id;
}
