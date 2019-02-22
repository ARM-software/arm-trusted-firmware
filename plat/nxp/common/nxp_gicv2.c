/*
 * Copyright 2018-2019 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <platform_def.h>

#include <drivers/arm/gicv2.h>
#include <plat/common/platform.h>

#include <plat_common.h>

/******************************************************************************
 * On a GICv2 system, the Group 1 secure interrupts are treated as Group 0
 * interrupts.
 *****************************************************************************/
static const interrupt_prop_t ls_interrupt_props[] = {
	PLAT_G1S_IRQ_PROPS(GICV2_INTR_GROUP0),
	PLAT_G0_IRQ_PROPS(GICV2_INTR_GROUP0)
};

static unsigned int target_mask_array[PLATFORM_CORE_COUNT];

static gicv2_driver_data_t ls_gic_data = {
	/* GIC base would be populated in gic driver init */
	.gicd_base = 0,
	.gicc_base = 0,
	.interrupt_props = ls_interrupt_props,
	.interrupt_props_num = ARRAY_SIZE(ls_interrupt_props),
	.target_masks = target_mask_array,
	.target_masks_num = ARRAY_SIZE(target_mask_array),
};

/******************************************************************************
 * ARM common helper to initialize the GICv2 only driver.
 *****************************************************************************/
void plat_gic_driver_init(void)
{
	uint32_t gicc_base, gicd_base;

	get_gic_offset(&gicc_base, &gicd_base);
	ls_gic_data.gicd_base = (uintptr_t)gicd_base;
	ls_gic_data.gicc_base = (uintptr_t)gicc_base;

	gicv2_driver_init(&ls_gic_data);
}

void plat_gic_init(void)
{
	gicv2_distif_init();
	gicv2_pcpu_distif_init();
	gicv2_cpuif_enable();
}

/******************************************************************************
 * ARM common helper to enable the GICv2 CPU interface
 *****************************************************************************/
void plat_gic_cpuif_enable(void)
{
	gicv2_cpuif_enable();
}

/******************************************************************************
 * ARM common helper to disable the GICv2 CPU interface
 *****************************************************************************/
void plat_gic_cpuif_disable(void)
{
	gicv2_cpuif_disable();
}

/******************************************************************************
 * NXP common helper to initialize GICv2 per cpu
 *****************************************************************************/
void plat_gic_pcpu_init(void)
{
	gicv2_pcpu_distif_init();
	gicv2_cpuif_enable();
}

/******************************************************************************
 * Stubs for Redistributor power management. Although GICv2 doesn't have
 * Redistributor interface, these are provided for the sake of uniform GIC API
 *****************************************************************************/
void plat_gic_redistif_on(void)
{
}

void plat_gic_redistif_off(void)
{
}
