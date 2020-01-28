/*
 * Copyright (c) 2019-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <plat/arm/common/plat_arm.h>

/******************************************************************************
 * The power domain tree descriptor.
 ******************************************************************************/
static const unsigned char sgi575_pd_tree_desc[] = {
	PLAT_ARM_CLUSTER_COUNT,
	CSS_SGI_MAX_CPUS_PER_CLUSTER,
	CSS_SGI_MAX_CPUS_PER_CLUSTER
};

/*******************************************************************************
 * This function returns the topology tree information.
 ******************************************************************************/
const unsigned char *plat_get_power_domain_tree_desc(void)
{
	return sgi575_pd_tree_desc;
}

/*******************************************************************************
 * The array mapping platform core position (implemented by plat_my_core_pos())
 * to the SCMI power domain ID implemented by SCP.
 ******************************************************************************/
const uint32_t plat_css_core_pos_to_scmi_dmn_id_map[] = {
	0, 1, 2, 3, 4, 5, 6, 7
};
