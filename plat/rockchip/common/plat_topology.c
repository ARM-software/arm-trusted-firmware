/*
 * Copyright (c) 2013-2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <plat_private.h>
#include <platform_def.h>
#include <psci.h>

/*******************************************************************************
 * This function returns the RockChip default topology tree information.
 ******************************************************************************/
const unsigned char *plat_get_power_domain_tree_desc(void)
{
	return rockchip_power_domain_tree_desc;
}

int plat_core_pos_by_mpidr(u_register_t mpidr)
{
	unsigned int cluster_id, cpu_id;

	cpu_id = mpidr & MPIDR_AFFLVL_MASK;
	cluster_id = mpidr & MPIDR_CLUSTER_MASK;

	cpu_id += (cluster_id >> PLAT_RK_CLST_TO_CPUID_SHIFT);

	if (cpu_id >= PLATFORM_CORE_COUNT)
		return -1;

	return cpu_id;
}
