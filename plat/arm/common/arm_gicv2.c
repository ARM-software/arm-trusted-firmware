/*
 * Copyright (c) 2015-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <gicv2.h>
#include <plat_arm.h>
#include <platform.h>
#include <platform_def.h>

/******************************************************************************
 * The following functions are defined as weak to allow a platform to override
 * the way the GICv2 driver is initialised and used.
 *****************************************************************************/
#pragma weak plat_arm_gic_driver_init
#pragma weak plat_arm_gic_init
#pragma weak plat_arm_gic_cpuif_enable
#pragma weak plat_arm_gic_cpuif_disable
#pragma weak plat_arm_gic_pcpu_init

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
void plat_arm_gic_driver_init(void)
{
	gicv2_driver_init(&arm_gic_data);
}

void plat_arm_gic_init(void)
{
	gicv2_distif_init();
	gicv2_pcpu_distif_init();
	gicv2_set_pe_target_mask(plat_my_core_pos());
	gicv2_cpuif_enable();
}

/******************************************************************************
 * ARM common helper to enable the GICv2 CPU interface
 *****************************************************************************/
void plat_arm_gic_cpuif_enable(void)
{
	gicv2_cpuif_enable();
}

/******************************************************************************
 * ARM common helper to disable the GICv2 CPU interface
 *****************************************************************************/
void plat_arm_gic_cpuif_disable(void)
{
	gicv2_cpuif_disable();
}

/******************************************************************************
 * ARM common helper to initialize the per cpu distributor interface in GICv2
 *****************************************************************************/
void plat_arm_gic_pcpu_init(void)
{
	gicv2_pcpu_distif_init();
	gicv2_set_pe_target_mask(plat_my_core_pos());
}

/******************************************************************************
 * Stubs for Redistributor power management. Although GICv2 doesn't have
 * Redistributor interface, these are provided for the sake of uniform GIC API
 *****************************************************************************/
void plat_arm_gic_redistif_on(void)
{
	return;
}

void plat_arm_gic_redistif_off(void)
{
	return;
}


/******************************************************************************
 * ARM common helper to save & restore the GICv3 on resume from system suspend.
 * The normal world currently takes care of saving and restoring the GICv2
 * registers due to legacy reasons. Hence we just initialize the Distributor
 * on resume from system suspend.
 *****************************************************************************/
void plat_arm_gic_save(void)
{
	return;
}

void plat_arm_gic_resume(void)
{
	gicv2_distif_init();
	gicv2_pcpu_distif_init();
}
