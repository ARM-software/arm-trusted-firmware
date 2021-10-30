/*
 * Copyright (c) 2013-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <arch.h>
#include <drivers/arm/fvp/fvp_pwrc.h>
#include <fconf_hw_config_getter.h>
#include <lib/cassert.h>
#include <plat/arm/common/arm_config.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>

#include <platform_def.h>

/* The FVP power domain tree descriptor */
static unsigned char fvp_power_domain_tree_desc[FVP_CLUSTER_COUNT + 2];


CASSERT(((FVP_CLUSTER_COUNT > 0) && (FVP_CLUSTER_COUNT <= 256)),
			assert_invalid_fvp_cluster_count);

/*******************************************************************************
 * This function dynamically constructs the topology according to cpu-map node
 * in HW_CONFIG dtb and returns it.
 ******************************************************************************/
const unsigned char *plat_get_power_domain_tree_desc(void)
{
	unsigned int i;
	uint32_t cluster_count, cpus_per_cluster;

	/*
	 * fconf APIs are not supported for RESET_TO_SP_MIN, RESET_TO_BL31 and
	 * BL2_AT_EL3 systems.
	 */
#if RESET_TO_SP_MIN || RESET_TO_BL31 || BL2_AT_EL3
	cluster_count = FVP_CLUSTER_COUNT;
	cpus_per_cluster = FVP_MAX_CPUS_PER_CLUSTER * FVP_MAX_PE_PER_CPU;
#else
	cluster_count = FCONF_GET_PROPERTY(hw_config, topology, plat_cluster_count);
	cpus_per_cluster = FCONF_GET_PROPERTY(hw_config, topology, cluster_cpu_count);
	/* Several FVP Models use the same blanket dts. Ex: FVP_Base_Cortex-A65x4
	 * and FVP_Base_Cortex-A65AEx8 both use same dts but have different number of
	 * CPUs in the cluster, as reflected by build flags FVP_MAX_CPUS_PER_CLUSTER.
	 * Take the minimum of two to ensure PSCI functions do not exceed the size of
	 * the PSCI data structures allocated at build time.
	 */
	cpus_per_cluster = MIN(cpus_per_cluster,
			(uint32_t)(FVP_MAX_CPUS_PER_CLUSTER * FVP_MAX_PE_PER_CPU));

#endif

	assert(cluster_count > 0U);
	assert(cpus_per_cluster > 0U);

	/*
	 * The highest level is the system level. The next level is constituted
	 * by clusters and then cores in clusters.
	 */
	fvp_power_domain_tree_desc[0] = 1;
	fvp_power_domain_tree_desc[1] = (unsigned char)cluster_count;

	for (i = 0; i < cluster_count; i++)
		fvp_power_domain_tree_desc[i + 2] = (unsigned char)cpus_per_cluster;

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
	if ((arm_config.flags & ARM_CONFIG_FVP_SHIFTED_AFF) != 0U) {
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
	return (int) plat_arm_calc_core_pos(mpidr);
}
