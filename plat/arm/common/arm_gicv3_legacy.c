/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arm_def.h>
#include <arm_gic.h>
#include <plat_arm.h>
#include <platform.h>
#include <platform_def.h>

/******************************************************************************
 * The following function is defined as weak to allow a platform to override
 * the way the Legacy GICv3 driver is initialised and used.
 *****************************************************************************/
#pragma weak plat_arm_gic_driver_init
#pragma weak plat_arm_gic_init
#pragma weak plat_arm_gic_cpuif_enable
#pragma weak plat_arm_gic_cpuif_disable
#pragma weak plat_arm_gic_pcpu_init

/*
 * In the GICv3 Legacy mode, the Group 1 secure interrupts are treated as Group
 * 0 interrupts.
 */
static const unsigned int irq_sec_array[] = {
	PLAT_ARM_G0_IRQS,
	PLAT_ARM_G1S_IRQS
};

void plat_arm_gic_driver_init(void)
{
	arm_gic_init(PLAT_ARM_GICC_BASE,
		     PLAT_ARM_GICD_BASE,
		     PLAT_ARM_GICR_BASE,
		     irq_sec_array,
		     ARRAY_SIZE(irq_sec_array));
}

/******************************************************************************
 * ARM common helper to initialize the GIC.
 *****************************************************************************/
void plat_arm_gic_init(void)
{
	arm_gic_setup();
}

/******************************************************************************
 * ARM common helper to enable the GIC CPU interface
 *****************************************************************************/
void plat_arm_gic_cpuif_enable(void)
{
	arm_gic_cpuif_setup();
}

/******************************************************************************
 * ARM common helper to disable the GIC CPU interface
 *****************************************************************************/
void plat_arm_gic_cpuif_disable(void)
{
	arm_gic_cpuif_deactivate();
}

/******************************************************************************
 * ARM common helper to initialize the per-cpu distributor in GICv2 or
 * redistributor interface in GICv3.
 *****************************************************************************/
void plat_arm_gic_pcpu_init(void)
{
	arm_gic_pcpu_distif_setup();
}

/******************************************************************************
 * Stubs for Redistributor power management. Although legacy configuration isn't
 * supported, these are provided for the sake of uniform GIC API
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
 *****************************************************************************/
void plat_arm_gic_save(void)
{
	return;
}

void plat_arm_gic_resume(void)
{
	arm_gic_setup();
}
