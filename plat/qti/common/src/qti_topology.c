/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2018,2020 The Linux Foundation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>

#include <platform_def.h>
#include <qti_plat.h>

/* The QTI power domain tree descriptor */
const unsigned char qti_power_domain_tree_desc[] = {
	/* One domain to represent PDC */
	PLAT_PDC_COUNT,
	/* One domain to represent RSC */
	PLAT_RSC_COUNT,
	/* There is one top-level FCM cluster */
	PLAT_CLUSTER_COUNT,
	/* No. of cores in the FCM cluster */
	PLAT_CLUSTER0_CORE_COUNT
};

/*******************************************************************************
 * This function returns the ARM default topology tree information.
 ******************************************************************************/
const unsigned char *plat_get_power_domain_tree_desc(void)
{
	return qti_power_domain_tree_desc;
}

/** Function: plat_core_pos_by_mpidr
 * This function implements a part of the critical interface between the psci
 * generic layer and the platform that allows the former to query the platform
 * to convert an MPIDR to a unique linear index. An error code (-1) is returned
 * in case the MPIDR is invalid.
 */
int plat_core_pos_by_mpidr(u_register_t mpidr)
{
	int core_linear_index = plat_qti_core_pos_by_mpidr(mpidr);

	if (core_linear_index < PLATFORM_CORE_COUNT) {
		return core_linear_index;
	} else {
		return -1;
	}
}
