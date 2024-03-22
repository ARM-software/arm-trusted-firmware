/*
 * Copyright (c) 2015-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <platform_def.h>

#include <arch.h>

#include <rpi_shared.h>

/* The power domain tree descriptor */
static unsigned char power_domain_tree_desc[] = {
	/* Number of root nodes */
	PLATFORM_CLUSTER_COUNT,
	/* Number of children for the first node */
	PLATFORM_CLUSTER0_CORE_COUNT,
};

/*******************************************************************************
 * This function returns the ARM default topology tree information.
 ******************************************************************************/
const unsigned char *plat_get_power_domain_tree_desc(void)
{
	return power_domain_tree_desc;
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

	/*
	 * When MT is set, lowest affinity represents the thread ID.
	 * Since we only support one thread per core, discard this field
	 * so cluster and core IDs go back into Aff1 and Aff0 respectively.
	 * The upper bits are also affected, but plat_rpi3_calc_core_pos()
	 * does not use them.
	 */
	if ((read_mpidr() & MPIDR_MT_MASK) != 0) {
		if (MPIDR_AFFLVL0_VAL(mpidr) != 0) {
			return -1;
		}
		mpidr >>= MPIDR_AFFINITY_BITS;
	}

	if (mpidr & ~(MPIDR_CLUSTER_MASK | MPIDR_CPU_MASK)) {
		return -1;
	}

	cluster_id = MPIDR_AFFLVL1_VAL(mpidr);
	cpu_id = MPIDR_AFFLVL0_VAL(mpidr);

	if (cluster_id >= PLATFORM_CLUSTER_COUNT) {
		return -1;
	}

	if (cpu_id >= PLATFORM_MAX_CPUS_PER_CLUSTER) {
		return -1;
	}

	return plat_rpi3_calc_core_pos(mpidr);
}
