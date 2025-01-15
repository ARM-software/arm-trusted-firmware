/*
 * Copyright (c) 2025, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>

#include <lib/psci/psci.h>
#include <platform_def.h>

#pragma weak plat_get_power_domain_tree_desc

#define PWR_DOMAIN_GROUP_COUNT		U(3)
#define PWR_DOMAIN_GROUP_NODE_0		U(4)
#define PWR_DOMAIN_GROUP_NODE_1		U(3)
#define PWR_DOMAIN_GROUP_NODE_2		U(1)

static const unsigned char mtk_power_domain_tree_desc[] = {
	PLATFORM_SYSTEM_COUNT,
	PWR_DOMAIN_GROUP_COUNT,
	PWR_DOMAIN_GROUP_NODE_0,
	PWR_DOMAIN_GROUP_NODE_1,
	PWR_DOMAIN_GROUP_NODE_2
};

/*******************************************************************************
 * This function returns the default topology tree information.
 ******************************************************************************/
const unsigned char *plat_get_power_domain_tree_desc(void)
{
	return mtk_power_domain_tree_desc;
}
