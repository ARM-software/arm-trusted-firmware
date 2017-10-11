/*
 * Copyright (c) 2013-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arm_config.h>
#include <cassert.h>
#include <plat_arm.h>
#include <platform_def.h>
#include "drivers/pwrc/fvp_pwrc.h"

/* The FVP power domain tree descriptor */
unsigned char fvp_power_domain_tree_desc[FVP_CLUSTER_COUNT + 2];


CASSERT(FVP_CLUSTER_COUNT && FVP_CLUSTER_COUNT <= 256, assert_invalid_fvp_cluster_count);

/*******************************************************************************
 * This function dynamically constructs the topology according to
 * FVP_CLUSTER_COUNT and returns it.
 ******************************************************************************/
const unsigned char *plat_get_power_domain_tree_desc(void)
{
	unsigned int i;

	/*
	 * The highest level is the system level. The next level is constituted
	 * by clusters and then cores in clusters.
	 */
	fvp_power_domain_tree_desc[0] = 1;
	fvp_power_domain_tree_desc[1] = FVP_CLUSTER_COUNT;

	for (i = 0; i < FVP_CLUSTER_COUNT; i++)
		fvp_power_domain_tree_desc[i + 2] = FVP_MAX_CPUS_PER_CLUSTER;


	return fvp_power_domain_tree_desc;
}

/*******************************************************************************
 * This function returns the core count within the cluster corresponding to
 * `mpidr`.
 ******************************************************************************/
unsigned int plat_arm_get_cluster_core_count(u_register_t mpidr)
{
	return FVP_MAX_CPUS_PER_CLUSTER;
}

/*******************************************************************************
 * This function implements a part of the critical interface between the psci
 * generic layer and the platform that allows the former to query the platform
 * to convert an MPIDR to a unique linear index. An error code (-1) is returned
 * in case the MPIDR is invalid.
 ******************************************************************************/
int plat_core_pos_by_mpidr(u_register_t mpidr)
{
	unsigned int clus_id, cpu_id, thread_id;

	/* Validate affinity fields */
	if (arm_config.flags & ARM_CONFIG_FVP_SHIFTED_AFF) {
		thread_id = MPIDR_AFFLVL0_VAL(mpidr);
		cpu_id = MPIDR_AFFLVL1_VAL(mpidr);
		clus_id = MPIDR_AFFLVL2_VAL(mpidr);
	} else {
		thread_id = 0;
		cpu_id = MPIDR_AFFLVL0_VAL(mpidr);
		clus_id = MPIDR_AFFLVL1_VAL(mpidr);
	}

	if (clus_id >= FVP_CLUSTER_COUNT)
		return -1;
	if (cpu_id >= FVP_MAX_CPUS_PER_CLUSTER)
		return -1;
	if (thread_id >= FVP_MAX_PE_PER_CPU)
		return -1;

	if (fvp_pwrc_read_psysr(mpidr) == PSYSR_INVALID)
		return -1;

	/*
	 * Core position calculation for FVP platform depends on the MT bit in
	 * MPIDR. This function cannot assume that the supplied MPIDR has the MT
	 * bit set even if the implementation has. For example, PSCI clients
	 * might supply MPIDR values without the MT bit set. Therefore, we
	 * inject the current PE's MT bit so as to get the calculation correct.
	 * This of course assumes that none or all CPUs on the platform has MT
	 * bit set.
	 */
	mpidr |= (read_mpidr_el1() & MPIDR_MT_MASK);
	return plat_arm_calc_core_pos(mpidr);
}
