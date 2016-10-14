/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <bl_common.h>
#include <gicv3.h>
#include <interrupt_props.h>
#include <k3_gicv3.h>
#include <platform.h>
#include <platform_def.h>
#include <utils.h>

/* The GICv3 driver only needs to be initialized in EL3 */
uintptr_t rdistif_base_addrs[PLATFORM_CORE_COUNT];

static const interrupt_prop_t k3_interrupt_props[] = {
	PLAT_ARM_G1S_IRQ_PROPS(INTR_GROUP1S),
	PLAT_ARM_G0_IRQ_PROPS(INTR_GROUP0)
};

static unsigned int k3_mpidr_to_core_pos(unsigned long mpidr)
{
	return (unsigned int)plat_core_pos_by_mpidr(mpidr);
}

gicv3_driver_data_t k3_gic_data = {
	.rdistif_num = PLATFORM_CORE_COUNT,
	.rdistif_base_addrs = rdistif_base_addrs,
	.interrupt_props = k3_interrupt_props,
	.interrupt_props_num = ARRAY_SIZE(k3_interrupt_props),
	.mpidr_to_core_pos = k3_mpidr_to_core_pos,
};

void k3_gic_driver_init(uintptr_t gicd_base, uintptr_t gicr_base)
{
	/*
	 * The GICv3 driver is initialized in EL3 and does not need
	 * to be initialized again in SEL1. This is because the S-EL1
	 * can use GIC system registers to manage interrupts and does
	 * not need GIC interface base addresses to be configured.
	 */
	k3_gic_data.gicd_base = gicd_base;
	k3_gic_data.gicr_base = gicr_base;
	gicv3_driver_init(&k3_gic_data);
}

void k3_gic_init(void)
{
	gicv3_distif_init();
	gicv3_rdistif_init(plat_my_core_pos());
	gicv3_cpuif_enable(plat_my_core_pos());
}

void k3_gic_cpuif_enable(void)
{
	gicv3_cpuif_enable(plat_my_core_pos());
}

void k3_gic_cpuif_disable(void)
{
	gicv3_cpuif_disable(plat_my_core_pos());
}

void k3_gic_pcpu_init(void)
{
	gicv3_rdistif_init(plat_my_core_pos());
}
