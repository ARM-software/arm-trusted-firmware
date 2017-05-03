/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <bl_common.h>
#include <gicv3.h>
#include <platform.h>
#include <platform_def.h>
#include <utils.h>

/******************************************************************************
 * The following functions are defined as weak to allow a platform to override
 * the way the GICv3 driver is initialised and used.
 *****************************************************************************/
#pragma weak plat_rockchip_gic_driver_init
#pragma weak plat_rockchip_gic_init
#pragma weak plat_rockchip_gic_cpuif_enable
#pragma weak plat_rockchip_gic_cpuif_disable
#pragma weak plat_rockchip_gic_pcpu_init

/* The GICv3 driver only needs to be initialized in EL3 */
uintptr_t rdistif_base_addrs[PLATFORM_CORE_COUNT];

/* Array of Group1 secure interrupts to be configured by the gic driver */
const unsigned int g1s_interrupt_array[] = {
	PLAT_RK_G1S_IRQS
};

/* Array of Group0 interrupts to be configured by the gic driver */
const unsigned int g0_interrupt_array[] = {
	PLAT_RK_G0_IRQS
};

static unsigned int plat_rockchip_mpidr_to_core_pos(unsigned long mpidr)
{
	return (unsigned int)plat_core_pos_by_mpidr(mpidr);
}

const gicv3_driver_data_t rockchip_gic_data = {
	.gicd_base = PLAT_RK_GICD_BASE,
	.gicr_base = PLAT_RK_GICR_BASE,
	.g0_interrupt_num = ARRAY_SIZE(g0_interrupt_array),
	.g1s_interrupt_num = ARRAY_SIZE(g1s_interrupt_array),
	.g0_interrupt_array = g0_interrupt_array,
	.g1s_interrupt_array = g1s_interrupt_array,
	.rdistif_num = PLATFORM_CORE_COUNT,
	.rdistif_base_addrs = rdistif_base_addrs,
	.mpidr_to_core_pos = plat_rockchip_mpidr_to_core_pos,
};

void plat_rockchip_gic_driver_init(void)
{
	/*
	 * The GICv3 driver is initialized in EL3 and does not need
	 * to be initialized again in SEL1. This is because the S-EL1
	 * can use GIC system registers to manage interrupts and does
	 * not need GIC interface base addresses to be configured.
	 */
#ifdef IMAGE_BL31
	gicv3_driver_init(&rockchip_gic_data);
#endif
}

/******************************************************************************
 * RockChip common helper to initialize the GIC. Only invoked
 * by BL31
 *****************************************************************************/
void plat_rockchip_gic_init(void)
{
	gicv3_distif_init();
	gicv3_rdistif_init(plat_my_core_pos());
	gicv3_cpuif_enable(plat_my_core_pos());
}

/******************************************************************************
 * RockChip common helper to enable the GIC CPU interface
 *****************************************************************************/
void plat_rockchip_gic_cpuif_enable(void)
{
	gicv3_cpuif_enable(plat_my_core_pos());
}

/******************************************************************************
 * RockChip common helper to disable the GIC CPU interface
 *****************************************************************************/
void plat_rockchip_gic_cpuif_disable(void)
{
	gicv3_cpuif_disable(plat_my_core_pos());
}

/******************************************************************************
 * RockChip common helper to initialize the per-cpu redistributor interface
 * in GICv3
 *****************************************************************************/
void plat_rockchip_gic_pcpu_init(void)
{
	gicv3_rdistif_init(plat_my_core_pos());
}
