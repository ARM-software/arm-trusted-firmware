/*
 * Copyright (c) 2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>

#include "fpga_private.h"
#include <platform_def.h>

const unsigned char *plat_get_power_domain_tree_desc(void)
{
	/* TODO: add description of power domain topology and PSCI implementation */
	return NULL;
}

int plat_core_pos_by_mpidr(u_register_t mpidr)
{
	/*
	 * TODO: calculate core position in a way that accounts for CPUs that
	 *       potentially implement multithreading
	 */
	return (int) plat_fpga_calc_core_pos(mpidr);
}
