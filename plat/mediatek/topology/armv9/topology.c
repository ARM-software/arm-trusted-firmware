/*
 * Copyright (c) 2022, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <lib/psci/psci.h>
#include <platform_def.h>

#pragma weak plat_get_power_domain_tree_desc

static const unsigned char mtk_power_domain_tree_desc[] = {
	/* Number of root nodes */
	PLATFORM_SYSTEM_COUNT,
	/* Number of children for the root node */
	PLATFORM_CLUSTER_COUNT,
	/* Number of children for the first cluster node */
	PLATFORM_CLUSTER0_CORE_COUNT,
};

/*******************************************************************************
 * This function returns the default topology tree information.
 ******************************************************************************/
const unsigned char *plat_get_power_domain_tree_desc(void)
{
	return mtk_power_domain_tree_desc;
}
