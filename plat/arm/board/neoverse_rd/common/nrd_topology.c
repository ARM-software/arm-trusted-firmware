/*
 * Copyright (c) 2018-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <plat/arm/common/plat_arm.h>

/*
 * Common topology related methods for Neoverse RD platforms
 */
/*******************************************************************************
 * This function returns the core count within the cluster corresponding to
 * `mpidr`.
 ******************************************************************************/
unsigned int plat_arm_get_cluster_core_count(u_register_t mpidr)
{
	return NRD_MAX_CPUS_PER_CLUSTER;
}

#if ARM_PLAT_MT
/******************************************************************************
 * Return the number of PE's supported by the CPU.
 *****************************************************************************/
unsigned int plat_arm_get_cpu_pe_count(u_register_t mpidr)
{
	return NRD_MAX_PE_PER_CPU;
}
#endif
