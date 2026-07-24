/*
 * Copyright (c) 2025-2026, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <plat/arm/common/plat_arm.h>
#include <plat/arm/css/common/css_pm.h>
#include <platform_def.h>

const unsigned char rdaspen_pd_tree_desc[] = {
	PLAT_ARM_CLUSTER_COUNT,
	PLATFORM_CLUSTER_0_CORE_COUNT,
	PLATFORM_CLUSTER_1_CORE_COUNT,
	PLATFORM_CLUSTER_2_CORE_COUNT,
	PLATFORM_CLUSTER_3_CORE_COUNT,
};

static const unsigned char cluster_id_core_count_map[] = {
	PLATFORM_CLUSTER_0_CORE_COUNT,
	PLATFORM_CLUSTER_1_CORE_COUNT,
	PLATFORM_CLUSTER_2_CORE_COUNT,
	PLATFORM_CLUSTER_3_CORE_COUNT,
};

const uint32_t plat_css_core_pos_to_scmi_dmn_id_map[] = {
	/* Primary core. */
	0x0,

#if PLATFORM_CORE_COUNT > 1
	0x1,
#endif

#if PLATFORM_CORE_COUNT > 2
	0x2,
#endif

#if PLATFORM_CORE_COUNT > 3
	0x3,
#endif

#if PLATFORM_CORE_COUNT > 4
	0x4,
#endif

#if PLATFORM_CORE_COUNT > 5
	0x5,
#endif

#if PLATFORM_CORE_COUNT > 6
	0x6,
#endif

#if PLATFORM_CORE_COUNT > 7
	0x7,
#endif

#if PLATFORM_CORE_COUNT > 8
	0x8,
#endif

#if PLATFORM_CORE_COUNT > 9
	0x9,
#endif

#if PLATFORM_CORE_COUNT > 10
	0xA,
#endif

#if PLATFORM_CORE_COUNT > 11
	0xB,
#endif

#if PLATFORM_CORE_COUNT > 12
	0xC,
#endif

#if PLATFORM_CORE_COUNT > 13
	0xD,
#endif

#if PLATFORM_CORE_COUNT > 14
	0xE,
#endif

#if PLATFORM_CORE_COUNT > 15
	0xF,
#endif
};

unsigned int
plat_css_core_pos_to_scmi_channel_id(unsigned int core_pos __unused,
				     uint32_t protocol_id __unused)
{
	return 0U;
}

/*******************************************************************************
 * This function returns the topology tree information.
 ******************************************************************************/
const unsigned char *plat_get_power_domain_tree_desc(void)
{
	return rdaspen_pd_tree_desc;
}

/*******************************************************************************
 * This function returns the platform total core count.
 ******************************************************************************/
unsigned int plat_arm_get_cluster_core_count(u_register_t mpidr)
{
	unsigned int cluster_id = MPIDR_AFFLVL2_VAL(mpidr);

	if (cluster_id >= PLAT_ARM_CLUSTER_COUNT) {
		ERROR("Unexpected cluster id %u\n", cluster_id);
		panic();
	}

	return cluster_id_core_count_map[cluster_id];
}

/******************************************************************************
 * Return the cluster ID of current CPU
 *****************************************************************************/
unsigned int plat_cluster_id_by_mpidr(u_register_t mpidr)
{
	return MPIDR_AFFLVL2_VAL(mpidr);
}
