/*
 * Copyright (c) 2015-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <plat_arm.h>

#if ARM_PLAT_MT
#pragma weak plat_arm_get_cpu_pe_count
#endif

/******************************************************************************
 * This function implements a part of the critical interface between the psci
 * generic layer and the platform that allows the former to query the platform
 * to convert an MPIDR to a unique linear index. An error code (-1) is
 * returned in case the MPIDR is invalid.
 *****************************************************************************/
int plat_core_pos_by_mpidr(u_register_t mpidr)
{
	if (arm_check_mpidr(mpidr) == 0)
		return plat_arm_calc_core_pos(mpidr);

	return -1;
}

#if ARM_PLAT_MT
/******************************************************************************
 * This function returns the PE count within the physical cpu corresponding to
 * `mpidr`. Now one cpu only have one thread, so just return 1.
 *****************************************************************************/
unsigned int plat_arm_get_cpu_pe_count(u_register_t mpidr)
{
	return 1;
}
#endif /* ARM_PLAT_MT */
