/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2018,2020 The Linux Foundation. All rights reserved.
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>

#include <platform_def.h>
#include <qti_plat.h>

static const unsigned char bruin_power_domain_tree_desc[] = {
	1,  /* one system-level node at the top */
	/* There is one top-level FCM cluster */
	PLAT_CLUSTER_COUNT,
	/* No. of cores in the FCM cluster */
	PLAT_CLUSTER0_CORE_COUNT
};

const unsigned char *plat_get_power_domain_tree_desc(void)
{
	return bruin_power_domain_tree_desc;
}

/** Function: plat_core_pos_by_mpidr
 * This function implements a part of the critical interface between the psci
 * generic layer and the platform that allows the former to query the platform
 * to convert an MPIDR to a unique linear index. An error code (-1) is returned
 * in case the MPIDR is invalid.
 */
int plat_core_pos_by_mpidr(u_register_t mpidr)
{
	unsigned int core_linear_index = plat_qti_core_pos_by_mpidr(mpidr);

	if (core_linear_index < PLATFORM_CORE_COUNT) {
		return (int)core_linear_index;
	} else {
		return -1;
	}
}
