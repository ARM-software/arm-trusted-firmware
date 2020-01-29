/*
 * Copyright (c) 2019-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <plat/arm/common/plat_arm.h>

/******************************************************************************
 * The power domain tree descriptor. RD-E1-Edge platform consists of two
 * clusters with eight CPUs in each cluster. The CPUs are multi-threaded with
 * two threads per CPU.
 ******************************************************************************/
static const unsigned char rde1edge_pd_tree_desc[] = {
	CSS_SGI_CHIP_COUNT,
	PLAT_ARM_CLUSTER_COUNT,
	CSS_SGI_MAX_CPUS_PER_CLUSTER * CSS_SGI_MAX_PE_PER_CPU,
	CSS_SGI_MAX_CPUS_PER_CLUSTER * CSS_SGI_MAX_PE_PER_CPU
};

/******************************************************************************
 * This function returns the topology tree information.
 ******************************************************************************/
const unsigned char *plat_get_power_domain_tree_desc(void)
{
	return rde1edge_pd_tree_desc;
}

/*******************************************************************************
 * The array mapping platform core position (implemented by plat_my_core_pos())
 * to the SCMI power domain ID implemented by SCP.
 ******************************************************************************/
const uint32_t plat_css_core_pos_to_scmi_dmn_id_map[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,		\
	16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31
};
