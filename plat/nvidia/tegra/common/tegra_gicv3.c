/*
 * Copyright (c) 2020, NVIDIA Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <common/bl_common.h>
#include <drivers/arm/gicv3.h>
#include <lib/utils.h>

#include <plat/common/platform.h>
#include <platform_def.h>
#include <tegra_private.h>
#include <tegra_def.h>

/* The GICv3 driver only needs to be initialized in EL3 */
static uintptr_t rdistif_base_addrs[PLATFORM_CORE_COUNT];

static unsigned int plat_tegra_mpidr_to_core_pos(unsigned long mpidr)
{
	return (unsigned int)plat_core_pos_by_mpidr(mpidr);
}

/******************************************************************************
 * Tegra common helper to setup the GICv3 driver data.
 *****************************************************************************/
void tegra_gic_setup(const interrupt_prop_t *interrupt_props,
		     unsigned int interrupt_props_num)
{
	/*
	 * Tegra GIC configuration settings
	 */
	static gicv3_driver_data_t tegra_gic_data;

	/*
	 * Register Tegra GICv3 driver
	 */
	tegra_gic_data.gicd_base = TEGRA_GICD_BASE;
	tegra_gic_data.gicr_base = TEGRA_GICR_BASE;
	tegra_gic_data.rdistif_num = PLATFORM_CORE_COUNT;
	tegra_gic_data.rdistif_base_addrs = rdistif_base_addrs;
	tegra_gic_data.mpidr_to_core_pos = plat_tegra_mpidr_to_core_pos;
	tegra_gic_data.interrupt_props = interrupt_props;
	tegra_gic_data.interrupt_props_num = interrupt_props_num;
	gicv3_driver_init(&tegra_gic_data);

	/* initialize the GICD and GICR */
	tegra_gic_init();
}

/******************************************************************************
 * Tegra common helper to initialize the GICv3 only driver.
 *****************************************************************************/
void tegra_gic_init(void)
{
	gicv3_distif_init();
	gicv3_rdistif_init(plat_my_core_pos());
	gicv3_cpuif_enable(plat_my_core_pos());
}

/******************************************************************************
 * Tegra common helper to disable the GICv3 CPU interface
 *****************************************************************************/
void tegra_gic_cpuif_deactivate(void)
{
	gicv3_cpuif_disable(plat_my_core_pos());
}

/******************************************************************************
 * Tegra common helper to initialize the per cpu distributor interface
 * in GICv3
 *****************************************************************************/
void tegra_gic_pcpu_init(void)
{
	gicv3_rdistif_init(plat_my_core_pos());
	gicv3_cpuif_enable(plat_my_core_pos());
}
