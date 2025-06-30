/*
 * Copyright (c) 2018-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <plat/common/platform.h>

#include <plat_private.h>

int plat_core_pos_by_mpidr(u_register_t mpidr)
{
	int ret = -1;

	if (((mpidr & MPIDR_CLUSTER_MASK) == 0U) &&
	       ((mpidr & MPIDR_CPU_MASK) < PLATFORM_CORE_COUNT)) {
		ret = (int)versal_calc_core_pos(mpidr);
	}

	return ret;
}
