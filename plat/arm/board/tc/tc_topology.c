/*
 * Copyright (c) 2020-2026, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <plat/arm/common/plat_arm.h>
#include <plat/arm/css/common/css_pm.h>
#include <platform_def.h>

/******************************************************************************
 * The power domain tree descriptor.
 ******************************************************************************/
const unsigned char tc_pd_tree_desc[] = {
	/* No of root nodes */
	ARM_SYSTEM_COUNT,
	PLAT_ARM_CLUSTER_COUNT,
	PLAT_MAX_CPUS_PER_CLUSTER,
};

/*******************************************************************************
 * This function returns the topology tree information.
 ******************************************************************************/
const unsigned char *plat_get_power_domain_tree_desc(void)
{
	return tc_pd_tree_desc;
}

/*******************************************************************************
 * The array mapping platform core position (implemented by plat_my_core_pos())
 * to the SCMI power domain ID implemented by SCP.
 ******************************************************************************/
const uint32_t plat_css_core_pos_to_scmi_dmn_id_map[] = {
	0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7,

#if PLATFORM_CORE_COUNT == 14
	0x8, 0x9, 0xA, 0xB, 0xC, 0xD,
#endif /* PLATFORM_CORE_COUNT == 14 */
};

unsigned int
plat_css_core_pos_to_scmi_channel_id(unsigned int core_pos __unused,
				     uint32_t protocol_id __unused)
{
	return 0U;
}

/*******************************************************************************
 * This function returns the core count within the cluster corresponding to
 * `mpidr`.
 ******************************************************************************/
unsigned int plat_arm_get_cluster_core_count(u_register_t mpidr)
{
	return PLAT_MAX_CPUS_PER_CLUSTER;
}

#if ARM_PLAT_MT
/******************************************************************************
 * Return the number of PE's supported by the CPU.
 *****************************************************************************/
unsigned int plat_arm_get_cpu_pe_count(u_register_t mpidr)
{
	return PLAT_MAX_PE_PER_CPU;
}
#endif

/******************************************************************************
 * Return the cluster ID of current CPU
 *****************************************************************************/
unsigned int plat_cluster_id_by_mpidr(u_register_t mpidr)
{
	return MPIDR_AFFLVL2_VAL(mpidr);
}
