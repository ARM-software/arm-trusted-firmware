/*
 * Copyright (c) 2015-2018, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <plat/common/platform.h>

#include <plat_private.h>

int32_t plat_core_pos_by_mpidr(u_register_t mpidr)
{
	int32_t core_pos = -1;

	if (((mpidr & MPIDR_CLUSTER_MASK) == 0U) &&
		((mpidr & MPIDR_CPU_MASK) < PLATFORM_CORE_COUNT)) {
		core_pos = (int32_t)zynqmp_calc_core_pos(mpidr);
	}

	return core_pos;
}
