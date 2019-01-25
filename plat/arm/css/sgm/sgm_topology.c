/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <plat/arm/common/plat_arm.h>

#include <sgm_plat_config.h>

/*******************************************************************************
 * This function returns the topology tree information.
 ******************************************************************************/
const unsigned char *plat_get_power_domain_tree_desc(void)
{
	return get_plat_config()->topology->power_tree;
}

/*******************************************************************************
 * This function returns the core count within the cluster corresponding to
 * `mpidr`.
 ******************************************************************************/
unsigned int plat_arm_get_cluster_core_count(u_register_t mpidr)
{
	return get_plat_config()->topology->plat_cluster_core_count;
}

/*
 * The array mapping platform core position (implemented by plat_my_core_pos())
 * to the SCMI power domain ID implemented by SCP.
 */
const uint32_t plat_css_core_pos_to_scmi_dmn_id_map[PLATFORM_CORE_COUNT] = {
			0, 1, 2, 3, 4, 5, 6, 7 };
