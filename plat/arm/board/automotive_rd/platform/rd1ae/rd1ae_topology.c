/*
 * Copyright (c) 2024, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <plat/arm/common/plat_arm.h>
#include <plat/arm/css/common/css_pm.h>

/******************************************************************************
 * The power domain tree descriptor.
 *
 * This descriptor defines the layout of the power domain tree for the RD1AE
 * platform, which consists of 16 clusters.
 ******************************************************************************/
const unsigned char rd1_ae_pd_tree_desc[] = {
	(PLAT_ARM_CLUSTER_COUNT),
	PLAT_MAX_CPUS_PER_CLUSTER,
	PLAT_MAX_CPUS_PER_CLUSTER,
	PLAT_MAX_CPUS_PER_CLUSTER,
	PLAT_MAX_CPUS_PER_CLUSTER,
	PLAT_MAX_CPUS_PER_CLUSTER,
	PLAT_MAX_CPUS_PER_CLUSTER,
	PLAT_MAX_CPUS_PER_CLUSTER,
	PLAT_MAX_CPUS_PER_CLUSTER,
	PLAT_MAX_CPUS_PER_CLUSTER,
	PLAT_MAX_CPUS_PER_CLUSTER,
	PLAT_MAX_CPUS_PER_CLUSTER,
	PLAT_MAX_CPUS_PER_CLUSTER,
	PLAT_MAX_CPUS_PER_CLUSTER,
	PLAT_MAX_CPUS_PER_CLUSTER,
	PLAT_MAX_CPUS_PER_CLUSTER,
	PLAT_MAX_CPUS_PER_CLUSTER,
};

/*******************************************************************************
 * This function returns the topology tree information.
 ******************************************************************************/
const unsigned char *plat_get_power_domain_tree_desc(void)
{
	return rd1_ae_pd_tree_desc;
}

/*******************************************************************************
 * The array mapping platform core position (implemented by plat_my_core_pos())
 * to the SCMI power domain ID implemented by SCP.
 ******************************************************************************/
const uint32_t plat_css_core_pos_to_scmi_dmn_id_map[] = {
	(SET_SCMI_CHANNEL_ID(0x0) | SET_SCMI_DOMAIN_ID(0x0)),
	(SET_SCMI_CHANNEL_ID(0x0) | SET_SCMI_DOMAIN_ID(0x1)),
	(SET_SCMI_CHANNEL_ID(0x0) | SET_SCMI_DOMAIN_ID(0x2)),
	(SET_SCMI_CHANNEL_ID(0x0) | SET_SCMI_DOMAIN_ID(0x3)),
	(SET_SCMI_CHANNEL_ID(0x0) | SET_SCMI_DOMAIN_ID(0x4)),
	(SET_SCMI_CHANNEL_ID(0x0) | SET_SCMI_DOMAIN_ID(0x5)),
	(SET_SCMI_CHANNEL_ID(0x0) | SET_SCMI_DOMAIN_ID(0x6)),
	(SET_SCMI_CHANNEL_ID(0x0) | SET_SCMI_DOMAIN_ID(0x7)),
	(SET_SCMI_CHANNEL_ID(0x0) | SET_SCMI_DOMAIN_ID(0x8)),
	(SET_SCMI_CHANNEL_ID(0x0) | SET_SCMI_DOMAIN_ID(0x9)),
	(SET_SCMI_CHANNEL_ID(0x0) | SET_SCMI_DOMAIN_ID(0xA)),
	(SET_SCMI_CHANNEL_ID(0x0) | SET_SCMI_DOMAIN_ID(0xB)),
	(SET_SCMI_CHANNEL_ID(0x0) | SET_SCMI_DOMAIN_ID(0xC)),
	(SET_SCMI_CHANNEL_ID(0x0) | SET_SCMI_DOMAIN_ID(0xD)),
	(SET_SCMI_CHANNEL_ID(0x0) | SET_SCMI_DOMAIN_ID(0xE)),
	(SET_SCMI_CHANNEL_ID(0x0) | SET_SCMI_DOMAIN_ID(0xF)),
};

unsigned int plat_arm_get_cluster_core_count(u_register_t mpidr)
{
	return PLAT_MAX_CPUS_PER_CLUSTER;
}
