/*
 * Copyright (c) 2016-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>

#include <arch.h>
#include <drivers/arm/cci.h>
#include <lib/utils.h>
#include <plat/arm/common/plat_arm.h>

static const int cci_map[] = {
	PLAT_ARM_CCI_CLUSTER0_SL_IFACE_IX,
	PLAT_ARM_CCI_CLUSTER1_SL_IFACE_IX
};

/******************************************************************************
 * Helper function to initialize ARM CCI driver.
 *****************************************************************************/
void __init plat_arm_interconnect_init(void)
{
	cci_init(PLAT_ARM_CCI_BASE, cci_map, (unsigned int)ARRAY_SIZE(cci_map));
}

/******************************************************************************
 * Helper function to place current master into coherency
 *****************************************************************************/
void plat_arm_interconnect_enter_coherency(void)
{
	cci_enable_snoop_dvm_reqs((unsigned int)MPIDR_AFFLVL1_VAL(read_mpidr_el1()));
}

/******************************************************************************
 * Helper function to remove current master from coherency
 *****************************************************************************/
void plat_arm_interconnect_exit_coherency(void)
{
	cci_disable_snoop_dvm_reqs((unsigned int)MPIDR_AFFLVL1_VAL(read_mpidr_el1()));
}
