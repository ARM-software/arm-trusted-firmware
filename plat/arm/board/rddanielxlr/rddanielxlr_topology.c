/*
 * Copyright (c) 2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/arm/css/common/css_pm.h>
#include <sgi_variant.h>

/******************************************************************************
 * The power domain tree descriptor.
 ******************************************************************************/
const unsigned char rd_daniel_xlr_pd_tree_desc_multi_chip[] = {
	((PLAT_ARM_CLUSTER_COUNT) * (CSS_SGI_CHIP_COUNT)),
	CSS_SGI_MAX_CPUS_PER_CLUSTER,
	CSS_SGI_MAX_CPUS_PER_CLUSTER,
	CSS_SGI_MAX_CPUS_PER_CLUSTER,
	CSS_SGI_MAX_CPUS_PER_CLUSTER,
#if (CSS_SGI_CHIP_COUNT > 1)
	CSS_SGI_MAX_CPUS_PER_CLUSTER,
	CSS_SGI_MAX_CPUS_PER_CLUSTER,
	CSS_SGI_MAX_CPUS_PER_CLUSTER,
	CSS_SGI_MAX_CPUS_PER_CLUSTER,
#endif
#if (CSS_SGI_CHIP_COUNT > 2)
	CSS_SGI_MAX_CPUS_PER_CLUSTER,
	CSS_SGI_MAX_CPUS_PER_CLUSTER,
	CSS_SGI_MAX_CPUS_PER_CLUSTER,
	CSS_SGI_MAX_CPUS_PER_CLUSTER,
#endif
#if (CSS_SGI_CHIP_COUNT > 3)
	CSS_SGI_MAX_CPUS_PER_CLUSTER,
	CSS_SGI_MAX_CPUS_PER_CLUSTER,
	CSS_SGI_MAX_CPUS_PER_CLUSTER,
	CSS_SGI_MAX_CPUS_PER_CLUSTER
#endif
};

/*******************************************************************************
 * This function returns the topology tree information.
 ******************************************************************************/
const unsigned char *plat_get_power_domain_tree_desc(void)
{
	if (plat_arm_sgi_get_multi_chip_mode() == 1)
		return rd_daniel_xlr_pd_tree_desc_multi_chip;
	panic();
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
#if (CSS_SGI_CHIP_COUNT > 1)
	(SET_SCMI_CHANNEL_ID(0x1) | SET_SCMI_DOMAIN_ID(0x0)),
	(SET_SCMI_CHANNEL_ID(0x1) | SET_SCMI_DOMAIN_ID(0x1)),
	(SET_SCMI_CHANNEL_ID(0x1) | SET_SCMI_DOMAIN_ID(0x2)),
	(SET_SCMI_CHANNEL_ID(0x1) | SET_SCMI_DOMAIN_ID(0x3)),
#endif
#if (CSS_SGI_CHIP_COUNT > 2)
	(SET_SCMI_CHANNEL_ID(0x2) | SET_SCMI_DOMAIN_ID(0x0)),
	(SET_SCMI_CHANNEL_ID(0x2) | SET_SCMI_DOMAIN_ID(0x1)),
	(SET_SCMI_CHANNEL_ID(0x2) | SET_SCMI_DOMAIN_ID(0x2)),
	(SET_SCMI_CHANNEL_ID(0x2) | SET_SCMI_DOMAIN_ID(0x3)),
#endif
#if (CSS_SGI_CHIP_COUNT > 3)
	(SET_SCMI_CHANNEL_ID(0x3) | SET_SCMI_DOMAIN_ID(0x0)),
	(SET_SCMI_CHANNEL_ID(0x3) | SET_SCMI_DOMAIN_ID(0x1)),
	(SET_SCMI_CHANNEL_ID(0x3) | SET_SCMI_DOMAIN_ID(0x2)),
	(SET_SCMI_CHANNEL_ID(0x3) | SET_SCMI_DOMAIN_ID(0x3))
#endif
};
