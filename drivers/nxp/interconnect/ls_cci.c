/*
 * Copyright 2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <arch.h>
#include <cci.h>

#include <plat_arm.h>

/******************************************************************************
 * The following functions are defined as weak to allow a platform to override
 * the way ARM CCI driver is initialised and used.
 *****************************************************************************/
#pragma weak plat_arm_interconnect_enter_coherency
#pragma weak plat_arm_interconnect_exit_coherency

/******************************************************************************
 * Helper function to place current master into coherency
 *****************************************************************************/
void plat_ls_interconnect_enter_coherency(unsigned int num_clusters)
{
	cci_enable_snoop_dvm_reqs(MPIDR_AFFLVL1_VAL(read_mpidr_el1()));

	for (uint32_t index = 1U; index < num_clusters; index++) {
		cci_enable_snoop_dvm_reqs(index);
	}
}

/******************************************************************************
 * Helper function to remove current master from coherency
 *****************************************************************************/
void plat_ls_interconnect_exit_coherency(void)
{
	cci_disable_snoop_dvm_reqs(MPIDR_AFFLVL1_VAL(read_mpidr_el1()));
}
