/*
 * Copyright (c) 2015-2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>

#include <drivers/arm/gic.h>
#include <drivers/arm/gicv2.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>

#if USE_GIC_DRIVER != 2
#error "This file should only be used with USE_GIC_DRIVER=2"
#endif

/******************************************************************************
 * On a GICv2 system, the Group 1 secure interrupts are treated as Group 0
 * interrupts.
 *****************************************************************************/
static const interrupt_prop_t arm_interrupt_props[] = {
	PLAT_ARM_G1S_IRQ_PROPS(GICV2_INTR_GROUP0),
	PLAT_ARM_G0_IRQ_PROPS(GICV2_INTR_GROUP0)
};

static unsigned int target_mask_array[PLATFORM_CORE_COUNT];

static const gicv2_driver_data_t arm_gic_data = {
	.gicd_base = PLAT_ARM_GICD_BASE,
	.gicc_base = PLAT_ARM_GICC_BASE,
	.interrupt_props = arm_interrupt_props,
	.interrupt_props_num = ARRAY_SIZE(arm_interrupt_props),
	.target_masks = target_mask_array,
	.target_masks_num = ARRAY_SIZE(target_mask_array),
};

/******************************************************************************
 * ARM common helper to initialize the GICv2 only driver.
 *****************************************************************************/
void __init gic_init(unsigned int cpu_idx)
{
	gicv2_driver_init(&arm_gic_data);
	gicv2_distif_init();
}

/******************************************************************************
 * ARM common helper to enable the GICv2 CPU interface
 *****************************************************************************/
void gic_cpuif_enable(unsigned int cpu_idx)
{
	gicv2_cpuif_enable();
}

/******************************************************************************
 * ARM common helper to disable the GICv2 CPU interface
 *****************************************************************************/
void gic_cpuif_disable(unsigned int cpu_idx)
{
	gicv2_cpuif_disable();
}

/******************************************************************************
 * ARM common helper to initialize the per cpu distributor interface in GICv2
 *****************************************************************************/
void gic_pcpu_init(unsigned int cpu_idx)
{
	gicv2_pcpu_distif_init();
	gicv2_set_pe_target_mask(plat_my_core_pos());
}

/******************************************************************************
 * Stubs for Redistributor power management. Although GICv2 doesn't have
 * Redistributor interface, these are provided for the sake of uniform GIC API
 *****************************************************************************/
void gic_pcpu_off(unsigned int cpu_idx)
{
	return;
}

/******************************************************************************
 * ARM common helper to save & restore the GICv3 on resume from system suspend.
 * The normal world currently takes care of saving and restoring the GICv2
 * registers due to legacy reasons. Hence we just initialize the Distributor
 * on resume from system suspend.
 *****************************************************************************/
void gic_save(void)
{
	return;
}

void gic_resume(void)
{
	gicv2_distif_init();
	gicv2_pcpu_distif_init();
}
