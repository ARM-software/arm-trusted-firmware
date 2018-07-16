/*
 * Copyright (c) 2016-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <bl_common.h>
#include <gicv2.h>
#include <platform.h>
#include <platform_def.h>
#include <utils.h>

#include <stm32mp1_private.h>

/******************************************************************************
 * On a GICv2 system, the Group 1 secure interrupts are treated as Group 0
 * interrupts.
 *****************************************************************************/
static const interrupt_prop_t stm32mp1_interrupt_props[] = {
	PLATFORM_G1S_PROPS(GICV2_INTR_GROUP0),
	PLATFORM_G0_PROPS(GICV2_INTR_GROUP0)
};

static unsigned int target_mask_array[PLATFORM_CORE_COUNT];

static const gicv2_driver_data_t platform_gic_data = {
	.gicd_base = STM32MP1_GICD_BASE,
	.gicc_base = STM32MP1_GICC_BASE,
	.interrupt_props = stm32mp1_interrupt_props,
	.interrupt_props_num = ARRAY_SIZE(stm32mp1_interrupt_props),
	.target_masks = target_mask_array,
	.target_masks_num = ARRAY_SIZE(target_mask_array),
};

void stm32mp1_gic_init(void)
{
	gicv2_driver_init(&platform_gic_data);
	gicv2_distif_init();

	stm32mp1_gic_pcpu_init();
}

void stm32mp1_gic_pcpu_init(void)
{
	gicv2_pcpu_distif_init();
	gicv2_set_pe_target_mask(plat_my_core_pos());
	gicv2_cpuif_enable();
}
