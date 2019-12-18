/*
 * Copyright (c) 2019-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_helpers.h>
#include <drivers/arm/ccn.h>

#include <platform_def.h>

static const unsigned char master_to_rn_id_map[] = {
	PLAT_BRCM_CLUSTER_TO_CCN_ID_MAP
};

static const ccn_desc_t bcm_ccn_desc = {
	.periphbase = PLAT_BRCM_CCN_BASE,
	.num_masters = ARRAY_SIZE(master_to_rn_id_map),
	.master_to_rn_id_map = master_to_rn_id_map
};

void plat_brcm_interconnect_init(void)
{
	ccn_init(&bcm_ccn_desc);
}

void plat_brcm_interconnect_enter_coherency(void)
{
	ccn_enter_snoop_dvm_domain(1 << MPIDR_AFFLVL1_VAL(read_mpidr_el1()));
}

void plat_brcm_interconnect_exit_coherency(void)
{
	ccn_exit_snoop_dvm_domain(1 << MPIDR_AFFLVL1_VAL(read_mpidr_el1()));
}
