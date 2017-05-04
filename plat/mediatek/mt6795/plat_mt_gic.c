/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <gicv2.h>
#include <plat_arm.h>
#include <platform.h>
#include <platform_def.h>

const unsigned int g0_interrupt_array[] = {
	PLAT_ARM_G0_IRQS
};

gicv2_driver_data_t arm_gic_data = {
	.gicd_base = BASE_GICD_BASE,
	.gicc_base = BASE_GICC_BASE,
	.g0_interrupt_num = ARRAY_SIZE(g0_interrupt_array),
	.g0_interrupt_array = g0_interrupt_array,
};

void plat_mt_gic_driver_init(void)
{
	gicv2_driver_init(&arm_gic_data);
}

void plat_mt_gic_init(void)
{
	gicv2_distif_init();
	gicv2_pcpu_distif_init();
	gicv2_cpuif_enable();
}

void plat_mt_gic_cpuif_enable(void)
{
	gicv2_cpuif_enable();
}

void plat_mt_gic_cpuif_disable(void)
{
	gicv2_cpuif_disable();
}

void plat_mt_gic_pcpu_init(void)
{
	gicv2_pcpu_distif_init();
}
