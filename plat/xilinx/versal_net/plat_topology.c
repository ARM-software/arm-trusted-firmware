/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2018-2022, Xilinx, Inc. All rights reserved.
 * Copyright (C) 2022, Advanced Micro Devices, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <plat/common/platform.h>

#include <plat_private.h>
#include <platform_def.h>

static const uint8_t plat_power_domain_tree_desc[] = {
	/* Number of root nodes */
	1,
	/* Number of clusters */
	PLATFORM_CLUSTER_COUNT,
	/* Number of children for the first cluster node */
	PLATFORM_CORE_COUNT_PER_CLUSTER,
	/* Number of children for the second cluster node */
	PLATFORM_CORE_COUNT_PER_CLUSTER,
	/* Number of children for the third cluster node */
	PLATFORM_CORE_COUNT_PER_CLUSTER,
	/* Number of children for the fourth cluster node */
	PLATFORM_CORE_COUNT_PER_CLUSTER,
};

const uint8_t *plat_get_power_domain_tree_desc(void)
{
	return plat_power_domain_tree_desc;
}

/*******************************************************************************
 * This function implements a part of the critical interface between the psci
 * generic layer and the platform that allows the former to query the platform
 * to convert an MPIDR to a unique linear index. An error code (-1) is returned
 * in case the MPIDR is invalid.
 ******************************************************************************/
int32_t plat_core_pos_by_mpidr(u_register_t mpidr)
{
	uint32_t cluster_id, cpu_id;

	mpidr &= MPIDR_AFFINITY_MASK;

	cluster_id = MPIDR_AFFLVL2_VAL(mpidr);
	cpu_id = MPIDR_AFFLVL1_VAL(mpidr);

	if (cluster_id >= PLATFORM_CLUSTER_COUNT) {
		return -3;
	}

	/*
	 * Validate cpu_id by checking whether it represents a CPU in
	 * one of the two clusters present on the platform.
	 */
	if (cpu_id >= PLATFORM_CORE_COUNT_PER_CLUSTER) {
		return -1;
	}

	return (cpu_id + (cluster_id * PLATFORM_CORE_COUNT_PER_CLUSTER));
}
