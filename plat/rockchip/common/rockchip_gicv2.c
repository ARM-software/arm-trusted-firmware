/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <bl_common.h>
#include <gicv2.h>
#include <platform_def.h>
#include <utils.h>

/******************************************************************************
 * The following functions are defined as weak to allow a platform to override
 * the way the GICv2 driver is initialised and used.
 *****************************************************************************/
#pragma weak plat_rockchip_gic_driver_init
#pragma weak plat_rockchip_gic_init
#pragma weak plat_rockchip_gic_cpuif_enable
#pragma weak plat_rockchip_gic_cpuif_disable
#pragma weak plat_rockchip_gic_pcpu_init

/******************************************************************************
 * On a GICv2 system, the Group 1 secure interrupts are treated as Group 0
 * interrupts.
 *****************************************************************************/
const unsigned int g0_interrupt_array[] = {
	PLAT_RK_G1S_IRQS,
};

/*
 * Ideally `rockchip_gic_data` structure definition should be a `const` but it
 * is kept as modifiable for overwriting with different GICD and GICC base when
 * running on FVP with VE memory map.
 */
gicv2_driver_data_t rockchip_gic_data = {
	.gicd_base = PLAT_RK_GICD_BASE,
	.gicc_base = PLAT_RK_GICC_BASE,
	.g0_interrupt_num = ARRAY_SIZE(g0_interrupt_array),
	.g0_interrupt_array = g0_interrupt_array,
};

/******************************************************************************
 * RockChip common helper to initialize the GICv2 only driver.
 *****************************************************************************/
void plat_rockchip_gic_driver_init(void)
{
	gicv2_driver_init(&rockchip_gic_data);
}

void plat_rockchip_gic_init(void)
{
	gicv2_distif_init();
	gicv2_pcpu_distif_init();
	gicv2_cpuif_enable();
}

/******************************************************************************
 * RockChip common helper to enable the GICv2 CPU interface
 *****************************************************************************/
void plat_rockchip_gic_cpuif_enable(void)
{
	gicv2_cpuif_enable();
}

/******************************************************************************
 * RockChip common helper to disable the GICv2 CPU interface
 *****************************************************************************/
void plat_rockchip_gic_cpuif_disable(void)
{
	gicv2_cpuif_disable();
}

/******************************************************************************
 * RockChip common helper to initialize the per cpu distributor interface
 * in GICv2
 *****************************************************************************/
void plat_rockchip_gic_pcpu_init(void)
{
	gicv2_pcpu_distif_init();
}
