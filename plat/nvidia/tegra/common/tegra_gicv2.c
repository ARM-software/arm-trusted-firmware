/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <platform_def.h>

#include <common/bl_common.h>
#include <drivers/arm/gicv2.h>
#include <lib/utils.h>

#include <tegra_private.h>
#include <tegra_def.h>

/******************************************************************************
 * Tegra common helper to setup the GICv2 driver data.
 *****************************************************************************/
void tegra_gic_setup(const interrupt_prop_t *interrupt_props,
		     unsigned int interrupt_props_num)
{
	/*
	 * Tegra GIC configuration settings
	 */
	static gicv2_driver_data_t tegra_gic_data;

	/*
	 * Register Tegra GICv2 driver
	 */
	tegra_gic_data.gicd_base = TEGRA_GICD_BASE;
	tegra_gic_data.gicc_base = TEGRA_GICC_BASE;
	tegra_gic_data.interrupt_props = interrupt_props;
	tegra_gic_data.interrupt_props_num = interrupt_props_num;
	gicv2_driver_init(&tegra_gic_data);
}

/******************************************************************************
 * Tegra common helper to initialize the GICv2 only driver.
 *****************************************************************************/
void tegra_gic_init(void)
{
	gicv2_distif_init();
	gicv2_pcpu_distif_init();
	gicv2_cpuif_enable();
}

/******************************************************************************
 * Tegra common helper to disable the GICv2 CPU interface
 *****************************************************************************/
void tegra_gic_cpuif_deactivate(void)
{
	gicv2_cpuif_disable();
}

/******************************************************************************
 * Tegra common helper to initialize the per cpu distributor interface
 * in GICv2
 *****************************************************************************/
void tegra_gic_pcpu_init(void)
{
	gicv2_pcpu_distif_init();
	gicv2_cpuif_enable();
}
