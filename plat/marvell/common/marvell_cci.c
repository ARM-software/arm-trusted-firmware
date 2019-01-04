/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

#include <drivers/arm/cci.h>

#include <plat_marvell.h>

static const int cci_map[] = {
	PLAT_MARVELL_CCI_CLUSTER0_SL_IFACE_IX,
	PLAT_MARVELL_CCI_CLUSTER1_SL_IFACE_IX
};

/****************************************************************************
 * The following functions are defined as weak to allow a platform to override
 * the way ARM CCI driver is initialised and used.
 ****************************************************************************
 */
#pragma weak plat_marvell_interconnect_init
#pragma weak plat_marvell_interconnect_enter_coherency
#pragma weak plat_marvell_interconnect_exit_coherency


/****************************************************************************
 * Helper function to initialize ARM CCI driver.
 ****************************************************************************
 */
void plat_marvell_interconnect_init(void)
{
	cci_init(PLAT_MARVELL_CCI_BASE, cci_map, ARRAY_SIZE(cci_map));
}

/****************************************************************************
 * Helper function to place current master into coherency
 ****************************************************************************
 */
void plat_marvell_interconnect_enter_coherency(void)
{
	cci_enable_snoop_dvm_reqs(MPIDR_AFFLVL1_VAL(read_mpidr_el1()));
}

/****************************************************************************
 * Helper function to remove current master from coherency
 ****************************************************************************
 */
void plat_marvell_interconnect_exit_coherency(void)
{
	cci_disable_snoop_dvm_reqs(MPIDR_AFFLVL1_VAL(read_mpidr_el1()));
}
