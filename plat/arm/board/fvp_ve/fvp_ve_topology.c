/*
 * Copyright (c) 2019, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>

#include <drivers/arm/fvp/fvp_pwrc.h>
#include <plat/arm/common/arm_config.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>

/* The FVP VE power domain tree descriptor */
static const unsigned char fvp_ve_power_domain_tree_desc[] = {
	1,
	/* No of children for the root node */
	FVP_VE_CLUSTER_COUNT,
	/* No of children for the first cluster node */
	FVP_VE_CORE_COUNT,
};

/*******************************************************************************
 * This function returns the topology according to FVP_VE_CLUSTER_COUNT.
 ******************************************************************************/
const unsigned char *plat_get_power_domain_tree_desc(void)
{
	return fvp_ve_power_domain_tree_desc;
}

/*******************************************************************************
 * Currently FVP VE has only been tested with one core, therefore 0 is returned.
 ******************************************************************************/
int plat_core_pos_by_mpidr(u_register_t mpidr)
{
	return 0;
}
