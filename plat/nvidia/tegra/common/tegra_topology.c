/*
 * Copyright (c) 2015-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>

#include <arch.h>
#include <lib/psci/psci.h>
#include <plat/common/platform.h>

#pragma weak plat_core_pos_by_mpidr

/*******************************************************************************
 * This function implements a part of the critical interface between the psci
 * generic layer and the platform that allows the former to query the platform
 * to convert an MPIDR to a unique linear index. An error code (-1) is returned
 * in case the MPIDR is invalid.
 ******************************************************************************/
int32_t plat_core_pos_by_mpidr(u_register_t mpidr)
{
	u_register_t cluster_id, cpu_id;
	int32_t result;

	cluster_id = (mpidr >> (u_register_t)MPIDR_AFF1_SHIFT) &
		     (u_register_t)MPIDR_AFFLVL_MASK;
	cpu_id = (mpidr >> (u_register_t)MPIDR_AFF0_SHIFT) &
		 (u_register_t)MPIDR_AFFLVL_MASK;

	/* CorePos = CoreId + (ClusterId * cpus per cluster) */
	result = (int32_t)cpu_id + ((int32_t)cluster_id *
		 PLATFORM_MAX_CPUS_PER_CLUSTER);

	if (cluster_id >= (u_register_t)PLATFORM_CLUSTER_COUNT) {
		result = PSCI_E_NOT_PRESENT;
	}

	/*
	 * Validate cpu_id by checking whether it represents a CPU in
	 * one of the two clusters present on the platform.
	 */
	if (cpu_id >= (u_register_t)PLATFORM_MAX_CPUS_PER_CLUSTER) {
		result = PSCI_E_NOT_PRESENT;
	}

	return result;
}
