/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>

#include <arch.h>
#include <arch_helpers.h>
#include <drivers/arm/ccn.h>

static const unsigned char master_to_rn_id_map[] = {
	PLAT_SQ_CLUSTER_TO_CCN_ID_MAP
};

static const ccn_desc_t sq_ccn_desc = {
	.periphbase = PLAT_SQ_CCN_BASE,
	.num_masters = ARRAY_SIZE(master_to_rn_id_map),
	.master_to_rn_id_map = master_to_rn_id_map
};

/******************************************************************************
 * Helper function to initialize SQ CCN driver.
 *****************************************************************************/
void plat_sq_interconnect_init(void)
{
	ccn_init(&sq_ccn_desc);
}

/******************************************************************************
 * Helper function to place current master into coherency
 *****************************************************************************/
void plat_sq_interconnect_enter_coherency(void)
{
	ccn_enter_snoop_dvm_domain(1 << MPIDR_AFFLVL1_VAL(read_mpidr_el1()));
}

/******************************************************************************
 * Helper function to remove current master from coherency
 *****************************************************************************/
void plat_sq_interconnect_exit_coherency(void)
{
	ccn_exit_snoop_dvm_domain(1 << MPIDR_AFFLVL1_VAL(read_mpidr_el1()));
}
