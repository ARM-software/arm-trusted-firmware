/*
 * Copyright (c) 2025, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <lib/psci/psci.h>

#include <plat_helpers.h>
#include <platform_def.h>
#include "rcar_private.h"
#include "rcar_scmi_id.h"

const unsigned char *plat_get_power_domain_tree_desc(void)
{
	static const unsigned char rcar_power_domain_tree_desc[] = {
		1,
		PLATFORM_CLUSTER_COUNT,
		PLATFORM_CLUSTER0_CORE_COUNT,
		PLATFORM_CLUSTER1_CORE_COUNT,
		PLATFORM_CLUSTER2_CORE_COUNT,
		PLATFORM_CLUSTER3_CORE_COUNT,
		PLATFORM_CLUSTER4_CORE_COUNT,
		PLATFORM_CLUSTER5_CORE_COUNT,
		PLATFORM_CLUSTER6_CORE_COUNT,
		PLATFORM_CLUSTER7_CORE_COUNT
	};

	return rcar_power_domain_tree_desc;
}

/*******************************************************************************
 * The array mapping platform core position (implemented by plat_my_core_pos())
 * to the SCMI power domain ID implemented by SCP.
 ******************************************************************************/
const uint32_t plat_css_core_pos_to_scmi_dmn_id_map[] = {
	(SET_SCMI_CHANNEL_ID(0x0) | SCP_POWER_DOMAIN_ID_PD_P_APU_CORE00),
	(SET_SCMI_CHANNEL_ID(0x0) | SCP_POWER_DOMAIN_ID_PD_P_APU_CORE01),
	(SET_SCMI_CHANNEL_ID(0x0) | SCP_POWER_DOMAIN_ID_PD_P_APU_CORE02),
	(SET_SCMI_CHANNEL_ID(0x0) | SCP_POWER_DOMAIN_ID_PD_P_APU_CORE03),
	(SET_SCMI_CHANNEL_ID(0x0) | SCP_POWER_DOMAIN_ID_PD_P_APU_CORE04),
	(SET_SCMI_CHANNEL_ID(0x0) | SCP_POWER_DOMAIN_ID_PD_P_APU_CORE05),
	(SET_SCMI_CHANNEL_ID(0x0) | SCP_POWER_DOMAIN_ID_PD_P_APU_CORE06),
	(SET_SCMI_CHANNEL_ID(0x0) | SCP_POWER_DOMAIN_ID_PD_P_APU_CORE07),
	(SET_SCMI_CHANNEL_ID(0x0) | SCP_POWER_DOMAIN_ID_PD_P_APU_CORE08),
	(SET_SCMI_CHANNEL_ID(0x0) | SCP_POWER_DOMAIN_ID_PD_P_APU_CORE09),
	(SET_SCMI_CHANNEL_ID(0x0) | SCP_POWER_DOMAIN_ID_PD_P_APU_CORE10),
	(SET_SCMI_CHANNEL_ID(0x0) | SCP_POWER_DOMAIN_ID_PD_P_APU_CORE11),
	(SET_SCMI_CHANNEL_ID(0x0) | SCP_POWER_DOMAIN_ID_PD_P_APU_CORE12),
	(SET_SCMI_CHANNEL_ID(0x0) | SCP_POWER_DOMAIN_ID_PD_P_APU_CORE13),
	(SET_SCMI_CHANNEL_ID(0x0) | SCP_POWER_DOMAIN_ID_PD_P_APU_CORE14),
	(SET_SCMI_CHANNEL_ID(0x0) | SCP_POWER_DOMAIN_ID_PD_P_APU_CORE15),
	(SET_SCMI_CHANNEL_ID(0x0) | SCP_POWER_DOMAIN_ID_PD_P_APU_CORE16),
	(SET_SCMI_CHANNEL_ID(0x0) | SCP_POWER_DOMAIN_ID_PD_P_APU_CORE17),
	(SET_SCMI_CHANNEL_ID(0x0) | SCP_POWER_DOMAIN_ID_PD_P_APU_CORE18),
	(SET_SCMI_CHANNEL_ID(0x0) | SCP_POWER_DOMAIN_ID_PD_P_APU_CORE19),
	(SET_SCMI_CHANNEL_ID(0x0) | SCP_POWER_DOMAIN_ID_PD_P_APU_CORE20),
	(SET_SCMI_CHANNEL_ID(0x0) | SCP_POWER_DOMAIN_ID_PD_P_APU_CORE21),
	(SET_SCMI_CHANNEL_ID(0x0) | SCP_POWER_DOMAIN_ID_PD_P_APU_CORE22),
	(SET_SCMI_CHANNEL_ID(0x0) | SCP_POWER_DOMAIN_ID_PD_P_APU_CORE23),
	(SET_SCMI_CHANNEL_ID(0x0) | SCP_POWER_DOMAIN_ID_PD_P_APU_CORE24),
	(SET_SCMI_CHANNEL_ID(0x0) | SCP_POWER_DOMAIN_ID_PD_P_APU_CORE25),
	(SET_SCMI_CHANNEL_ID(0x0) | SCP_POWER_DOMAIN_ID_PD_P_APU_CORE26),
	(SET_SCMI_CHANNEL_ID(0x0) | SCP_POWER_DOMAIN_ID_PD_P_APU_CORE27),
	(SET_SCMI_CHANNEL_ID(0x0) | SCP_POWER_DOMAIN_ID_PD_P_APU_CORE28),
	(SET_SCMI_CHANNEL_ID(0x0) | SCP_POWER_DOMAIN_ID_PD_P_APU_CORE29),
	(SET_SCMI_CHANNEL_ID(0x0) | SCP_POWER_DOMAIN_ID_PD_P_APU_CORE30),
	(SET_SCMI_CHANNEL_ID(0x0) | SCP_POWER_DOMAIN_ID_PD_P_APU_CORE31),
};


int plat_core_pos_by_mpidr(u_register_t mpidr)
{
	u_register_t cpu;

	/* ARMv8.2 arch */
	if ((mpidr & (MPIDR_AFFLVL_MASK << MPIDR_AFF0_SHIFT)) != 0U) {
		return -1;
	}

	cpu = plat_renesas_calc_core_pos(mpidr);
	if (cpu >= PLATFORM_CORE_COUNT) {
		return -1;
	}

	return (int)cpu;
}

int32_t rcar_cluster_pos_by_mpidr(u_register_t mpidr)
{
	u_register_t cluster;

	/* ARMv8.2 arch */
	if ((mpidr & (MPIDR_AFFLVL_MASK << MPIDR_AFF0_SHIFT)) != 0U) {
		return -1;
	}

	cluster = MPIDR_AFFLVL2_VAL(mpidr);
	if (cluster >= PLATFORM_CLUSTER_COUNT) {
		return -1;
	}

	return (int32_t)cluster;
}

/* FIXME: Selective counter enablement is mandatory here ! */
uint16_t plat_amu_aux_enables[PLATFORM_CORE_COUNT] = {
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
};
