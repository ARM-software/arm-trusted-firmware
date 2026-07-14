/*
 * Copyright (c) 2024-2026, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <plat/arm/common/plat_arm.h>
#include <plat/arm/css/common/css_pm.h>

#include <drivers/arm/css/scmi.h>

/******************************************************************************
 * The power domain tree descriptor.
 ******************************************************************************/
const unsigned char rd_v3_pd_tree_desc[] = {
	(PLAT_ARM_CLUSTER_COUNT) * (NRD_CHIP_COUNT),
	NRD_MAX_CPUS_PER_CLUSTER,
	NRD_MAX_CPUS_PER_CLUSTER,
	NRD_MAX_CPUS_PER_CLUSTER,
	NRD_MAX_CPUS_PER_CLUSTER,
#if (PLAT_ARM_CLUSTER_COUNT > 4 || \
	(NRD_PLATFORM_VARIANT == 2 && NRD_CHIP_COUNT > 1))
	NRD_MAX_CPUS_PER_CLUSTER,
	NRD_MAX_CPUS_PER_CLUSTER,
	NRD_MAX_CPUS_PER_CLUSTER,
	NRD_MAX_CPUS_PER_CLUSTER,
#endif
#if (PLAT_ARM_CLUSTER_COUNT > 8 || \
	(NRD_PLATFORM_VARIANT == 2 && NRD_CHIP_COUNT > 2))
	NRD_MAX_CPUS_PER_CLUSTER,
	NRD_MAX_CPUS_PER_CLUSTER,
	NRD_MAX_CPUS_PER_CLUSTER,
	NRD_MAX_CPUS_PER_CLUSTER,
#endif
#if (PLAT_ARM_CLUSTER_COUNT > 12 || \
	(NRD_PLATFORM_VARIANT == 2 && NRD_CHIP_COUNT > 3))
	NRD_MAX_CPUS_PER_CLUSTER,
	NRD_MAX_CPUS_PER_CLUSTER,
	NRD_MAX_CPUS_PER_CLUSTER,
	NRD_MAX_CPUS_PER_CLUSTER,
#endif
};

/*******************************************************************************
 * This function returns the topology tree information.
 ******************************************************************************/
const unsigned char *plat_get_power_domain_tree_desc(void)
{
	return rd_v3_pd_tree_desc;
}

/*******************************************************************************
 * The array mapping platform core position (implemented by plat_my_core_pos())
 * to the SCMI power domain ID implemented by SCP.
 ******************************************************************************/
#if NRD_PLATFORM_VARIANT == 2
const uint32_t plat_css_core_pos_to_scmi_dmn_id_map[] = {
	0x0, 0x1, 0x2, 0x3,

#if NRD_CHIP_COUNT > 1
	0x0, 0x1, 0x2, 0x3,
#endif

#if NRD_CHIP_COUNT > 2
	0x0, 0x1, 0x2, 0x3,
#endif

#if NRD_CHIP_COUNT > 3
	0x0, 0x1, 0x2, 0x3,
#endif
};
#else
const uint32_t plat_css_core_pos_to_scmi_dmn_id_map[] = {
	0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7,

#if NRD_PLATFORM_VARIANT == 0
	0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF,
#endif
};
#endif

unsigned int plat_css_core_pos_to_scmi_channel_id(unsigned int core_pos,
						  uint32_t protocol_id)
{
#if NRD_PLATFORM_VARIANT == 2
	if (protocol_id == SCMI_SYS_PWR_PROTO_ID) {
		core_pos = 0U;
	}

	return core_pos / (PLATFORM_CORE_COUNT / NRD_CHIP_COUNT);
#else
	return 0U;
#endif
}
