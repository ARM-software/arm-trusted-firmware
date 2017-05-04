/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <platform_def.h>
#include <psci.h>

unsigned int plat_get_aff_count(unsigned int aff_lvl, unsigned long mpidr)
{
	/* Report 1 (absent) instance at levels higher that the cluster level */
	if (aff_lvl > MPIDR_AFFLVL1)
		return PLATFORM_SYSTEM_COUNT;

	if (aff_lvl == MPIDR_AFFLVL1)
		return PLATFORM_CLUSTER_COUNT;

	return mpidr & 0x100 ? PLATFORM_CLUSTER1_CORE_COUNT :
			       PLATFORM_CLUSTER0_CORE_COUNT;
}

unsigned int plat_get_aff_state(unsigned int aff_lvl, unsigned long mpidr)
{
	return aff_lvl <= MPIDR_AFFLVL2 ? PSCI_AFF_PRESENT : PSCI_AFF_ABSENT;
}

int mt_setup_topology(void)
{
	/* [TODO] Make topology configurable via SCC */
	return 0;
}
