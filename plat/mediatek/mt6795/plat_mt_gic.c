/*
 * Copyright (c) 2016-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>

#include <common/interrupt_props.h>
#include <drivers/arm/gicv2.h>
#include <plat/common/platform.h>

static const interrupt_prop_t g0_interrupt_props[] = {
	INTR_PROP_DESC(FIQ_SMP_CALL_SGI, GIC_HIGHEST_SEC_PRIORITY,
		       GICV2_INTR_GROUP0, GIC_INTR_CFG_LEVEL),
};

gicv2_driver_data_t arm_gic_data = {
	.gicd_base = BASE_GICD_BASE,
	.gicc_base = BASE_GICC_BASE,
	.interrupt_props = g0_interrupt_props,
	.interrupt_props_num = ARRAY_SIZE(g0_interrupt_props),
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
